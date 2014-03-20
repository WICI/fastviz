/*
 * Buffer a subgraph of the full graph of edges provided as the input
 */

#ifndef VIZ_NET_COLLECTOR_HPP
#define VIZ_NET_COLLECTOR_HPP

#include <cassert>
#include <deque>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

#include <pms/clock_collector.hpp>
#include <viz/node.hpp>
#include <viz/net_collector_base.hpp>

using namespace std;

class net_collector : public net_collector_base {
public:

	net_collector (const unsigned maxstored, clock_collectors &mycc,
			unsigned verbose=1)
		:net_collector_base(maxstored) {
			minstr=1e100;
			nstored=0;
     		this->verbose=verbose;
			myclockcollector=&mycc;
	}

	void add_linkpack (vector <string> &linkpack, double weight=1,
			long ts=-1) {
		typedef typename vector <string>::const_iterator ittype;

		double m=linkpack.size();
		vector<set<node_base>::iterator> toupdate(m);
		double edgeincrement = weight; //edge score
		double nodeincrement; //node score

		// if (weight==1) { edgeincrement=2.0/(m-1)/m; nodeincrement=2.0/m; }
		// if (weight==2) { edgeincrement=1.0/m; nodeincrement=1.0; }
		// if (weight==3) { edgeincrement=2.0/sqrt(m-1)/sqrt(m); nodeincrement=2.0/sqrt(m); }

		// if (weight==1) edgeincrement=2.0/(m-1)/m;
		// if (weight==2) edgeincrement=2.0/(m-1);

		nodeincrement = edgeincrement*(m-1);

		//------------------------------------------------------------------
		// check if found nodes has already been stored, if not then add
		{
			unsigned iupd=0;
			for (ittype fp = linkpack.begin(); fp != linkpack.end(); ++fp) {
				// if (verbose==9) cout<<(*fp);

				// stored?
				node_base node;
				node.nm=(*fp);
				toupdate[iupd]=stored.find(node);
				myclockcollector->collect("TTTTfindinstored");

				// if not among the stored nodes
				if (toupdate[iupd]==stored.end()) {

					// let's add
					if (nstored<net.size()) {
						node.pos=nstored;
						names[node.pos]=node.nm;
						net[node.pos][node.pos]=nodeincrement;

						// but if it's weak then it won't stay long...
						if (nodeincrement<=minstr) {
							if (nodeincrement<minstr) {
								minstr=nodeincrement;
								weakest.clear();
								weakest.push_back(node.pos);
							}
							else {
								weakest.push_back(node.pos);
							}
						}
					}
					// or exchange the weakest one with the new one
					else {
						// assign the position
						assert(weakest.size()>0);
						node.pos=weakest.front();

						// erase from weakest, update stored names, and remove from stored names
						weakest.pop_front();
						names[node.pos]=node.nm;
						{node_base tmpnode; tmpnode.nm=names[node.pos];
						set<node_base>::iterator foundit=stored.find(tmpnode);
						if (foundit!=stored.end()) stored.erase(foundit);}

						// update matrix of weights
						for (unsigned j=0; j<net.size(); j++) {
							if (net[j][node.pos]) {
								//net[j][j]-=net[j][node.pos];
								//if (net[j][j]<minstr) refresh_weakest();
								net[j][node.pos]=0;
								net[node.pos][j]=0;
							}
						}
						net[node.pos][node.pos]=nodeincrement;

						// in case of weakest empty find new weakest elements
						refresh_weakest();
					}

					// and let's insert the new one!
					pair<set<node_base>::iterator,bool> insres;
					insres=stored.insert(node);
					nstored++;
					assert(insres.second); //TODO to be commented out
					toupdate[iupd]=insres.first;

				} //TODO add limitations for the size of stored
				// if among the stored nodes then update the node strength and the weakest set
				else {
					// increment the score of the arriving node
					double prevstr=net[(*toupdate[iupd]).pos][(*toupdate[iupd]).pos];
					net[(*toupdate[iupd]).pos][(*toupdate[iupd]).pos]+=nodeincrement;
					if (prevstr==minstr) {
						// if it was in the weakest set then leave the set now
						{deque<unsigned>::iterator foundit=
							find( weakest.begin(), weakest.end(), (*toupdate[iupd]).pos);
						if (foundit!=weakest.end()) weakest.erase(foundit);}

						// in case of weakest empty find new weakest elements
						refresh_weakest();
					}
				}
				iupd++;
			}
		}


		//------------------------------------------------------------------
		// now it's time to strengthen connection weights of the arriving nodes
		if (toupdate.size()>1)
		{
			for (vector<set<node_base>::iterator>::const_iterator i = toupdate.begin();
				  i != toupdate.end(); i++) {
				for (vector<set<node_base>::iterator>::const_iterator j = toupdate.begin();
					  j != toupdate.end(); j++) {
					unsigned pos1=(**i).pos;
					unsigned pos2=(**j).pos;
					// debugging
					//string nm1=(**i).nm;
					//string nm2=(**j).nm;
					if (pos1>net.size() || pos2>net.size()) {
						for (ittype fp = linkpack.begin(); fp != linkpack.end(); ++fp) {
								cout<<(*fp)<<" ";
								cout.flush();
						}
						for (vector<set<node_base>::iterator>::const_iterator k = toupdate.begin();
							k != toupdate.end(); k++) {
								cout<<(**k).nm<<" "<<(**k).pos<<"|";
								cout.flush();
						}
						cout<<endl;
					}

					if (pos1!=pos2) net[pos1][pos2]+=edgeincrement;
					//net[pos1][pos1]+=edgeincrement; // this is done before
				}
			}
		}

		myclockcollector->collect("TTTTaddedtostored");

   // debugging
   // cout<<"add_linkpack verbose: "<<verbose<<endl;
   // if (verbose>5) {
   // 	cout<<"add_linkpack: ";
   //    for (int i=0; i<10; i++)
   //       cout<<names[i]<<" "<<net[i][i]<<" ";
   // }

		//boost::this_thread::sleep(boost::posix_time::milliseconds(p_sleep));
	}

	// no need to do anything, net_collector_base is already up-to-date
	void update_net_collector_base () {}

	// forgetting
	void forget_connections (double forgetfactor) {
		for (int i=0; i<net.size(); i++)
			for (int j=0; j<net[i].size(); j++)
				net[i][j]*=forgetfactor;
	}

private:

	// in case of weakest empty find new weakest elements
	void refresh_weakest() {
		if (weakest.size()==0) {
			double currminstr=1e100;
			for (unsigned k=0; k<net.size(); k++) {
				if (net[k][k]<=currminstr) {
					if (net[k][k]<currminstr) {
						currminstr=net[k][k];
						weakest.clear();
						weakest.push_back(k);
					}
					else weakest.push_back(k);
				}
			}
			minstr=currminstr;
		}
	}

	set <node_base> stored; //stored nodes
	deque <unsigned> weakest;

	clock_collectors *myclockcollector;

	double minstr;
	unsigned nstored, verbose;
};



#endif