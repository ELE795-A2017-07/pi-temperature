#ifndef UTILS_H
#define UTILS_H

chrono::time_point<chrono::system_clock> get_time_point(int us_delay);

void busywait_until(chrono::time_point<chrono::system_clock> end_tp);

void busywait(int us_delay);

#endif /* Guard */
