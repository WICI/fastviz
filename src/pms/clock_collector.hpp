#ifndef PMS_CLOCK_COLLECTOR_HPP
#define PMS_CLOCK_COLLECTOR_HPP

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdarg.h>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>

using namespace std;
using boost::lexical_cast;

// Calculate how much time has passed between consequtive measurements
// Easy to use in loops to collect time spent in various parts of the loop


struct clock_collector{
	string first;
	double second;
	clock_collector(){second=0;}
	clock_collector(string name){first=name; second=0;}
	bool operator==(clock_collector other) {return first==other.first;}
};

class clock_collectors {
public:
	clock_collectors() { refresh(); }
	
	void addcollector(string name) { 
		collected.push_back(*(new clock_collector(name))); refresh(); 
	}
	void addcollectors(int n) {
		for (int i=0; i<n; i++) addcollector(lexical_cast<string>(n)); refresh(); 
	}
	
	void addnamedcollectors(int n, ...) {
		va_list vl;
		va_start( vl, n );
		for (int i=0; i<n; i++) {
			string name=lexical_cast<string>(va_arg( vl, char*));
			collected.push_back(*(new clock_collector(name)));
		}
		va_end(vl);
	}
	
	void refresh() { startpoint=clock(); }
	
	void collect(string name) {
		vector <clock_collector>::iterator it=find(collected.begin(), collected.end(), name);
		if (it!=collected.end()) {
			it->second+=(clock()-startpoint);
			startpoint=clock(); 
		}
		else error_memoryexeed();
	}
	
	// this is just for compatibility with the previous version
	void collect(int name) {
		vector <clock_collector>::iterator it=find(collected.begin(), collected.end(), 
			lexical_cast<string>(name));
		if (it!=collected.end()) {
			it->second+=(clock()-startpoint);
			startpoint=clock(); 
		}
		else error_memoryexeed();
	}
	
	void printall() {
		for (vector <clock_collector>::iterator it=collected.begin(); 
		it!=collected.end(); it++) {
			// 				printf("TTTTTTTTTTTTTTTTTTT %s%10.2f\n", 
			// 					(*it).first.c_str(), (*it).second/CLOCKS_PER_SEC);
			// 				cout<<"TTTTTTTTTTTTTTTTTTT "<<(*it).first)
			// 					<<setw(10)<<setprecision(2)<<(*it).second/CLOCKS_PER_SEC);
			cout<<setfill('T')<<setw(40)<<(*it).first<<setw(10)<<setprecision(2)
			<<setfill(' ')<<(*it).second/CLOCKS_PER_SEC<<"\n";
			startpoint=clock();
		}
	}
	
	void resetall() {
		for (vector <clock_collector>::iterator it=collected.begin(); 
		it!=collected.end(); it++) (*it).second=0;
		startpoint=clock();
	}
	
private:
	double startpoint;
	vector <clock_collector> collected;
	
	void error_memoryexeed()	{ cout<<"Error: clock_collectors memory storage exeeded\n"; exit(-1);}
		
};

#endif