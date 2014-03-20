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
   		clock_collectors &mycc, unsigned verbose=1) :net_collector_base(maxstored) {
      this->verbose=verbose;
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

   unsigned get_nodes_number() {
      return nodes_number;
   }

   void update_net_collector_base () {
      string name1, name2;
      unsigned id1, id2, newid1, newid2;
      double str1, str2;
      double weight;

      // reset the base collector
   	reset_collector_base_content();
      // cout<<"u"; cout.flush();
      apply_time_window(timewindow);

      // cout<<"U"; cout.flush();
      // a method for encoding node's name
      unsigned long lastassignedpos;
      auto insert_to_namepos = [&]( string name,
            unordered_map <string, unsigned long> &namepos) {
         unordered_map<string, unsigned long>::const_iterator found =
            namepos.find(name);
         if (found==namepos.end())
            namepos[name]=(++lastassignedpos);
      };
      // cout<<"u"; cout.flush();

      // get the strengths
      unordered_map <string, unsigned long> namepos_all;
      lastassignedpos=-1;
      unordered_map <unsigned long, double> strengths;
      // cout<<"U"; cout.flush();
      for (auto it=latest.begin(); it!=latest.end(); it++) {
         name1 = it->name1;
         name2 = it->name2;
         weight = it->weight;
         insert_to_namepos(name1, namepos_all);
         insert_to_namepos(name2, namepos_all);
         id1 = namepos_all[name1];
         id2 = namepos_all[name2];
         strengths[id1] += weight;
         strengths[id2] += weight;
      }
      nodes_number = namepos_all.size();

      // sort the strengths
      // cout<<"u"; cout.flush();
      auto sort_second = [](
            const pair<unsigned long, double> &lhs,
            const pair<unsigned long, double> &rhs ) {
         return lhs.second > rhs.second;
      };
      // cout<<"U"; cout.flush();
      vector< pair<unsigned long, double> > strengths_sorted;
      for (auto it=strengths.begin(); it!=strengths.end(); it++)
         strengths_sorted.push_back(*it);
      sort( strengths_sorted.begin(), strengths_sorted.end(), sort_second );

      // cout<<"u"; cout.flush();

      // fill the base collector with at most maxstored strongest nodes
      unordered_map <string, unsigned long> namepos_buf;
      lastassignedpos=-1;
      unsigned i_threshold;
      if (strengths_sorted.size()>maxstored) i_threshold=maxstored;
      else i_threshold=strengths_sorted.size();
      double str_threshold = strengths_sorted[i_threshold].second;
      if (verbose>3) {
         cout<<"strengths_sorted:"<<endl;
         for (auto i=0; i<strengths_sorted.size(); i++)
            cout<<strengths_sorted[i].first<<" "
                <<strengths_sorted[i].second<<" ";
         cout<<endl;
      }
      // cout<<"U"; cout.flush();
      for (auto it=latest.begin(); it!=latest.end(); it++) {
         name1 = it->name1;
         name2 = it->name2;
         weight = it->weight;
         id1 = namepos_all[name1];
         id2 = namepos_all[name2];
         str1 = strengths[id1];
         str2 = strengths[id2];
         // cout<<"h"; cout.flush();
         if ( str1>str_threshold && str2>str_threshold) {
            insert_to_namepos(name1, namepos_buf);
            insert_to_namepos(name2, namepos_buf);
            // cout<<"h"; cout.flush();
            newid1 = namepos_buf[name1];
            // cout<<"i"; cout.flush();
            newid2 = namepos_buf[name2];
            // cout<<"i"; cout.flush();
            net[newid1][newid2] += weight;
            net[newid2][newid1] += weight;
            // cout<<"i"; cout.flush();
            net[newid1][newid1] += weight;
            net[newid2][newid2] += weight;
            // cout<<"i"; cout.flush();
            names[newid1] = name1;
            names[newid2] = name2;
            // cout<<"h"; cout.flush();
         }
         // cout<<"h"<<endl; cout.flush();
      }
      // cout<<"u"<<endl; cout.flush();
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
   unsigned nodes_number;
};



#endif