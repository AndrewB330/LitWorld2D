#pragma once

#include <string>
#include <SDL2/SDL.h>
#include <memory>
#include <vector>
#include "window.hpp"

class Application {
public:
    Application() = default;

    ~Application();

    bool Init();

    bool CreateWindow(
            const WindowInfo &window_info,
            std::vector<std::shared_ptr<WindowRenderer>> renderers = {},
            std::vector<std::shared_ptr<WindowListener>> listeners = {}
    );

    void PollEvents();

    void Redraw();

    bool AnyWindowAlive() const;

private:
    bool initialized = false;

    std::vector<Window> windows;

};
