#pragma once

#include <SDL2/SDL_events.h>

class WindowListener {
public:
    WindowListener() = default;

    virtual ~WindowListener() = default;

    virtual bool ProcessEvent(const SDL_Event &event) = 0;

    virtual void StartFrameEvent() = 0;

};
