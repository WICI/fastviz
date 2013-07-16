#ifndef VIZ_NET_COLLECTOR_HPP
#define VIZ_NET_COLLECTOR_HPP

#include <cassert>
#include <deque>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

#include <pms/clock_collector.cpp>
#include <viz/node.cpp>

using namespace std;

class net_collector_base {
public:	
	vector <string> snm;
	vector <vector <double> > sw;
	
	net_collector_base (const unsigned maxstored)
		:snm(maxstored), sw(maxstored, vector <double> (maxstored,0)) {
	}
	
	//template <class T1> void add_linkpack (T1 linkpack) {}
	//void forget_connections (double forgetfactor) {}
};

class net_collector : public net_collector_base {
public:
	
	net_collector (const unsigned maxstored, clock_collectors &mycc)
		:net_collector_base(maxstored) {
			minstr=1, prevminstr=1; nstored=0;
			myclockcollector=&mycc;
			verbose=1;
	}
	
	//void add_linkpack(T1 linkpack, string label, double r, double g, double b)
	template <class T1>	
	void add_linkpack (T1 linkpack, int scoretype=1) {
		vector<set<node_base>::iterator> toupdate(linkpack.size()); 
		//double increment=1.0/(toupdate.size()-1);
		double increment; //edge score increment
		double strincrement; //node score increment
		
      if (scoretype==1) {
         increment=2.0/(toupdate.size()-1)/toupdate.size();
         strincrement=2.0/toupdate.size();
      }
      if (scoretype==2) {
         increment=1.0/toupdate.size();
         strincrement=1.0;
      }
      if (scoretype==3) {
         increment=2.0/sqrt(toupdate.size()-1)/sqrt(toupdate.size());
         strincrement=2.0/sqrt(toupdate.size());
      }
      
      
      
		//------------------------------------------------------------------
		// check if found tags has already been stored, if not then add
		{
			unsigned iupd=0;
			typedef typename T1::const_iterator ittype;
			for (ittype fp = linkpack.begin(); fp != linkpack.end(); ++fp) {
				if (verbose==9) cout<<(*fp);
				
				// stored?
				node_base tag;
				tag.nm=(*fp);
				toupdate[iupd]=st.find(tag);
				myclockcollector->collect("TTTTfindinstored");
				
				// if not among the stored tags
				if (toupdate[iupd]==st.end()) {
					
					// let's add 
					if (nstored<sw.size()) {
						tag.pos=nstored;
						snm[tag.pos]=tag.nm;
						sw[tag.pos][tag.pos]=strincrement;
						
						// but if it's weak then it won't stay long...
						if (strincrement<=minstr) {
							if (strincrement<minstr) {
								weakest.clear();
								weakest.push_back(tag.pos);
								prevminstr=minstr;
								minstr=strincrement;
							}
							else {
								weakest.push_back(tag.pos);
							}
						}
					}						
					// or exchange the weakest one with the new one
					else {
						// erase one of the weakest nodes from stored names
						node_base temptag;
						assert(weakest.size()>0);
						temptag.nm=snm[weakest.front()];	//TODO think if it's the best method
						
						{set<node_base>::iterator foundit=st.find(temptag);
						if (foundit!=st.end()) st.erase(foundit);}
						
						// assign position, update stored names and remove from weakest
						tag.pos=weakest.front();
						snm[tag.pos]=tag.nm;							
						weakest.pop_front();
						
						// update matrix of weights
						for (unsigned j=0; j<sw.size(); j++) {
							if (sw[j][tag.pos]) {
								//TODO minstr should be updated here but I omit it to
								// have faster algorithm
								sw[j][j]-=sw[j][tag.pos];
								//if (sw[j][j]<minstr) {prevminstr=minstr; minstr=sw[j][j];}
								sw[j][tag.pos]=0;
								sw[tag.pos][j]=0;
                     }
                  }
                  sw[tag.pos][tag.pos]=strincrement;
                  
                  // in case of weakest empty find new weakest elements
                  //sw[tag.pos][tag.pos]=999999;							
                  if (weakest.size()==0) {
                     double currminstr=999999;
                     for (unsigned i=0; i<sw.size(); i++) {
                        if (sw[i][i]<=currminstr) {
                           if (sw[i][i]<currminstr) {
                              currminstr=sw[i][i];
                              weakest.clear();
                              weakest.push_back(i);
                           }
                           else weakest.push_back(i);
                        }
                     }
                     minstr=currminstr;
                  }							
               }
				
               // and let's insert the new one!
               pair<set<node_base>::iterator,bool> insres;
               insres=st.insert(tag);
               nstored++;
               assert(insres.second); //TODO to be commented out
               toupdate[iupd]=insres.first;
				
            } //TODO add limitations for the size of st
            else {
               // if in weakest set then will leave the set now
               if (sw[(*toupdate[iupd]).pos][(*toupdate[iupd]).pos]==minstr) {
                  //TODO wrong i need to find element with the position, not with strength
                  // so probably weakest has to be a deque of structs...
                  {deque<unsigned>::iterator foundit=find( weakest.begin(), weakest.end(), (*toupdate[iupd]).pos);
                  if (foundit!=weakest.end()) weakest.erase(foundit); }							
                  
                  // in case of weakest empty find new weakest elements							
                  if (weakest.size()==0) {
                     double currminstr=999999;
                     for (unsigned k=0; k<sw.size(); k++) {
                        if (sw[k][k]<=currminstr) {
                           if (sw[k][k]<currminstr) {
                              currminstr=sw[k][k];
                              weakest.clear();
                              weakest.push_back(k);
                           }
                           else weakest.push_back(k);
                        }
                     }
                     minstr=currminstr;
                  }
               }
               sw[(*toupdate[iupd]).pos][(*toupdate[iupd]).pos]+=strincrement;
            }
            iupd++;
         }
      }
	
	
		//------------------------------------------------------------------
		// now it's time to strengthen connections weights
		if (toupdate.size()>0)
		{
			for (vector<set<node_base>::iterator>::const_iterator i = toupdate.begin();
			i != toupdate.end(); ++i)
			for (vector<set<node_base>::iterator>::const_iterator j = toupdate.begin();
			j != toupdate.end(); ++j) {
				unsigned pos1=(**i).pos;
				unsigned pos2=(**j).pos;
				if (pos1!=pos2) sw[pos1][pos2]+=increment;
				//sw[pos1][pos1]+=increment; // this is done before
			}
		}
      
		myclockcollector->collect("TTTTaddedtostored");
		
		//boost::this_thread::sleep(boost::posix_time::milliseconds(p_sleep));
	}
	
	void forget_connections (double forgetfactor) {
		for (int i=0; i<sw.size(); i++) for (int j=0; j<sw[i].size(); j++)
			if (sw[i][j]!=0) sw[i][j]*=forgetfactor;
	}
	
private:
	set <node_base> st; //stored hashtags
	deque <unsigned> weakest;
	
	clock_collectors *myclockcollector;
	
	double minstr, prevminstr;
	unsigned nstored, verbose;
};



#endif