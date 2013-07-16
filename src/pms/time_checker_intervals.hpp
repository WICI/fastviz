#ifndef TIME_CHECKER_INTERVALS_HPP
#define TIME_CHECKER_INTERVALS_HPP

#include <iostream>
#include <cmath>

#include <boost/date_time/posix_time/posix_time.hpp>

// namespace pt = boost::posix_time;
using namespace boost::posix_time;

// Start a time checker with a specific time and an interval. Update it with times
// afterwards, and the update function will return true if the time given represents
// a shift into a new interval.

// calculates how many intervals elapsed since the last reset
class time_checker_intervals {
public:
	time_checker_intervals(unsigned long start, unsigned long interval) 
	: start(start), interval(interval), cur_time(start), last_update_time(start) { 
	}
	
	int operator () (unsigned long new_time) {
		cur_time = new_time;
		
		int howmany=floor((cur_time-last_update_time)/(double)interval);
		return howmany;
	}
	
	void reset() {
		last_update_time = cur_time;
	}
	
	unsigned till_next(unsigned long time_now) {
		unsigned next_tick = last_update_time + interval;
		return next_tick > time_now ? (next_tick - time_now) : 0;
	}
	
// private:
	const unsigned long start;
	const unsigned long interval;
	unsigned long cur_time;
	unsigned long last_update_time;
};

class time_checker_intervals_micro {
public:
	time_checker_intervals_micro(ptime start, time_duration interval) 
	: start(start), interval(interval), cur_time(start), last_update_time(start) { 
	}
	
	int operator () (ptime new_time) {
		cur_time = new_time;
		
		int howmany=floor((cur_time-last_update_time).total_microseconds()/
			(double)interval.total_microseconds());
		return howmany;
	}
	
	void reset() {
		last_update_time = cur_time;
	}
	
	time_duration till_next(ptime time_now) {
		ptime next_tick = last_update_time + interval;
		cur_time = time_now;
		//cout<<last_update_time<<" "<<time_now<<" "<<next_tick<<" "<<next_tick - time_now<<'\n';
		return next_tick > time_now ? (next_tick - time_now) : microseconds(0);
	}
	
private:
	ptime start;
	time_duration interval;
	ptime cur_time;
	ptime last_update_time;
};



#endif
