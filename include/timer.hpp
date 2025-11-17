#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <string>

namespace matmul {

class Timer {
public:
    Timer();

    // Start/stop timer
    void start();
    void stop();
    void reset();

    // Get elapsed time
    double elapsed_seconds() const;
    double elapsed_milliseconds() const;
    double elapsed_microseconds() const;

    // Get formatted time string
    std::string elapsed_string() const;

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint start_time_;
    TimePoint stop_time_;
    bool is_running_;
};

} // namespace matmul

#endif // TIMER_HPP
