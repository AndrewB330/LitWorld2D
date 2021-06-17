#include "time_utils.hpp"
#include <iostream>

using namespace lit::common;

typedef std::chrono::duration<double, std::ratio<1> > second_;

timer::timer() {
    m_start_time = std::chrono::high_resolution_clock::now();
}

double timer::get_elapsed_seconds() {
    auto cur_time = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast<second_>(cur_time - m_start_time).count();
    return delta;
}

void timer::reset() {
    m_start_time = std::chrono::high_resolution_clock::now();
}

double fps_timer::get_average_fps() const {
    if (m_times.empty()) {
        return 0;
    }
    return m_times.size() / (m_times.back() - m_times.front());
}

double fps_timer::get_average_ms() const {
    if (m_times.empty()) {
        return 0;
    }
    return (m_times.back() - m_times.front()) / m_times.size() * 1000.0;
}

void fps_timer::frame_start() {
    if (m_frame_started) {
        frame_end();
    }
    m_frame_started = true;
    m_timer = timer();
}

void fps_timer::frame_end() {
    double time = m_timer.get_elapsed_seconds();
    while(m_times.size() > kMaxFramesCount) {
        m_times.pop();
    }
    m_times.push((m_times.empty() ? 0 : m_times.back()) + time);
    m_frame_started = false;
}
