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

   net_collector_timewindow ( const unsigned maxstored,
         const double timewindow, const double forgetconst, const string viztype,
    		clock_collectors &mycc, unsigned verbose=1):
            net_collector_base(maxstored),
            timewindow(timewindow),
            forgetconst(forgetconst),
            verbose(verbose),
            viztype(viztype) {
      myclockcollector=&mycc;
   }

   void add_linkpack (vector <string> &linkpack, double weight, long ts, int verbose=0) {
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
      long id1, id2, newid1, newid2;
      double str1, str2;
      double weight;

      // reset the base collector
    	reset_collector_base_content();
      if (viztype=="exptimewindow")
         apply_exp_decay();
      else if (viztype=="timewindow")
         apply_time_window();

      // a method for encoding node's name
      long lastassignedpos;
      auto insert_to_namepos = [&]( string name,
            unordered_map <string, long> &namepos) {
         auto found = namepos.find(name);
         if (found==namepos.end())
            namepos[name]=(++lastassignedpos);
      };

      // get the strengths
      unordered_map <string, long> namepos_all;
      lastassignedpos=-1;
      unordered_map <long, double> strengths;
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
      auto sort_second = [](
            const pair<long, double> &lhs,
            const pair<long, double> &rhs ) {
         return lhs.second > rhs.second;
      };
      vector< pair<long, double> > strengths_sorted;
      for (auto it=strengths.begin(); it!=strengths.end(); it++)
         strengths_sorted.push_back(*it);
      sort( strengths_sorted.begin(), strengths_sorted.end(), sort_second );

      // get node's id by either creating it or finding it
      auto addget_namepos = [&]( string name,
            unordered_map <string, long> &namepos) {
         auto found = namepos.find(name);
         if (found==namepos.end()){
            if (namepos.size()<maxstored) {
               namepos[name]=(++lastassignedpos);
               names[lastassignedpos] = name;
               return lastassignedpos;
            }
            else return (long)-1;
         }
         else return namepos[name];
      };

      // fill the base collector with at most maxstored strongest nodes
      // cout<<"1"; cout.flush();
      unordered_map <string, long> namepos_buf;
      lastassignedpos=-1;
      long i_threshold;
      if (strengths_sorted.size()>maxstored) i_threshold=maxstored;
      else i_threshold=strengths_sorted.size()-1;
      double str_threshold = strengths_sorted[i_threshold].second;
      if (verbose>3) {
         cout<<"strengths_sorted:"<<endl;
         for (auto i=0; i<strengths_sorted.size(); i++)
            cout<<strengths_sorted[i].first<<" "
                <<strengths_sorted[i].second<<" ";
         cout<<endl;
      }
      // cout<<"2"; cout.flush();
      for (auto it=latest.begin(); it!=latest.end(); it++) {
         // cout<<"3"; cout.flush();
         name1 = it->name1;
         name2 = it->name2;
         weight = it->weight;
         id1 = namepos_all[name1];
         id2 = namepos_all[name2];
         str1 = strengths[id1];
         str2 = strengths[id2];
         newid1=newid2=-1;
         // cout<<"4"; cout.flush();
         if ( str1>=str_threshold ) {
            newid1 = addget_namepos(name1, namepos_buf);
            if (newid1>=0) net[newid1][newid1] += weight;
         }
         // cout<<"5"; cout.flush();
         if ( str2>=str_threshold ) {
            newid2 = addget_namepos(name2, namepos_buf);
            if (newid2>=0) net[newid2][newid2] += weight;
         }
         // cout<<"6"; cout.flush();
         if ( newid1>=0 && newid2>=0 ) {
            net[newid1][newid2] += weight;
            net[newid2][newid1] += weight;
         }
      }
      // cout<<"9"; cout.flush();
   }

   // no forgetting for this method
   void forget_connections (double forgetfactor) {}

private:

   // a fast pow, about 3 times faster than pow
   inline double fastpow(double a, double b) {
     // calculate approximation with fraction of the exponent
     int e = (int) b;
     union {
       double d;
       int x[2];
     } u = { a };
     u.x[1] = (int)((b - e) * (u.x[1] - 1072632447) + 1072632447);
     u.x[0] = 0;

     // exponentiation by squaring with the exponent's integer part
     // double r = u.d makes everything much slower, not sure why
     double r = 1.0;
     while (e) {
       if (e & 1) {
         r *= a;
       }
       a *= a;
       e >>= 1;
     }

     return r * u.d;
   }

   void apply_exp_decay () {
      // keep the size of the list smaller than the given number of links
      const int maxlinks = 1e6;
      // const int maxlinks = 2e5;
      if (latest.size()>maxlinks) {
         int i=0;
         auto limitingit = latest.begin();
         while( i<latest.size()-maxlinks ) { limitingit++; i++; }
         latest.erase( latest.begin(), limitingit );
      }

      // modify weight to account for eponential decay
      long latesttime=latest.back().ts;
      double invtimewindow = 1.0/timewindow;
      for (auto it=latest.begin(); it!=latest.end(); it++) {
         long timepassed = latesttime - it->ts;
         double decayfactor = pow( forgetconst,
            invtimewindow * ( timepassed  - timewindow*0.5 ) );
         it->weight = it->orgweigth * decayfactor;
      }
   }

   void apply_time_window () {
      long latesttime=latest.back().ts;
      list<link_timed>::iterator limitingit=latest.begin();
      while(latesttime - limitingit->ts > timewindow) limitingit++;
      latest.erase(latest.begin(), limitingit);
   }

private:

   const double timewindow;
   const double forgetconst;
   const unsigned verbose;
   const string viztype;

   list <link_timed> latest;
   clock_collectors *myclockcollector;
   unsigned nodes_number;
};



#endif