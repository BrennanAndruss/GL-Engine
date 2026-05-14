#pragma once

#include <cstddef>

namespace engine
{
    class AssetManager;
    class Object;
    class Scene;

    void updateEditorSelectionFromMouse(Scene& scene, std::size_t startIndex, Object*& selectedObject);
    void drawEditorObjectControls(Scene& scene, AssetManager& assets, std::size_t initialObjectCount, Object*& selectedObject);
}
