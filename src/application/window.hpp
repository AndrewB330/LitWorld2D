#pragma once

#include <string>
#include <memory>
#include <vector>
#include <SDL2/SDL.h>
#include "window_listener.hpp"
#include "window_renderer.hpp"

struct WindowInfo {
    std::string title = "Default";
    int width = 800;
    int height = 600;
    bool maximized = true;
    bool resizable = true;
};

class Window {
public:
    Window() = default;

    Window(const Window &window) = delete;

    Window(Window &&window) noexcept;

    Window(WindowInfo info);

    ~Window();

    bool Init();

    void Redraw();

    bool ProcessEvent(const SDL_Event &event);

    bool IsClosed() const;

    void AddRenderer(std::shared_ptr<WindowRenderer> renderer);

    void AddListener(std::shared_ptr<WindowListener> listener);

private:
    WindowInfo info = WindowInfo();

    SDL_Window *sdl_window = nullptr;
    SDL_GLContext gl_context = nullptr;

    bool initialized = false;
    bool closed = false;

    std::vector<std::shared_ptr<WindowRenderer>> renderers;
    std::vector<std::shared_ptr<WindowListener>> listeners;
};

