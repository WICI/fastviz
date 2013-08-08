#ifndef VIZ_VIZ_SELECTOR_HPP
#define VIZ_VIZ_SELECTOR_HPP

#include <cmath>
#include <list>
#include <set>
#include <vector>

#include <pms/clock_collector.hpp>
#include <viz/client.hpp>
#include <viz/node.hpp>
#include <viz/net_collector.hpp>

using namespace std;

/*
 * Select for the visualization a subgraph of the buffered subraph
 */

class viz_selector_base {
public:
	virtual void draw (const unsigned maxvisualized, double edgeminweight, 
                      int verbose=0, string excluded="",
							 double r=0.5, double g=0.5, double b=0.5) {};

	void add_labels(string datetime="label not specified",
                   string label1="label not specified", 
                   string label2="label not specified", 
						 string producer="truthy.indiana.edu") {
		oc->set_attributes( "label",datetime);
		oc->add_label("datetime");
		oc->set_attributes( "label",label1, "x",5, "y",55, "size",50 );
		oc->add_label("label1");
		oc->set_attributes( "label",label2, "x",5, "y",55+40, "size",50 );
		oc->add_label("label2");
		// uhm, it's strange to have these attribs here, since this is not generic, but well... 
		// we're not sure what would be a better soulution, that's it
		oc->set_attributes( "label",producer, "x",1050, "y",715, "size",20 );
		oc->add_label("producer");
	}
	
	void change_label_datetime(string datetime) {
		oc->set_attributes( "label",datetime);
		oc->change_label("datetime");
	}
	
   // this is purelly for the purpose of aggregated statistics
   // and slows down the algorithm, so it shouldn't be used in general
   virtual long get_how_many_drawn() {
      return -1;
   }
   
protected:
	client_base *oc; // output client
	unsigned eid;
	
   // create a list of all stored nodes sorted by strength and select the strongest
	template <class T0> //TODO is template here needed?
	void select_nodes(net_collector_base *network, const unsigned maxvisualized, 
							vector <T0> &vntmp, double edgeminweight, string excluded="" ) {
		//TODO rewrite to a new container, which can be sorted with different keys
		vector<T0> sntmp; //TODO this should be a list
		T0 tmpnode;
		for (int i=0; i<network->sw.size(); i++) {
			//DANGER what happens when snm is not yet initialized (initialized
         // with default values), e.g. when number of stored nodes is low
         tmpnode.nm=network->snm[i];
			tmpnode.pos=i;
			tmpnode.str=network->sw[i][i];
			sntmp.push_back(tmpnode);
		}
		sort ( sntmp.begin(), sntmp.end(), compare_node_strength<T0> );
		// for (int i=0; i<sntmp.size(); i++) cout<<sntmp[i].nm<<" "<<network->sw[sntmp[i].pos][sntmp[i].pos]<<" ";
		// cout<<endl;
		
		// select strongest and filter out singletons, and late excluded node
		unsigned currvisualized;
		if (sntmp.size()>maxvisualized) currvisualized=maxvisualized;
		else currvisualized=sntmp.size();
		{vector <T0> vnstrongest(sntmp.end()-currvisualized,sntmp.end());
		for (int i=0; i<vnstrongest.size(); i++) {
			//double str=0;
			int edges=0;
			for (int j=0; j<vnstrongest.size(); j++) if (i!=j) edges+=(
				network->sw[vnstrongest[i].pos][vnstrongest[j].pos]
				>
				edgeminweight
				);
			if (edges>0 && vnstrongest[i].nm!=excluded ) vntmp.push_back(vnstrongest[i]);
		}}
		
		// sort according to the name in order to compare with previous state
		sort ( vntmp.begin(), vntmp.end() );		
	}
	
   
   // clears ids of the edges removed from the visualization
   // eidm is a matrix of size of sw matrix, to save processor time,
   // probably it could be implemented differently, no it's memory expensive
	template <class T0, class T1>
	static void clean_edgeids(T0 &prevvisn, T1 &eidm, typename T0::iterator &node_deleted) {
		for (int i=0; i<prevvisn.size(); i++) {
			eidm[node_deleted->pos][prevvisn[i].pos]=0;
			eidm[prevvisn[i].pos][node_deleted->pos]=0;
		}
	}
	
   // makes differential comparison of what has been visualized and what will
   // be visualized, and sends the changes to the output client
	template <class T0, class T1, class T2, class F>
	void adddelete_nodes(T0 &prevvisn, T0 &vntmp, T1 &visn, T2 &eidm, F f ) {
		typedef typename T0::iterator ittype0;
		typedef typename T1::iterator ittype1;
		typedef typename T1::value_type vtype1;
		ittype0 first1, last1, first2, last2;
		first1=prevvisn.begin(); last1=prevvisn.end();
		first2=vntmp.begin(); last2=vntmp.end();
		// layout - red/blue
		//oc->set_attributes("r",1, "g",0.5, "b",0.5, "label",(*first2).nm);
		// layout - yellow/blue
		//oc->set_attributes("r",1, "g",1, "b",0, "label",(*first2).nm);
		while (first1!=last1 && first2!=last2)
		{
         // remove node from visualization
			if ((*first1).nm<(*first2).nm) {
				oc->delete_node((*first1).nm);
				f(prevvisn, eidm, first1);
				{ittype1 foundit=find(visn.begin(),visn.end(),(*first1).nm);
				assert(foundit!=visn.end()); visn.erase(foundit);}
				++first1;
			}
         // add node and all its edges to visualization
			else if ((*first2).nm<(*first1).nm) {
				oc->set_attributes("r",1, "g",1, "b",0, "label",(*first2).nm);
				oc->add_node((*first2).nm);
				{vtype1 tmpnode=(*first2); visn.push_back(tmpnode); visn.back().counter=1;}
				++first2;						
			}
         // update outgoing edge weights of the node     
			else {				
				first1++; first2++;
			}
		}
      // remove node from visualization
		while (first1!=last1) {
			oc->delete_node((*first1).nm);
			f(prevvisn, eidm, first1);
			{ittype1 foundit=find(visn.begin(),visn.end(),(*first1).nm);
			assert(foundit!=visn.end()); visn.erase(foundit);}
			++first1;
		}
      // add node and all its edges to visualization
		while (first2!=last2) {
			oc->set_attributes("r",1, "g",1, "b",0, "label",(*first2).nm);
			oc->add_node((*first2).nm);
			{vtype1 tmpnode=(*first2); visn.push_back(tmpnode); visn.back().counter=1;}
			++first2;						
		}		
	}
	
   // sends to the output client changes in node sizes and colors
	template <class T0>
	void change_nodes(net_collector_base *network, T0 &visn, string excluded="") {
		typedef typename T0::iterator itype;
		for (itype i=visn.begin(); i!=visn.end(); i++) {
			if (i->nm!=excluded) {
				// layout - red/blue
				//oc->set_attributes( "r",(*i).counter, "g",0.5, "b",1.4999-(*i).counter,
				//						 "size",5*sqrt(network->sw[(*i).pos][(*i).pos]) );
				// layout - yellow/blue
				//oc->set_attributes( "r",(*i).counter, "g",(*i).counter, "b",2-2*(*i).counter,
				//						  "size",5*sqrt(network->sw[(*i).pos][(*i).pos]) );
				oc->set_attributes( "r",0.0, "g",0.2, "b",0.8,
										  "size",5*sqrt(network->sw[(*i).pos][(*i).pos]) );
			}
			//else {oc->set_attributes( "r",1, "g",0.5, "b",0.5, "size",5*6 );}
			oc->change_node((*i).nm);
			if ((*i).counter>0.5) (*i).counter-=0.02;
			//(*i).counter=0.95*(*i).counter;
		}
	}
	
	template <class T0>
	static unsigned extract_position(const T0 &node_object) {
		return node_object.pos;
	}
	
   // sends to the output client changes in edge weights and colors
	template <class T0, class T1, class F>
	void change_edges(net_collector_base *network, T0 &visn, T1 &eidm, F extractpos,
			double edgeminweight=0.0001, double r=0.5, double g=0.5, double b=0.5) {
		typedef typename T0::iterator itype;
		for (itype i=visn.begin(); i!=visn.end(); i++)
			for (itype j=visn.begin(); j!=visn.end(); j++) {
				if (network->sw[extractpos(*i)][extractpos(*j)]>edgeminweight) if (i!=j) {
					if (eidm[extractpos(*i)][extractpos(*j)]) {
						oc->set_attributes( "weight",network->sw[extractpos(*i)][extractpos(*j)], 
							"r",r, "g",g, "b",b );
						oc->change_edge( eidm[extractpos(*i)][extractpos(*j)] );
					}
					else {
						oc->set_attributes( "source",(*i).nm, "target",network->snm[extractpos(*j)],
												  "directed",false, "weight",network->sw[extractpos(*i)][extractpos(*j)],
												  "r",r, "g",g, "b",b );
						oc->add_edge( eid );
						eidm[extractpos(*i)][extractpos(*j)]=eid;
						eidm[extractpos(*j)][extractpos(*i)]=eid;
						eid++;
					}
				}
			}
	}
	
};


class viz_selector: public viz_selector_base {
public:
   
	viz_selector (net_collector_base &mynet, client_base &client, clock_collectors &mycc)
		:eidm(mynet.sw.size(), vector <unsigned long> (mynet.sw.size(),0))
	{
		network=&mynet;
		oc=&client;
		myclockcollector=&mycc;
		eid=1;
		verbose=1;
	}
	
   // this is purelly for the purpose of aggregated statistics
   // and slows down the algorithm, so it shouldn't be used in general
   long get_how_many_drawn() {
      return allnodes_drawn.size();
   }
   
	void change_net(net_collector_base &mynet) {
		network=&mynet;
	}
	
   // the main method, calling all the private methods
	void draw (const unsigned maxvisualized, double edgeminweight, 
               int verbose=0, string excluded="", 
					double r=0.5, double g=0.5, double b=0.5) {
		if (verbose==5) cout<<"___________________________________________"<<endl;
		vector<node_the> vntmp;
		select_nodes(network, maxvisualized, vntmp, edgeminweight, excluded );
		myclockcollector->collect("TTTTselect_nodes");
		
		//TODO I would prefer to give the function as argument without a need
		// of specifying the template, but I'm not sure if it's possible
		adddelete_nodes(prevvisn, vntmp, visn, eidm, 
			clean_edgeids < vector <node_the>, vector <vector <unsigned long> > > );
		if (verbose==5) {
			for (int i=0; i<prevvisn.size(); i++) cout<<prevvisn[i].nm<<" "; cout<<endl;
			for (int i=0; i<vntmp.size(); i++) cout<<vntmp[i].nm<<" "; cout<<endl;
			for (list<node_with_counter>::iterator i=visn.begin(); i!=visn.end(); i++) cout<<(*i).nm<<" ";
			cout<<endl;
		}
		myclockcollector->collect("TTTTadddelete_nodes");

		change_nodes( network, visn, excluded );
		change_edges( network, visn, eidm, extract_position<node_with_counter>, 
						  edgeminweight, 0.4, 0.6, 0.8 );
		swap(prevvisn,vntmp);
		myclockcollector->collect("TTTTupdate_nodes_edges");
		
      if (verbose>0) allnodes_drawn.insert( visn.begin(), visn.end() );
      
		oc->update();
		myclockcollector->collect("TTTTgcupdate");
	}
	
private:
	net_collector_base *network;
	
	vector <node_the> prevvisn;
	list <node_with_counter> visn;
	vector <vector <unsigned long> > eidm;
   
   // this is purelly for the purpose of aggregated statistics
   // and slows down the algorithm, so it shouldn't be used in general
   set <node_with_counter> allnodes_drawn;
   
	clock_collectors *myclockcollector;
	
	unsigned verbose;
};


#endif