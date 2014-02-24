/*
 * Buffer a subgraph of the full graph of edges provided as the input
 * by simply storing the graph of interactions appearing in a time-window
 */

#ifndef VIZ_NET_COLLECTOR_TIMEWINDOW_HPP
#define VIZ_NET_COLLECTOR_TIMEWINDOW_HPP

#include <cassert>
#include <deque>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>
#include <list>
#include <unordered_map>

#include <pms/clock_collector.hpp>
#include <viz/node.hpp>
#include <viz/link.cpp>
#include <viz/net_collector_base.hpp>

using namespace std;

class net_collector_timewindow : public net_collector_base {
public:
   
   net_collector_timewindow (const unsigned maxstored, double timewindow, 
   		clock_collectors &mycc) :net_collector_base(maxstored) {
      verbose=1;
      myclockcollector=&mycc;
      this->timewindow=timewindow;
   }
   
   void add_linkpack (vector <string> &linkpack, double weight, long ts) {
      for (int i=0; i<linkpack.size(); i++) 
         for (int j=0; j<linkpack.size(); j++) if (i<j) {
            string name1=linkpack[i];
            string name2=linkpack[j];
            link_timed l(name1, name2, weight, ts);
            latest.push_back( l );
         }
   }

   void update_net_collector_base () {
      string name1, name2;
      unsigned pos1, pos2;
      double weight;
      
      // reset the base collector
   	reset_collector_base_content();
      apply_time_window(timewindow);

      // prepare helper containers and functions
      unordered_map <string, unsigned long> namepos;
      unsigned long lastassignedpos=-1;         
      auto insert_to_namepos = [&](string name) {
         unordered_map<string, unsigned long>::const_iterator found = 
            namepos.find(name);
         if (found==namepos.end())
            namepos[name]=(++lastassignedpos);
      };

      // fill the base collector with the data
      list<link_timed>::iterator it;
      unsigned links_omited=0;
      for (it=latest.begin(); it!=latest.end(); it++) {
         name1 = it->name1;
         name2 = it->name2;
         weight = it->weight;
         insert_to_namepos(name1);
         insert_to_namepos(name2);
         pos1 = namepos[name1];
         pos2 = namepos[name2];
         if (pos1>=maxstored || pos2>=maxstored) {
            links_omited++;
            continue;
         }
         else {
            net[pos1][pos2] += weight;
            net[pos2][pos1] += weight;
            net[pos1][pos1] += weight;
            net[pos2][pos2] += weight;
            names[pos1] = name1;
            names[pos2] = name2;
         }
      }
      if (links_omited>0)
         cout<<"Warning: maxstored reached, "<<links_omited<<"."<<endl;

   }
   
   // no forgetting for this method
   void forget_connections (double forgetfactor) {}

private:

   void apply_time_window (double timewindow) {
      long latesttime=latest.back().ts;
      list<link_timed>::iterator limitingit=latest.begin();
      while(latesttime - limitingit->ts > timewindow) limitingit++;
      latest.erase(latest.begin(), limitingit);
   }

   list <link_timed> latest;
   double timewindow;
   clock_collectors *myclockcollector;
   unsigned verbose;
};



#endif