#ifndef UTILS_H
#define UTILS_H

#include <chrono>

std::chrono::time_point<std::chrono::system_clock> get_time_point(int us_delay);

void busywait_until(std::chrono::time_point<std::chrono::system_clock> end_tp);

void busywait(int us_delay);

#endif /* Guard */
