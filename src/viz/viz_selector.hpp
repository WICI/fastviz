/*
 * Select for the visualization a subgraph of the buffered subraph
 */

#ifndef VIZ_VIZ_SELECTOR_HPP
#define VIZ_VIZ_SELECTOR_HPP

#include <cmath>
#include <list>
#include <set>
#include <vector>

#include <igraph.h>

#include <pms/std_to_igraph.cpp>
#include <pms/clock_collector.hpp>
#include <viz/client.hpp>
#include <viz/node.hpp>
#include <viz/net_collector.hpp>

using namespace std;

class viz_selector_base {
public:
	virtual void draw (const unsigned maxvisualized, double edgeminweight,
                      string excluded="", bool hide_singletons=true) {};

	void add_labels(string datetime="",
                   string label1="",
                   string label2="",
						 string producer="") {
		oc->set_attributes( "label",datetime);
		oc->add_label("datetime");
		oc->set_attributes( "label",label1, "x",5, "y",55, "size",50 );
		oc->add_label("label1");
		oc->set_attributes( "label",label2, "x",5, "y",55+40, "size",50 );
		oc->add_label("label2");
		oc->set_attributes( "label",producer, "x",1050, "y",715, "size",20 );
		oc->add_label("producer");
	}

	void change_label_datetime(string datetime) {
		oc->set_attributes( "label",datetime);
		oc->change_label("datetime");
	}

   // get aggregated statistics
   virtual long get_how_many_drawn()  {};
	unsigned get_nodes_visualized(){ return nodes_visualized; }
	unsigned get_nodes_not_visualized(){ return nodes_not_visualized; }
	double get_total_score(){ return total_score; }
 	virtual void get_netsstats(char *output) {};

protected:

   // create a list of all stored nodes sorted by strength and select the strongest
	template <class T0>
	void select_nodes(net_collector_base *netcol, const unsigned maxvisualized,
							vector <T0> &vntmp, double edgeminweight,
							string excluded="", bool hide_singletons=true ) {
		// get all buffered nodes and sort them by strength
		vector<T0> bnodes;
		T0 tmpnode;
		for (int i=0; i<netcol->maxstored; i++) if (netcol->names[i]!="") {
         tmpnode.nm=netcol->names[i];
			tmpnode.pos=i;
			tmpnode.str=netcol->net[i][i];
			bnodes.push_back(tmpnode);
		}
		sort ( bnodes.begin(), bnodes.end(), compare_node_strength<T0> );
  		// debugging
		// if (verbose>4){
		// 	for (int i=0; i<bnodes.size(); i++)
		// 		cout<<bnodes[i].nm<<" "<<
		// 			 netcol->net[bnodes[i].pos][bnodes[i].pos]<<" ";
		// 	cout<<endl;
		// 	for (int i=0; i<10; i++)
		// 		cout<<netcol->names[i]<<" "<<netcol->net[i][i]<<" ";
		// }

		// select strongest nodes
		unsigned currvisualized;
		if (bnodes.size()>maxvisualized) currvisualized=maxvisualized;
		else currvisualized=bnodes.size();
		{vector <T0> bnstrongest(bnodes.end()-currvisualized,bnodes.end());

		// filter out singletons, and late excluded node
		nodes_visualized=nodes_not_visualized=total_score=0;
		for (int i=0; i<bnstrongest.size(); i++) {
			int edges=0;
			for (int j=0; j<bnstrongest.size(); j++) if (i!=j) {
				double weight = netcol->net[bnstrongest[i].pos][bnstrongest[j].pos];
				total_score+=weight;
				edges+=(weight>edgeminweight);
			}
			if ( bnstrongest[i].nm!=excluded ) {
				if (hide_singletons) {
					if (edges>0) {
						vntmp.push_back(bnstrongest[i]);
						nodes_visualized++;
					}
					else nodes_not_visualized++;
				}
				else {
					vntmp.push_back(bnstrongest[i]);
					nodes_visualized++;
				}
			}
			else nodes_not_visualized++;
		}}

		// sort according to the name in order to compare with previous state
		sort ( vntmp.begin(), vntmp.end() );
	}


   // clears ids of the edges removed from the visualization
   // eidm is a matrix of size of net matrix, to save processor time,
   // it could be implemented differently, now it's very memory expensive
	template <class T0, class T1>
	static void clean_edgeids(T0 &prevvisn, T1 &eidm,
			typename T0::iterator &node_deleted) {
		for (int i=0; i<prevvisn.size(); i++) {
			eidm[node_deleted->pos][prevvisn[i].pos]=0;
			eidm[prevvisn[i].pos][node_deleted->pos]=0;
		}
	}

   // makes differential comparison of prevvisn and vntmp,
   // namely of what has been visualized and what will
   // be visualized, and sends the changes to the output client
	template <class T1, class T2, class T3, class F>
	void adddelete_nodes(T1 &prevvisn, T2 &vntmp, T3 &eidm,
			F cleaner_function ) {
		typedef typename T1::iterator ittype1;
		typedef typename T2::iterator ittype2;
		ittype1 first1, last1;
		ittype2 first2, last2;
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
				cleaner_function(prevvisn, eidm, first1);
				++first1;
			}
         // add node and all its edges to visualization
			else if ((*first2).nm<(*first1).nm) {
				oc->set_attributes("r",1, "g",1, "b",0, "label",(*first2).nm);
				oc->add_node((*first2).nm);
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
			cleaner_function(prevvisn, eidm, first1);
			++first1;
		}
      // add node and all its edges to visualization
		while (first2!=last2) {
			oc->set_attributes("r",1, "g",1, "b",0, "label",(*first2).nm);
			oc->add_node((*first2).nm);
			++first2;
		}
	}

   // sends to the output client changes in node sizes and colors
	template <class T0>
	void change_nodes(net_collector_base *netcol, T0 &visn, string excluded="") {
		typedef typename T0::iterator itype;
		for (itype i=visn.begin(); i!=visn.end(); i++) {
			if (i->nm!=excluded) {
				oc->set_attributes( "r",0.0, "g",0.2, "b",0.8,
										  "size",5*sqrt(netcol->net[(*i).pos][(*i).pos]) );
			}
			oc->change_node((*i).nm);
		}
	}

	template <class T0>
	static unsigned extract_position(const T0 &node_object) {
		return node_object.pos;
	}

   // sends to the output client changes in edge weights and colors
	template <class T0, class T1, class F>
	void change_edges(net_collector_base *netcol, T0 &visn,
							T1 &eidm, F extractpos, double edgeminweight=0.0001,
							double r=0.5, double g=0.5, double b=0.5) {
		typedef typename T0::iterator itype;
		for (itype i=visn.begin(); i!=visn.end(); i++)
			for (itype j=visn.begin(); j!=visn.end(); j++) {
				if (netcol->net[extractpos(*i)][extractpos(*j)]>edgeminweight)
				if (i!=j) {
					if (eidm[extractpos(*i)][extractpos(*j)]) {
						oc->set_attributes(
							"weight",netcol->net[extractpos(*i)][extractpos(*j)],
							"r",r, "g",g, "b",b );
						oc->change_edge( eidm[extractpos(*i)][extractpos(*j)] );
					}
					else {
						oc->set_attributes(
							"source",(*i).nm,
							"target",netcol->names[extractpos(*j)],
							"directed",false,
							"weight",netcol->net[extractpos(*i)][extractpos(*j)],
							"r",r, "g",g, "b",b );
						oc->add_edge( eid );
						eidm[extractpos(*i)][extractpos(*j)]=eid;
						eidm[extractpos(*j)][extractpos(*i)]=eid;
						eid++;
					}
				}
			}
	}

	client_base *oc; // output client
	unsigned eid;

protected:
	unsigned verbose;

private:
	unsigned nodes_visualized, nodes_not_visualized;
	double total_score;
};


class viz_selector: public viz_selector_base {
public:

	viz_selector (net_collector_base &mynet, client_base &client,
			clock_collectors &mycc, int verbose) :
			eidm(mynet.maxstored, vector <unsigned long> (mynet.maxstored,0)) {
		netcol=&mynet;
		oc=&client;
		myclockcollector=&mycc;
		eid=1;
		this->verbose=verbose;
	}


   // the main method, calling all the private methods
	void draw (const unsigned maxvisualized, double edgeminweight,
               string excluded="", bool hide_singletons=true ) {
		if (verbose>5) cout<<"___________________________________________"<<endl;

		// selects nodes from netcol, removes singletons, sorts them, and
		// puts them into vntmp
		vector<node_the> vntmp;
		select_nodes(netcol, maxvisualized, vntmp, edgeminweight,
			excluded, hide_singletons );
		myclockcollector->collect("TTTTselect_nodes");

		adddelete_nodes(prevvisn, vntmp, eidm,
			clean_edgeids < vector <node_the>, vector <vector <unsigned long> > > );
		if (verbose>5) {
			for (int i=0; i<prevvisn.size(); i++) cout<<prevvisn[i].nm<<" ";
				cout<<endl;
			for (int i=0; i<vntmp.size(); i++) cout<<vntmp[i].nm<<" ";
				cout<<endl;
			cout<<endl;
		}
		myclockcollector->collect("TTTTadddelete_nodes");

		change_nodes( netcol, vntmp, excluded );
		change_edges( netcol, vntmp, eidm, extract_position<node_the>,
						  edgeminweight, 0.4, 0.6, 0.8 );
		if (verbose>4) {
			cout<<"nodes visualized (draw): ";
			for (vector <node_the>::iterator it=vntmp.begin(); it!=vntmp.end(); it++)
				cout<<it->nm<<","<<it->pos<<" ";
			cout<<endl;
		}
   	if (verbose>0) allnodes_drawn.insert( vntmp.begin(), vntmp.end() );
		swap(prevvisn,vntmp);
		myclockcollector->collect("TTTTupdate_nodes_edges");

		oc->update();
		myclockcollector->collect("TTTTgcupdate");

	}

   // get networks statistics
 	void get_netsstats(char *output) {
		netstats ns_buf, ns_viz;
		igraph_t g;
		igraph_vector_t weights;

		// get the visualized graph and its properties
		vector <vector <double> > viznet;
		get_visualized_net( viznet );
		igraph_vector_init(&weights, 0);
		vv_to_igraph(viznet, g, weights);
		ns_viz = get_netstats(g, weights);
		igraph_vector_destroy(&weights);
		igraph_destroy(&g);

		// get the buffered graph and its properties
		igraph_vector_init(&weights, 0);
		vv_to_igraph(netcol->net, g, weights);
		ns_buf = get_netstats(g, weights);
		igraph_vector_destroy(&weights);
		igraph_destroy(&g);

		sprintf( output,
            "avgdeg_buffered=%9.2f, avgdeg_visualized=%9.2f, "
            "avgstr_buffered=%9.2f, avgstr_visualized=%9.2f, "
            "ccglo_buffered=%6.3f, ccglo_visualized=%6.3f, "
            "ccloc_buffered=%6.3f, ccloc_visualized=%6.3f, "
            "assdeg_buffered=%6.3f, assdeg_visualized=%6.3f, "
            "assstr_buffered=%6.3f, assstr_visualized=%6.3f",
            ns_buf.avgdeg, ns_viz.avgdeg,
            ns_buf.avgstr, ns_viz.avgstr,
            ns_buf.ccglo, ns_viz.ccglo,
            ns_buf.ccloc, ns_viz.ccloc,
            ns_buf.assdeg, ns_viz.assdeg,
            ns_buf.assstr, ns_viz.assstr );

   }

private:

	// get the visualized graph and its properties
   void get_visualized_net( vector <vector <double> > &viznet ) {
		// vector<unsigned> nodesdrawn;
		// for (vector <node_the>::iterator it=prevvisn.begin();
		// 	it!=prevvisn.end(); it++) nodesdrawn.push_back( it->pos );
		// if (verbose>4) {
		// 	cout<<"nodes visualized (get_netsstats): ";
		// 	for (vector <node_the>::iterator it=prevvisn.begin();
		// 		it!=prevvisn.end(); it++) cout<<it->nm<<","<<it->pos<<" ";
		// 	cout<<endl;
		// }

		for (int i=0; i<prevvisn.size(); i++) {
			vector <double> curnode;
			for (int j=0; j<prevvisn.size(); j++)
				curnode.push_back( netcol->net[prevvisn[i].pos][prevvisn[j].pos] );
			viznet.push_back( curnode );
		}

      if (verbose>3) {
         cout<<"viznet network (full):"<<endl;
         for (int i=0; i<viznet.size(); i++) cout<<prevvisn[i].nm<<" ";
         cout<<endl;
         for (int i=0; i<viznet.size(); i++) {
            for (int j=0; j<viznet.size(); j++)
               cout<<viznet[i][j]<<" ";
            cout<<endl;
         }
      }
   }

   struct netstats {
   	netstats() {};
   	netstats( double avgdeg, double avgstr, double ccglo, double ccloc,
   			double assdeg, double assstr ){
   		this->avgdeg = avgdeg;
   		this->avgstr = avgstr;
   		this->ccglo = ccglo;
   		this->ccloc = ccloc;
   		this->assdeg = assdeg;
   		this->assstr = assstr;
   	}
   	double avgdeg, avgstr, ccglo, ccloc, assdeg, assstr;
 	   netstats& operator=(const netstats &other) {
   		this->avgdeg = other.avgdeg;
   		this->avgstr = other.avgstr;
   		this->ccglo = other.ccglo;
   		this->ccloc = other.ccloc;
   		this->assdeg = other.assdeg;
   		this->assstr = other.assstr;
   	}
   };

   // get networks statistics
 	netstats get_netstats( igraph_t &g, igraph_vector_t &weights,
 			igraph_bool_t directed = false ) {

		// get degree
		igraph_real_t avgdeg;
		igraph_vector_t degrees;
		igraph_vector_init(&degrees, 0);
		igraph_degree( &g, &degrees, igraph_vss_all(), IGRAPH_ALL, directed);
		for (int i=0; i<igraph_vector_size(&degrees); i++)
			avgdeg+=VECTOR(degrees)[i];
		cout<<"degree: "<<avgdeg<<" "<<igraph_vector_size(&degrees)<<endl;
		avgdeg /= 1.0*igraph_vector_size(&degrees);

		// get strength
		igraph_real_t avgstr;
		igraph_vector_t strengths;
		igraph_vector_init(&strengths, 0);
		igraph_strength( &g, &strengths, igraph_vss_all(), IGRAPH_ALL, directed, &weights);
		for (int i=0; i<igraph_vector_size(&strengths); i++)
			avgstr+=VECTOR(strengths)[i];
		avgstr /= 1.0*igraph_vector_size(&strengths);

		// get cc
		igraph_real_t ccglo;
		igraph_transitivity_undirected( &g, &ccglo,
			IGRAPH_TRANSITIVITY_ZERO );
		igraph_real_t ccloc;
		igraph_transitivity_avglocal_undirected( &g, &ccloc,
			IGRAPH_TRANSITIVITY_ZERO );

		// get assortativity
		igraph_real_t assdeg, assstr;
		igraph_assortativity_nominal( &g, &degrees, &assdeg, directed );
		igraph_assortativity_nominal( &g, &strengths, &assstr, directed );

		igraph_vector_destroy(&degrees);
		igraph_vector_destroy(&strengths);

		netstats output( (double)avgdeg, (double)avgstr, (double)ccglo,
							  (double)ccloc, (double)assdeg, (double)assstr );
		return output;
   }

   // get aggregated statistics
   long get_how_many_drawn() { return allnodes_drawn.size(); }

private:
	net_collector_base *netcol;

   // main containers
	vector <node_the> prevvisn;
	vector <vector <unsigned long> > eidm;

   // used only for the purpose of aggregated statistics
   set <node_the> allnodes_drawn;

	clock_collectors *myclockcollector;
};


#endif