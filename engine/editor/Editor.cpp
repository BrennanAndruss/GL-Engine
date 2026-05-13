#include "editor/Editor.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <glm/gtc/matrix_inverse.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "core/Input.h"
#include "core/Time.h"
#include "renderer/resources/Material.h"
#include "resources/AssetManager.h"
#include "scene/Scene.h"
#include "scene/components/Components.h"

namespace
{
    bool rayIntersectsAabb(const glm::vec3& rayOrigin,
        const glm::vec3& rayDirection,
        const glm::vec3& boundsMin,
        const glm::vec3& boundsMax,
        float& outDistance)
    {
        float tMin = 0.0f;
        float tMax = std::numeric_limits<float>::max();

        for (int i = 0; i < 3; ++i)
        {
            if (std::fabs(rayDirection[i]) < 1e-6f)
            {
                if (rayOrigin[i] < boundsMin[i] || rayOrigin[i] > boundsMax[i])
                {
                    return false;
                }
                continue;
            }

            const float invDirection = 1.0f / rayDirection[i];
            float t1 = (boundsMin[i] - rayOrigin[i]) * invDirection;
            float t2 = (boundsMax[i] - rayOrigin[i]) * invDirection;
            if (t1 > t2)
            {
                std::swap(t1, t2);
            }

            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);
            if (tMax < tMin)
            {
                return false;
            }
        }

        outDistance = (tMin >= 0.0f) ? tMin : tMax;
        return outDistance >= 0.0f;
    }

    bool rayIntersectsSphere(const glm::vec3& rayOrigin,
        const glm::vec3& rayDirection,
        const glm::vec3& center,
        float radius,
        float& outDistance)
    {
        const glm::vec3 toCenter = rayOrigin - center;
        const float a = glm::dot(rayDirection, rayDirection);
        const float b = 2.0f * glm::dot(toCenter, rayDirection);
        const float c = glm::dot(toCenter, toCenter) - (radius * radius);
        const float discriminant = b * b - 4.0f * a * c;
        if (discriminant < 0.0f)
        {
            return false;
        }

        const float sqrtDiscriminant = std::sqrt(discriminant);
        const float t0 = (-b - sqrtDiscriminant) / (2.0f * a);
        const float t1 = (-b + sqrtDiscriminant) / (2.0f * a);
        outDistance = (t0 >= 0.0f) ? t0 : t1;
        return outDistance >= 0.0f;
    }

    bool rayIntersectsObject(const engine::Object& object,
        const glm::vec3& rayOrigin,
        const glm::vec3& rayDirection,
        float& outDistance)
    {
        const glm::vec3 worldPos = object.transform.getWorldPosition();
        const glm::vec3 worldScale = glm::abs(object.transform.getWorldScale());

        if (const auto* box = object.getComponent<engine::BoxCollider>())
        {
            const glm::vec3 center = worldPos + box->center;
            const glm::vec3 halfExtents = box->size * worldScale;
            return rayIntersectsAabb(
                rayOrigin,
                rayDirection,
                center - halfExtents,
                center + halfExtents,
                outDistance
            );
        }

        if (const auto* sphere = object.getComponent<engine::SphereCollider>())
        {
            const float radiusScale = std::max(worldScale.x, std::max(worldScale.y, worldScale.z));
            const float radius = sphere->radius * radiusScale;
            return rayIntersectsSphere(rayOrigin, rayDirection, worldPos + sphere->center, radius, outDistance);
        }

        // Fallback to a small transform-based AABB so mesh-only objects are still pickable.
        const glm::vec3 fallbackHalfExtents = glm::max(worldScale * 0.5f, glm::vec3(0.25f));
        return rayIntersectsAabb(
            rayOrigin,
            rayDirection,
            worldPos - fallbackHalfExtents,
            worldPos + fallbackHalfExtents,
            outDistance
        );
    }

    engine::Object* pickObjectAtMouse(engine::Scene& scene, std::size_t startIndex)
    {
        engine::Camera* camera = scene.getMainCamera();
        if (!camera)
        {
            return nullptr;
        }

        int windowWidth = 0;
        int windowHeight = 0;
        if (GLFWwindow* window = glfwGetCurrentContext())
        {
            glfwGetWindowSize(window, &windowWidth, &windowHeight);
        }

        if (windowWidth <= 0 || windowHeight <= 0)
        {
            const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
            windowWidth = static_cast<int>(displaySize.x);
            windowHeight = static_cast<int>(displaySize.y);
        }

        if (windowWidth <= 0 || windowHeight <= 0)
        {
            return nullptr;
        }

        const glm::vec2 mousePos = engine::Input::getMousePos();
        const float ndcX = (2.0f * mousePos.x) / static_cast<float>(windowWidth) - 1.0f;
        const float ndcY = 1.0f - (2.0f * mousePos.y) / static_cast<float>(windowHeight);

        camera->updateViewMatrix();
        const auto& cameraData = camera->getCameraData();
        const glm::mat4 inverseViewProjection = glm::inverse(cameraData.projection * cameraData.view);

        glm::vec4 nearPoint = inverseViewProjection * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
        glm::vec4 farPoint = inverseViewProjection * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
        if (std::fabs(nearPoint.w) < 1e-6f || std::fabs(farPoint.w) < 1e-6f)
        {
            return nullptr;
        }

        nearPoint /= nearPoint.w;
        farPoint /= farPoint.w;

        const glm::vec3 rayOrigin = glm::vec3(nearPoint);
        const glm::vec3 rayDirection = glm::normalize(glm::vec3(farPoint - nearPoint));

        engine::Object* closestObject = nullptr;
        float closestDistance = std::numeric_limits<float>::max();

        const auto& objects = scene.getObjects();
        for (std::size_t i = startIndex; i < objects.size(); ++i)
        {
            const auto& object = objects[i];
            if (object->getComponent<engine::Camera>())
            {
                continue;
            }

            float hitDistance = 0.0f;
            if (rayIntersectsObject(*object, rayOrigin, rayDirection, hitDistance) && hitDistance < closestDistance)
            {
                closestDistance = hitDistance;
                closestObject = object.get();
            }
        }

        return closestObject;
    }
}

namespace engine
{
    void Editor::initialize(GLFWwindow* windowHandle)
    {
        if (_initialized)
        {
            return;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(windowHandle, true);
        ImGui_ImplOpenGL3_Init("#version 150");

        _initialized = true;
    }

    void Editor::shutdown()
    {
        if (!_initialized)
        {
            return;
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        _initialized = false;
    }

    void Editor::beginFrame()
    {
        if (!_initialized)
        {
            return;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Editor::draw(Scene& scene, AssetManager& assets)
    {
        if (!_initialized)
        {
            return;
        }

        if (!_capturedInitialSceneState)
        {
            _initialObjectCount = scene.getObjects().size();
            _capturedInitialSceneState = true;
        }

        const bool mouseOverUi = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemHovered();
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !mouseOverUi)
        {
            if (Object* pickedObject = pickObjectAtMouse(scene, _initialObjectCount))
            {
                _selectedObject = pickedObject;
            }
        }

        ImGui::Begin("Editor");

        if (ImGui::Button("Save Scene"))
        {
            const std::string filename = "scene_objects.txt";
            bool ok = writeObjectsToFile(scene.getObjects(), filename, _initialObjectCount, assets);
            if (ok)
            {
                std::cout << "Saved scene to " << filename << std::endl;
            }
            else
            {
                std::cerr << "Failed to save scene to " << filename << std::endl;
            }
        }

        if (ImGui::Button("Load Scene"))
        {
            const std::string filename = "scene_objects.txt";
            bool ok = readObjectsFromFile(filename, scene, assets);
            if (ok)
            {
                std::cout << "Loaded scene from " << filename << std::endl;
            }
            else
            {
                std::cerr << "Failed to load scene from " << filename << std::endl;
            }
        }

       

        ImGui::Text("Create object:");
        if (ImGui::Button("Create Cube"))
        {
            int nextCubeIndex = 0;
            while (true)            {
                std::string cubeName = "Cube" + std::to_string(nextCubeIndex);
                bool nameExists = false;
                for (const auto& object : scene.getObjects())
                { 
                    if (object->name == cubeName)
                    {                        
                        nameExists = true;
                        break;
                    }
                }
                if (!nameExists) break;
                nextCubeIndex++;
            }
            auto& cube = scene.createObject("Cube" + std::to_string(nextCubeIndex));
            auto& meshRenderer = cube.addComponent<MeshRenderer>();
            meshRenderer.mesh = assets.getMeshHandle("cube");
            meshRenderer.material = assets.getMaterialHandle("defaultMat");

            _selectedObject = &cube;


        }

        ImGui::Separator();

        if (_selectedObject) {
            if (ImGui::Button("Clear Selection"))
            {
                _selectedObject = nullptr;
            }

            if (ImGui::Button("Duplicate Selection") && _selectedObject)
            {
                int nextCopyIndex = 0;
                while (true)
                {
                    std::string copyName = _selectedObject->name + "_Copy" + std::to_string(nextCopyIndex);
                    bool nameExists = false;
                    for (const auto& object : scene.getObjects())
                    {
                        if (object->name == copyName)
                        {
                            nameExists = true;
                            break;
                        }
                    }
                    if (!nameExists) break;
                    nextCopyIndex++;
                }

                auto& newObject = scene.createObject(_selectedObject->name + "_Copy" + std::to_string(nextCopyIndex));
                newObject.transform.setPosition(_selectedObject->transform.getPosition() + glm::vec3(1.0f, 0.0f, 0.0f));
                newObject.transform.setRotation(_selectedObject->transform.getRotation());
                newObject.transform.setScale(_selectedObject->transform.getScale());

                // Copy MeshRenderer if exists
                if (auto* meshRenderer = _selectedObject->getComponent<MeshRenderer>())
                {
                    auto& newMeshRenderer = newObject.addComponent<MeshRenderer>();
                    newMeshRenderer.mesh = meshRenderer->mesh;
                    newMeshRenderer.material = meshRenderer->material;
                }

                // Copy BoxCollider if exists
                if (auto* boxCollider = _selectedObject->getComponent<BoxCollider>())
                {
                    auto& newBoxCollider = newObject.addComponent<BoxCollider>();
                    newBoxCollider.center = boxCollider->center;
                    newBoxCollider.size = boxCollider->size;
                    newBoxCollider.isTrigger = boxCollider->isTrigger;
                }

                // Copy SphereCollider if exists
                if (auto* sphereCollider = _selectedObject->getComponent<SphereCollider>())
                {
                    auto& newSphereCollider = newObject.addComponent<SphereCollider>();
                    newSphereCollider.center = sphereCollider->center;
                    newSphereCollider.radius = sphereCollider->radius;
                    newSphereCollider.isTrigger = sphereCollider->isTrigger;
                }

                _selectedObject = &newObject;
            }

        }

        ImGui::Separator();

        ImGui::Text("Objects:");
        


        const auto& objects = scene.getObjects();
        if (_selectedObject)
        {
            bool selectedStillListed = false;
            for (std::size_t i = _initialObjectCount; i < objects.size(); ++i)
            {
                if (objects[i].get() == _selectedObject)
                {
                    selectedStillListed = true;
                    break;
                }
            }

            if (!selectedStillListed)
            {
                _selectedObject = nullptr;
            }
        }

        for (std::size_t i = _initialObjectCount; i < objects.size(); ++i)
        {
            const auto& object = objects[i];
            const bool isSelected = _selectedObject == object.get();
            if (ImGui::Selectable(object->name.c_str(), isSelected))
            {
                _selectedObject = object.get();
            }
        }

        ImGui::End();

        ImGui::Begin("Transform");
        if (_selectedObject)
        {
            ImGui::Text("Selected: %s", _selectedObject->name.c_str());
            ImGui::Separator();

            glm::vec3 position = _selectedObject->transform.getPosition();
            glm::vec3 rotation = _selectedObject->transform.getEulerAngles();
            glm::vec3 scale = _selectedObject->transform.getScale();

            if (ImGui::DragFloat3("Position", &position.x, 0.1f))
            {
                _selectedObject->transform.setPosition(position);
            }

            if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0f))
            {
                _selectedObject->transform.setEulerAngles(rotation);
            }

            if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.01f, 100.0f))
            {
                _selectedObject->transform.setScale(scale);
            }

            // add component button

            const bool hasCollider = _selectedObject->getComponent<BoxCollider>() != nullptr;
            const bool hasRigidBody = _selectedObject->getComponent<RigidBody>() != nullptr;
            if (!hasCollider || !hasRigidBody)
            {
                if (ImGui::Button("Add Physics Collision"))
                {
                    if (!hasCollider)
                    {
                        _selectedObject->addComponent<BoxCollider>();
                    }

                    if (!hasRigidBody)
                    {
                        _selectedObject->addComponent<RigidBody>();
                    }
                }
            }

            if (auto* rigidBody = _selectedObject->getComponent<RigidBody>())
            {
                const char* bodyTypes[] = { "Static", "Kinematic", "Dynamic" };
                int selectedType = static_cast<int>(rigidBody->getBodyType());
                if (ImGui::Combo("RigidBody Type", &selectedType, bodyTypes, IM_ARRAYSIZE(bodyTypes)))
                {
                    rigidBody->setBodyType(static_cast<RigidBody::BodyType>(selectedType));
                }

                if (rigidBody->getBodyType() == RigidBody::BodyType::Dynamic)
                {
                    ImGui::DragFloat("Mass", &rigidBody->mass, 0.1f, 0.0f, 1000.0f);
                }
            }
            
        }
        else
        {
            ImGui::TextDisabled("Select an object to edit its transform.");
        }
        ImGui::End();

        // Debugging
        ImGui::Begin("Debug");
        ImGui::Text("FPS: %.1f", Time::deltaTime() > 0.0f ? 1.0f / Time::deltaTime() : 0.0f);
        ImGui::Text("Delta: %.3f ms", Time::deltaTime() * 1000.0f);
        ImGui::End();
    }

    void Editor::endFrame()
    {
        if (!_initialized)
        {
            return;
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    bool Editor::writeObjectsToFile(const std::vector<std::unique_ptr<Object>>& objects,
        const std::string& filename,
        std::size_t startIndex,
        const AssetManager& assets)
    {
        try
        {
            const std::string tmpPath = filename + ".tmp";

            std::ofstream file(tmpPath, std::ios::binary);
            if (!file.is_open())
            {
                std::cerr << "Failed to open temp file for writing: " << tmpPath << std::endl;
                return false;
            }

            for (std::size_t i = startIndex; i < objects.size(); ++i)
            {
                const auto& object = objects[i];
                file << "Object: " << object->name << "\n";
                // transforms
                file << "  Position: " << object->transform.getPosition().x << ", "
                     << object->transform.getPosition().y << ", "
                     << object->transform.getPosition().z << "\n";
                file << "  Rotation: " << object->transform.getEulerAngles().x << ", "
                     << object->transform.getEulerAngles().y << ", "
                     << object->transform.getEulerAngles().z << "\n";
                file << "  Scale: " << object->transform.getScale().x << ", "
                     << object->transform.getScale().y << ", "
                     << object->transform.getScale().z << "\n";
                // mesh & material
                if (auto* meshRenderer = object->getComponent<MeshRenderer>())
                {                    
                    file << "  Mesh: " << (meshRenderer->mesh.valid() ? std::to_string(meshRenderer->mesh.index) : std::string("invalid")) << "\n";
                    const std::string materialName = assets.getMaterialName(meshRenderer->material);
                    file << "  Material: " << (!materialName.empty() ? materialName : std::string("invalid")) << "\n";
                    if (auto* material = assets.getMaterial(meshRenderer->material))
                    {
                        const std::string difTexName = assets.getTextureName(material->difTex);
                        const std::string specTexName = assets.getTextureName(material->specTex);
                        file << "  DiffuseTexture: " << (!difTexName.empty() ? difTexName : std::string("invalid")) << "\n";
                        file << "  SpecularTexture: " << (!specTexName.empty() ? specTexName : std::string("invalid")) << "\n";
                    }
                }
                if (auto* collider = object->getComponent<BoxCollider>())
                {
                    file << "  BoxCollider: size=" << collider->size.x << "," << collider->size.y << "," << collider->size.z
                         << " isTrigger=" << collider->isTrigger << "\n";
                }
                if (auto* rigidBody = object->getComponent<RigidBody>())
                {
                    file << "  RigidBody: type=" << static_cast<int>(rigidBody->getBodyType()) << " mass=" << rigidBody->mass << "\n";
                }
                file << "\n";
                if (!file.good())
                {
                    std::cerr << "Write error while writing to temp file: " << tmpPath << std::endl;
                    file.close();
                    std::remove(tmpPath.c_str());
                    return false;
                }
            }

            file.flush();
            if (!file)
            {
                std::cerr << "Flush failed for temp file: " << tmpPath << std::endl;
                file.close();
                std::remove(tmpPath.c_str());
                return false;
            }

            file.close();

            if (std::rename(tmpPath.c_str(), filename.c_str()) != 0)
            {
                std::cerr << "Failed to rename temp file to target: " << filename << std::endl;
                std::remove(tmpPath.c_str());
                return false;
            }

            return true;
        }
        catch (const std::exception& ex)
        {
            std::cerr << "Exception writing objects to file: " << ex.what() << std::endl;
            return false;
        }
    }

    bool Editor::readObjectsFromFile(const std::string& filename, Scene& scene, AssetManager& assets)
    {
        try
        {
            std::ifstream file(filename);
            if (!file.is_open())
            {
                std::cerr << "Failed to open file for reading: " << filename << std::endl;
                return false;
            }

            auto parseVec3 = [](const std::string& value) -> glm::vec3
            {
                std::stringstream stream(value);
                float x = 0.0f;
                float y = 0.0f;
                float z = 0.0f;
                char comma = ',';
                stream >> x >> comma >> y >> comma >> z;
                return glm::vec3(x, y, z);
            };

            std::string line;
            const Handle<Material> defaultMaterial = assets.getMaterialHandle("defaultMat");
            struct LoadedObject
            {
                std::string name;
                glm::vec3 position{0.0f};
                glm::vec3 rotation{0.0f};
                glm::vec3 scale{1.0f};

                bool hasMesh = false;
                Handle<Mesh> mesh;

                bool hasMaterial = false;
                Handle<Material> material;

                bool hasBoxCollider = false;
                glm::vec3 boxColliderSize{1.0f};
                bool boxColliderIsTrigger = false;

                bool hasRigidBody = false;
                RigidBody::BodyType rigidBodyType = RigidBody::BodyType::Static;
                float rigidBodyMass = 1.0f;
            };

            auto flushObject = [&](const LoadedObject& data)
            {
                Object& object = scene.createObject(data.name);
                object.transform.setPosition(data.position);
                object.transform.setEulerAngles(data.rotation);
                object.transform.setScale(data.scale);

                if (data.hasMesh || data.hasMaterial)
                {
                    auto& meshRenderer = object.addComponent<MeshRenderer>();
                    if (data.hasMesh)
                    {
                        meshRenderer.mesh = data.mesh;
                    }
                    if (data.hasMaterial)
                    {
                        meshRenderer.material = data.material;
                    }
                    else
                    {
                        meshRenderer.material = defaultMaterial;
                    }
                }

                if (data.hasBoxCollider)
                {
                    auto& collider = object.addComponent<BoxCollider>();
                    collider.size = data.boxColliderSize;
                    collider.isTrigger = data.boxColliderIsTrigger;
                }

                if (data.hasRigidBody)
                {
                    auto& rigidBody = object.addComponent<RigidBody>();
                    rigidBody.setBodyType(data.rigidBodyType);
                    rigidBody.mass = data.rigidBodyMass;
                }
            };

            std::unique_ptr<LoadedObject> currentObject;

            while (std::getline(file, line))
            {
                if (line.rfind("Object: ", 0) == 0)
                {
                    if (currentObject)
                    {
                        flushObject(*currentObject);
                    }

                    currentObject = std::make_unique<LoadedObject>();
                    currentObject->name = line.substr(8);
                    continue;
                }

                if (!currentObject)
                {
                    continue;
                }

                if (line.rfind("  Position: ", 0) == 0)
                {
                    currentObject->position = parseVec3(line.substr(12));
                }
                else if (line.rfind("  Rotation: ", 0) == 0)
                {
                    currentObject->rotation = parseVec3(line.substr(12));
                }
                else if (line.rfind("  Scale: ", 0) == 0)
                {
                    currentObject->scale = parseVec3(line.substr(9));
                }
                else if (line.rfind("  Mesh: ", 0) == 0)
                {
                    const std::string value = line.substr(8);
                    if (value != "invalid")
                    {
                        currentObject->mesh.index = static_cast<std::size_t>(std::stoull(value));
                        if (assets.getMesh(currentObject->mesh))
                        {
                            currentObject->hasMesh = true;
                        }
                    }
                }
                else if (line.rfind("  Material: ", 0) == 0)
                {
                    const std::string value = line.substr(12);
                    if (value != "invalid")
                    {
                        Handle<Material> materialHandle = assets.getMaterialHandle(value);
                        if (!materialHandle.valid())
                        {
                            try
                            {
                                materialHandle.index = static_cast<std::size_t>(std::stoull(value));
                            }
                            catch (const std::exception&)
                            {
                                materialHandle = {};
                            }
                        }

                        if (assets.getMaterial(materialHandle))
                        {
                            currentObject->material = materialHandle;
                            currentObject->hasMaterial = true;
                        }
                    }
                }
                else if (line.rfind("  BoxCollider: ", 0) == 0)
                {
                    const std::string value = line.substr(15);
                    const std::size_t sizePrefix = value.find("size=");
                    const std::size_t triggerPrefix = value.find(" isTrigger=");
                    if (sizePrefix == std::string::npos)
                    {
                        continue;
                    }

                    const std::string sizeValue = value.substr(sizePrefix + 5, triggerPrefix == std::string::npos
                        ? std::string::npos
                        : triggerPrefix - (sizePrefix + 5));
                    std::stringstream stream(sizeValue);
                    float x = 0.0f;
                    float y = 0.0f;
                    float z = 0.0f;
                    char comma = ',';
                    stream >> x >> comma >> y >> comma >> z;
                    currentObject->hasBoxCollider = true;
                    currentObject->boxColliderSize = glm::vec3(x, y, z);

                    if (triggerPrefix != std::string::npos)
                    {
                        const std::string triggerValue = value.substr(triggerPrefix + 11);
                        currentObject->boxColliderIsTrigger = triggerValue == "1" || triggerValue == "true";
                    }
                }
                else if (line.rfind("  RigidBody: ", 0) == 0)
                {
                    const std::string value = line.substr(13);
                    const std::size_t typePrefix = value.find("type=");
                    const std::size_t massPrefix = value.find(" mass=");
                    if (typePrefix == std::string::npos || massPrefix == std::string::npos)
                    {
                        continue;
                    }

                    const std::string typeValue = value.substr(typePrefix + 5, massPrefix - (typePrefix + 5));
                    const std::string massValue = value.substr(massPrefix + 6);
                    int bodyType = 0;
                    float mass = 0.0f;
                    bodyType = std::stoi(typeValue);
                    mass = std::stof(massValue);
                    currentObject->hasRigidBody = true;
                    currentObject->rigidBodyType = static_cast<RigidBody::BodyType>(bodyType);
                    currentObject->rigidBodyMass = mass;
                }
            }

            if (currentObject)
            {
                flushObject(*currentObject);
            }

            return true;
        }
        catch (const std::exception& ex)
        {
            std::cerr << "Exception reading objects from file: " << ex.what() << std::endl;
            return false;
        }
    }
}


