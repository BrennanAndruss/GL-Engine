#pragma once

struct GLFWwindow;

namespace engine
{
    class Scene;
    class AssetManager;
    class Object;
}

#include <memory>
#include <string>
#include <vector>
#include <cstddef>

namespace engine
{
    class Editor
    {
    public:
        Editor() = default;
        ~Editor() = default;

        void initialize(GLFWwindow* windowHandle);
        void shutdown();

        void beginFrame();
        void draw(Scene& scene, AssetManager& assets);
        void endFrame();
        bool writeObjectsToFile(const std::vector<std::unique_ptr<Object>>& objects,
                    const std::string& filename,
                    std::size_t startIndex,
                    const AssetManager& assets);
        bool readObjectsFromFile(const std::string& filename, Scene& scene, AssetManager& assets);
        bool hasSelectedObject() const { return _selectedObject != nullptr; }
      

    private:
        bool _initialized = false;
        bool _capturedInitialSceneState = false;
        std::size_t _initialObjectCount = 0;
        bool _toolActive = true;
        float _toolColor[4] = { 0.20f, 0.55f, 0.90f, 1.0f };
		Object* _selectedObject = nullptr;
    };
}
