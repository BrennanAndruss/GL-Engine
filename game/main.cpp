#include "core/Application.h"

int main() {
    engine::Application app(640, 480);
    app.setClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    app.run();
    return 0;
}