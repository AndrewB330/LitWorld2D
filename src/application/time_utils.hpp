#pragma once

#include <chrono>
#include <queue>

namespace lit::common {

    class timer {
    public:
        timer();

        void reset();

        double get_elapsed_seconds();

    private:
        #ifdef _MSC_VER
        std::chrono::steady_clock::time_point m_start_time;
        #else
        std::chrono::system_clock::time_point m_start_time;
        #endif
    };

    class fps_timer {
    public:
        fps_timer() = default;

        void frame_start();

        void frame_end();

        [[nodiscard]] double get_average_fps() const;

        [[nodiscard]] double get_average_ms() const;

    private:
        const int kMaxFramesCount = 60;

        bool m_frame_started = false;

        std::queue<double> m_times;

        timer m_timer;
    };

}
