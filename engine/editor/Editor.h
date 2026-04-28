#pragma once

struct GLFWwindow;

namespace engine
{
	class Scene;
    class AssetManager;
    class Object;
}

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

    private:
        bool _initialized = false;
        bool _toolActive = true;
        float _toolColor[4] = { 0.20f, 0.55f, 0.90f, 1.0f };
		Object* _selectedObject = nullptr;
    };
}
