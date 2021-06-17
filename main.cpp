#include <iostream>
#include <sample_scene.hpp>
#include <application/application.hpp>

int main(int, char **) {
    Application app;
    app.Init();

    WindowInfo game_window;
    game_window.title = "LitWorld2D";
    game_window.maximized = true;
    game_window.width = 1280;
    game_window.height = 720;

    auto scene = std::make_shared<Scene>();

    app.CreateWindow(game_window, {scene}, {scene});


    while (app.AnyWindowAlive()) {
        app.PollEvents();
        app.Redraw();
    }

    return 0;
}
