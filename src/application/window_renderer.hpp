#pragma once

#include "window_listener.hpp"

class WindowRenderer {
public:
    WindowRenderer() = default;

    virtual ~WindowRenderer() = default;

    virtual bool Init(SDL_Window *window, SDL_GLContext context) = 0;

    virtual void Redraw() = 0;
};
