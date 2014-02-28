/*
 * Buffer a subgraph of the full graph of edges provided as the input
 */

#ifndef VIZ_NET_COLLECTOR_BASE_HPP
#define VIZ_NET_COLLECTOR_BASE_HPP

#include <vector>
#include <unordered_map>

using namespace std;

class net_collector_base {
public:

	net_collector_base (const unsigned maxstored)
		:names(maxstored), net(maxstored, vector <double> (maxstored,0)),
		maxstored(maxstored) {}

	void reset_collector_base_content () {
		for (int i=0; i<net.size(); i++)
			for (int j=0; j<net[i].size(); j++)
				net[i][j]=0;
		for (int i=0; i<names.size(); i++) names[i]="";
	}

   unsigned get_nodes_number() {
   	unsigned result;
		for (int i=0; i<names.size(); i++) if (names[i]!="") result++;
		return result;
   }

   double get_total_score() {
   	double result=0;
		for (int i=0; i<net.size(); i++)
			for (int j=0; j<net[i].size(); j++) if (i!=j)
				result += net[i][j];
		return result;
   }

   virtual void add_linkpack (
   	vector <string> &linkpack, double weight, long ts) = 0;
   virtual void update_net_collector_base () = 0;
   virtual void forget_connections (double forgetfactor) = 0;

	const unsigned maxstored; // 20000 corresponds to around 4gb of memory
	vector <string> names;
	vector <vector <double> > net;

};

#endif