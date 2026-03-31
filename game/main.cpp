#include "MyGame.h"
#include "core/Application.h"

int main() {
    engine::AppConfig config;
    config.title = "My Game";
    config.width = 640;
    config.height = 480;

    engine::Application app(config);
    app.run(std::make_unique<MyGame>());
    return 0;
}