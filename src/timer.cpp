#include "timer.hpp"
#include <sstream>
#include <iomanip>

namespace matmul {

Timer::Timer() : is_running_(false) {}

void Timer::start() {
    start_time_ = Clock::now();
    is_running_ = true;
}

void Timer::stop() {
    stop_time_ = Clock::now();
    is_running_ = false;
}

void Timer::reset() {
    start_time_ = TimePoint();
    stop_time_ = TimePoint();
    is_running_ = false;
}

double Timer::elapsed_seconds() const {
    TimePoint end_time = is_running_ ? Clock::now() : stop_time_;
    auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(
        end_time - start_time_);
    return duration.count();
}

double Timer::elapsed_milliseconds() const {
    return elapsed_seconds() * 1000.0;
}

double Timer::elapsed_microseconds() const {
    return elapsed_seconds() * 1000000.0;
}

std::string Timer::elapsed_string() const {
    double seconds = elapsed_seconds();
    std::ostringstream oss;

    if (seconds < 0.001) {
        // Display in microseconds
        oss << std::fixed << std::setprecision(2) << (seconds * 1e6) << " μs";
    } else if (seconds < 1.0) {
        // Display in milliseconds
        oss << std::fixed << std::setprecision(2) << (seconds * 1e3) << " ms";
    } else if (seconds < 60.0) {
        // Display in seconds
        oss << std::fixed << std::setprecision(3) << seconds << " s";
    } else {
        // Display in minutes and seconds
        int minutes = static_cast<int>(seconds / 60);
        double remaining_seconds = seconds - (minutes * 60);
        oss << minutes << "m " << std::fixed << std::setprecision(2)
            << remaining_seconds << "s";
    }

    return oss.str();
}

} // namespace matmul
