#ifndef PACE_CHECKER_HPP
#define PACE_CHECKER_HPP

#include <string>

#include <boost/timer.hpp>

// Check to see if the rate of reading tweets is keeping pace with
// the tweets themselves. Does this by comparing the wall clock time
// with the passage of time in the tweets stream.

class pace_checker {
   boost::timer t;
   const unsigned long first_tweet_time;
   unsigned long last_pace_tweet_time;
   double last_elapsed;
   unsigned long last_tweet_time;

public:
   pace_checker(const unsigned long start_tweet_time) 
      : first_tweet_time(start_tweet_time), last_pace_tweet_time(0),
        last_elapsed(0), last_tweet_time(0) { }

   void next_tweet(const unsigned long when) {
      last_tweet_time = when;
   }

   double pace_instant();
   double pace_overall() const;
   std::string stats();
};

#endif
