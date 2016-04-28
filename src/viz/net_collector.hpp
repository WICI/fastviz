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
#include <list>

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
			long ts=-1, int verbose=0) {
		typedef typename vector <string>::const_iterator ittype;

		double m=linkpack.size();
		list<set<node_base>::iterator> toupdate;
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
				toupdate.push_back(stored.find(node));
				myclockcollector->collect("TTTTfindinstored");

				// debug
				if (verbose>3) {
					cout<<"New node: "<<node.nm<<" weight="<<weight<<" str="<<nodeincrement<<endl;
					print_weakest();
					print_stored();
				}

				// if not among the stored nodes
				if (toupdate.back()==stored.end()) {

					// let's add the new node
					if (stored.size()<net.size()) {
						node.pos=stored.size();
						names[node.pos]=node.nm;
						net[node.pos][node.pos]=nodeincrement;
					}

					// or exchange the weakest node with the new one
					else {

						// assign the position
						assert(weakest.size()>0);
						node.pos=weakest.front();

						// erase from weakest, update stored names, and remove from stored names
						weakest.pop_front();
						{node_base weaknode; weaknode.nm=names[node.pos];
						set<node_base>::iterator foundit=stored.find(weaknode);
						assert(foundit!=stored.end());

						auto it=toupdate.begin();
						while ( it!=toupdate.end() ) {
							if (*it==foundit) {
								// cout<<"A node destined for update removed: "<<foundit->nm;
								// cout<<" while adding node: "<<node.nm<<endl;
								it=toupdate.erase(it);
							}
							else it++;
						}


						stored.erase(foundit);}
						names[node.pos]=node.nm;

						// update matrix of weights
						for (unsigned j=0; j<net.size(); j++) {
							net[j][node.pos]=0;
							net[node.pos][j]=0;
						}
						net[node.pos][node.pos]=nodeincrement;

						// in case of weakest empty find new weakest elements
						refill_weakest();
					}

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

					// and let's insert the new node
					pair<set<node_base>::iterator,bool> insres;
					insres=stored.insert(node);
					assert(insres.second);
					toupdate.back()=insres.first;

				}

				// if among the stored nodes then update the node strength and the weakest set
				else {

					// increment the score of the arriving node
					node.pos=(*toupdate.back()).pos;
					double prevstr=net[node.pos][node.pos];
					net[node.pos][node.pos]+=nodeincrement;

					// debug, shouldn't happen
					if (prevstr<minstr) {
						cout<<"Error: a node with strength lower than minstr "<<prevstr<<" < "<<minstr<<endl;
						print_weakest();
						print_stored();
					}

					// if it was in the weakest set then leave the set now
					if (prevstr==minstr) {
						{deque<unsigned>::iterator foundit=
							find( weakest.begin(), weakest.end(), node.pos);
						if (foundit!=weakest.end()) weakest.erase(foundit);}

						// in case of weakest empty find new weakest elements
						refill_weakest();
					}

					// debug, shouldn't happen
					else {
						deque<unsigned>::iterator foundit=
							find( weakest.begin(), weakest.end(), node.pos);
						if (foundit!=weakest.end()) {
							cout<<"Error: found among weakest a node with strength "<<prevstr<<" > "<<minstr<<endl;
							print_weakest();
							print_stored();
						}
					}

				}


				// debug
				if (verbose>3) {
					cout<<"After buffering: "<<names[node.pos]<<" pos="<<node.pos
						 <<" str="<<net[node.pos][node.pos]<<endl;
					print_weakest();
					print_stored();

					deque<unsigned>::iterator foundit=
						find( weakest.begin(), weakest.end(), node.pos);
					if (net[node.pos][node.pos]>minstr && foundit!=weakest.end()) {
						cout<<"This is a naughty node! (It's among the weakest but it's strength is high.)"<<endl;
					}

					cout<<endl;
				}

			}
		}


		//------------------------------------------------------------------
		// now it's time to strengthen connection weights of the arriving nodes
		if (toupdate.size()>1)
		{
			for (auto it1 = toupdate.begin(); it1 != toupdate.end(); it1++) {
				for (auto it2 = toupdate.begin(); it2 != toupdate.end(); it2++) {
					unsigned pos1=(**it1).pos;
					unsigned pos2=(**it2).pos;

					// debug
					//string nm1=(**it1).nm;
					//string nm2=(**it2).nm;
					if (pos1>net.size() || pos2>net.size()) {
						cout<<"Nodes ids too high: "<<pos1<<" or "<<pos2<<endl;
						cout.flush();
						// cout<<" for nodes: "<<(**it1).nm<<" or "<<(**it2).nm<<endl;
						cout.flush();
						cout<<"Whole linkpack:"<<endl;
						for (ittype fp = linkpack.begin(); fp != linkpack.end(); ++fp) {
								cout<<(*fp)<<" ";
								cout.flush();
						}
						cout<<"Whole toupdate list:"<<endl;
						for (auto k = toupdate.begin(); k != toupdate.end(); k++) {
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
		minstr*=forgetfactor;
		for (int i=0; i<net.size(); i++)
			for (int j=0; j<net[i].size(); j++)
				net[i][j]*=forgetfactor;
	}

private:

	void print_weakest() {
		cout<<"Weakest:";
		for (auto it=weakest.begin(); it!=weakest.end(); it++) {
			auto pos=(*it);
			cout<<" "<<names[pos]<<" "<<pos<<" "<<net[pos][pos]<<" | ";
		}
		cout<<endl;
	}

	void print_stored() {
		cout<<"Stored:";
		for (auto it=stored.begin(); it!=stored.end(); it++) {
			auto pos=it->pos;
			cout<<" "<<it->nm<<" "<<names[pos]<<" "<<pos<<" "<<net[pos][pos]<<" | ";
		}
		cout<<endl;
	}

	// in case of weakest empty find new weakest elements
	void refill_weakest() {
		if (weakest.size()==0) {
			double currminstr=1e100;
			for (auto it=stored.begin(); it!=stored.end(); it++) {
				auto pos=it->pos;
				if (net[pos][pos]<=currminstr) {
					if (net[pos][pos]<currminstr) {
						currminstr=net[pos][pos];
						weakest.clear();
						weakest.push_back(pos);
					}
					else weakest.push_back(pos);
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