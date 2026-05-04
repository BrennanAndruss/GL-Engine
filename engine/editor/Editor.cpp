#include "editor/Editor.h"

#include <cmath>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "core/Time.h"
#include "resources/AssetManager.h"
#include "scene/Scene.h"
#include "scene/components/Components.h"

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

        ImGui::Begin("Editor");

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
            meshRenderer.material = assets.getMaterialHandle("grassMat");

            _selectedObject = &cube;


        }

        ImGui::Separator();

        ImGui::Text("Select object:");
        if (ImGui::Button("Clear Selection"))
        {
            _selectedObject = nullptr;
        }


        for (const auto& object : scene.getObjects())
        {
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

    
}
