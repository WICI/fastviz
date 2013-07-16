#include "pace_checker.hpp"

#include <iomanip>
#include <sstream>

#include <util/format_time.hpp>

using namespace std;

double pace_checker::pace_instant() {
   unsigned long last = (last_pace_tweet_time ? last_pace_tweet_time : first_tweet_time);
   
   // tweet elapsed time / real elapsed time
   double t_elapsed = t.elapsed();
   double p = (last_tweet_time - last) / (t_elapsed - last_elapsed);
   last_pace_tweet_time = last_tweet_time;
   last_elapsed = t_elapsed;
   return p;
}

double pace_checker::pace_overall() const {
   return (last_tweet_time - first_tweet_time) / t.elapsed();
}

string pace_checker::stats() {
   ostringstream s;

   unsigned long tweet_elapsed = last_tweet_time - first_tweet_time;
   unsigned long wall_elapsed = (unsigned long)t.elapsed();

   s << setprecision(1) << fixed;

   s << "instant: " << pace_instant() << ", overall: " << pace_overall() << ". "
     << format_elapsed(tweet_elapsed) << " twt, " 
     << format_elapsed(wall_elapsed) << " wall.";

   return s.str();
}
