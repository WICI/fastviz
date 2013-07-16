#ifndef TIME_CHECKER_HPP
#define TIME_CHECKER_HPP

#include <iostream>
#include <cmath>

// Start a time checker with a specific time and an interval. Update it with times
// afterwards, and the update function will return true if the time given represents
// a shift into a new interval.

// calculates if the interval elapsed since the last reset
class time_checker {
public:
   time_checker(unsigned long start, unsigned long interval) 
      : start(start), interval(interval), cur_time(start), last_update_time(start) { 
   }

   bool operator () (unsigned long new_time) {
      cur_time = new_time;

      if (cur_time > last_update_time + interval) {
         return true;
      } else {
         return false;
      }
   }

   void reset() {
      last_update_time = cur_time;
   }

   unsigned till_next(unsigned long time_now) {
      unsigned next_tick = last_update_time + interval;
      return next_tick > time_now ? (next_tick - time_now) : 0;
   }

private:
   const unsigned long start;
   const unsigned long interval;
   unsigned long cur_time;
   unsigned long last_update_time;
};

#endif
