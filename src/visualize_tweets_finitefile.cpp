/*
 * The main code of the first visualization tool that selects nodes
 * with the highest scores in order to visualize them as a smaller
 * subnetwork.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>

#include <boost/program_options.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/local_time_adjustor.hpp>

#include <util/time_checker.hpp>
#include <util/pace_checker.hpp>

#include <pms/clock_collector.hpp>
#include <pms/time_checker_intervals.hpp>

#include <viz/client.hpp>
#include <viz/client_gephi.hpp>
#include <viz/net_collector.hpp>
#include <viz/net_collector_timewindow.cpp>
#include <viz/viz_selector.hpp>

using namespace std;
namespace pt = boost::posix_time;

//=====================================================================
// global variable used by signal handler
//=====================================================================
bool keep_going = true;

void handle_kill(int sig) {
   keep_going = false;
   // unregister myself
   signal(SIGINT, SIG_DFL);
}

//=====================================================================
// loads a pack of links from a line of file
//=====================================================================
void get_linkpack( const char *bufch,
                   vector <string> &linkpack, time_t &time ) {
   istringstream bufis(bufch);
   bufis>>time;
   if (!bufis.good()) return;
   string node;

   // to assure no unique, also in case last field read twice
   set<string> unique;
   while (bufis.good()) {
      bufis>>node;
      unique.insert(node);
   }
   linkpack.assign(unique.begin(),unique.end());

   // debugging
   // cout<<"Line: "<<bufch<<endl;
   // if (linkpack.size()<2) {
   //    cout<<"Line: "<<bufch<<endl;
   //    cout<<" has has less than 3 collumns, skipping the line..."<<endl;
   // }
}

void get_weighted_linkpack( const char *bufch,
                   vector <string> &linkpack, double &weight, time_t &time ) {
   istringstream bufis(bufch);
   bufis>>time;
   if (!bufis.good()) return;
   string node;

   // to assure no unique, also in case last field read twice
   set<string> unique;
   bufis>>node;
   while (bufis.good()) {
      unique.insert(node);
      bufis>>node;
   }
   weight = atof(node.c_str());
   linkpack.assign(unique.begin(),unique.end());

}


//=====================================================================
// the main function, reads sequentially lines of the input files
// output differential network files
// either to the output file, or to the gephi server
// if verbose>0 gives extra statics on the network reduction
//=====================================================================

int do_filter( int verbose, string viztype,
               string input, string inputformat,
               string output, string server,
               const unsigned maxstored, const unsigned maxvisualized,
               unsigned forgetevery, double forgetconst,
               double timewindow, double edgemin,
               string label1, string label2, string label3,
               string hidden_node, bool hide_singletons,
               unsigned timecontraction, unsigned fps
               ) {
   // system signals handlers
   signal(SIGINT, handle_kill);

   //=====================================================================
   // load data
   //=====================================================================
   vector <string> linkpack;
   double weight = 1;
   time_t linktime, prev_linktime;
   set <string> all_nodes; // used solely for gathering additional statistics

   ifstream inputnet(input.c_str());
   {
      static char bufch[100000];
      inputnet.getline(bufch,100000);
      if (inputformat=="weighted")
         get_weighted_linkpack(bufch, linkpack, weight, linktime);
      else
         get_linkpack(bufch, linkpack, linktime);
   }

   time_t firstlink_time=linktime;
   long upd_interval=round(1.0*timecontraction/fps);
   if (upd_interval<1) {
      cout<<"The timecontraction set is smaller than fps ("<<fps<<")."
          <<"Please select it higher than fps."<<endl;
      exit(1);
   }

   cout<<"Starting to create differential network files."<<endl;
   cout<<"List of parameters:"<<endl;
   cout<<"  verbose: "<<verbose<<endl;
   cout<<"  viztype: "<<viztype<<endl;
   cout<<"  input: "<<input<<endl;
   cout<<"  inputformat: "<<inputformat<<endl;
   cout<<"  output: "<<output<<endl;
   cout<<"  server: "<<server<<endl;
   cout<<"  maxstored: "<<maxstored<<endl;
   cout<<"  maxvisualized: "<<maxvisualized<<endl;
   cout<<"  forgetevery: "<<forgetevery<<endl;
   cout<<"  forgetconst: "<<forgetconst<<endl;
   cout<<"  timewindow: "<<timewindow<<endl;
   cout<<"  edgemin: "<<edgemin<<endl;
   cout<<"  label1: "<<label1<<endl;
   cout<<"  label2: "<<label2<<endl;
   cout<<"  label3: "<<label3<<endl;
   cout<<"  hidden_node: "<<hidden_node<<endl;
   cout<<"  hide_singletons: "<<hide_singletons<<endl;
   cout<<"  timecontraction: "<<timecontraction<<endl;
   cout<<"  fps: "<<fps<<endl;

   cout<<"Derived:"<<endl;
   cout<<"  interval: "<<upd_interval<<endl;

   //=====================================================================
   // time checkers inits
   //=====================================================================
   time_checker stats_checker(time(0), 10);
   pace_checker pace_check(linktime);
   pt::time_duration real_interval=microseconds(1000000/fps); //in microseconds
   time_checker_intervals_micro videotime_checker(pt::microsec_clock::local_time(), real_interval);

   //=====================================================================
   // cpu clock collectors
   //=====================================================================
   clock_collectors myclockcollector;
   myclockcollector.addnamedcollectors(11, "TTTTdatareading",
      "TTTTadd_linkpack",    "TTTThashtagextractor",
      "TTTTfindinstored", "TTTTaddedtostored",
      "TTTTmtrtend", "TTTTnonmatchingkeywords+forgetting",
      "TTTTselect_nodes", "TTTTadddelete_nodes",
      "TTTTupdate_nodes_edges", "TTTTgcupdate");

   //=====================================================================
   // selectors and visualizers classes
   //=====================================================================
   client_base *myoutput;
   if (server!="") myoutput=new client_gephi(server,output);
   else myoutput=new client_file(output);

   net_collector_base *mynet;
   viz_selector_base *myviz;

   if (viztype=="fastviz")
      mynet=new net_collector( maxstored, myclockcollector );
   else
      mynet=new net_collector_timewindow( maxstored, timewindow,  myclockcollector );

   myviz=new viz_selector( *mynet, *myoutput, myclockcollector, verbose );

   if (server=="")
      myviz->add_labels( pt::to_simple_string(pt::from_time_t(linktime)),
                        label1, label2, label3 );

   //=====================================================================
   // time to start
   //=====================================================================
   long total_read = 0, total_links = 0, total_malformed = 0;
   double total_score;
   int line=1, frame=0;
   long ts;

   for (ts=firstlink_time; keep_going; ts+=upd_interval)
   {
      // printf("%d %d\n",ts,linktime); cout.flush();
      frame++;

      if (!inputnet.good()) {
         keep_going=0;
         cout<<"The file has finished (0), last line number is "<<line<<endl;
      }

      while ( keep_going && linktime>=ts && linktime<ts+upd_interval )
      {
         ++total_read;

         //TODO I'm not sure how much sense this has...
         pace_check.next_tweet(linktime);
         myclockcollector.collect("TTTTdatareading");

         //=====================================================================
         // update information about stored nodes
         //=====================================================================
         {
            myclockcollector.collect("TTTTadd_linkpack");
            total_links+=(linkpack.size()-1)*linkpack.size();
            if ( linkpack.size()>1 ) {
               unsigned nodes = linkpack.size();
               total_score += weight * nodes * (nodes-1);
               mynet->add_linkpack( linkpack, weight, linktime );
            }
         }
         if (verbose>1) {
            all_nodes.insert( linkpack.begin(), linkpack.end() );
         }

         //=====================================================================
         // print stats
         //=====================================================================
         if (stats_checker(time(0))) {
            stats_checker.reset();
            if (verbose>5)
            cout << "########################################\n"
                  << "pace: " << pace_check.stats() << endl
                  << "cur link time: " << pt::to_simple_string(pt::from_time_t(linktime))
                  <<" (timestamp "<<linktime<<")"<< endl
                  << "lines read: " << total_read << endl
                  << "links encountered: " << total_links << "(" << (double(total_links) / total_read)
                  << ")" << endl
                  << "malformed: " << total_malformed << endl;
         }

         //=====================================================================
         // link reading
         //=====================================================================
         {static char bufch[100000];
         line++;
         if (!inputnet.getline(bufch,100000)) {
            keep_going=0;
            cout<<"The file has finished (1), last line number is "<<line<<endl;
            break;
         }
         if (!inputnet.good()) {
            keep_going=0;
            cout<<"The file has finished (2), last line number is "<<line<<endl;
         }
         linkpack.clear();
         prev_linktime=linktime;
         if (inputformat=="weighted")
            get_weighted_linkpack(bufch, linkpack, weight, linktime);
         else
            get_linkpack(bufch, linkpack, linktime);
         if (prev_linktime>linktime) {
            cout<<"Data is not sorted in increasing order of the timestamps, exiting."
                <<endl;
            keep_going=0;
         }}
      }

      //=====================================================================
      // forgetting
      //=====================================================================
      //if (total_links%10==0)
      if (forgetevery>0) if (frame%forgetevery==0) {
         mynet->forget_connections(forgetconst);
         total_score *= forgetconst;
      }
      myclockcollector.collect("TTTTnonmatchingkeywords+forgetting");

      //=====================================================================
      // visualize selected set of nodes (creates data for a frame)
      //=====================================================================
      if (server=="")
         myviz->change_label_datetime(
            pt::to_simple_string(pt::from_time_t(long(ts))));

      // update adjeciency matric if needed and draw
      mynet->update_net_collector_base();
      myviz->draw(maxvisualized, edgemin, hidden_node, hide_singletons);

      // debugging
      if (verbose>2) if (frame%50==0) {
         for (int i=0; i<10; i++) cout<<mynet->names[i]<<" ";
         cout<<endl;
         for (int i=0; i<10; i++) {
            for (int j=0; j<10; j++)
               cout<<mynet->net[i][j]<<" ";
            cout<<endl;
         }
      }

      // output additional statistics
      if (verbose>0) if (frame%50==0) {
         auto nodes_encountered = all_nodes.size();
         auto score_encountered = total_score;
         auto nodes_buffered = mynet->get_nodes_number();
         auto score_buffered = mynet->get_total_score();
         auto nodes_visualized = myviz->get_nodes_visualized();
         auto score_visualized = myviz->get_total_score();
         auto nodes_hidden = myviz->get_nodes_not_visualized();
         printf("Frame stats:"
            "nodes_encountered=%6d, score_encountered=%6.0f, "
            "nodes_buffered=%6d, score_buffered=%6.0f, "
            "nodes_visualized=%6d, score_visualized=%6.0f, "
            "nodes_hidden=%6d, %s.\n",
            nodes_encountered, score_encountered,
            nodes_buffered, score_buffered,
            nodes_visualized, score_visualized,
            nodes_hidden, myviz->get_netsstats().c_str() );
      }

      // sleep if gephi server is specified to in between sent events
      if (server!="") {
         /*
         boost::this_thread::sleep(
            videotime_checker.till_next(pt::microsec_clock::local_time())
            );
         */
         sleep( videotime_checker.till_next(
            pt::microsec_clock::local_time() ).seconds() );
         videotime_checker.reset();
      }

   }
   cout<<"Total lines read: "<<total_read
       <<", links loaded: "<<total_links
       <<", frames generated: "<<frame<<endl;
   if (verbose>0)
      cout<<"Total nodes encountered: "<<all_nodes.size()
          <<", total nodes drawn: "<<myviz->get_how_many_drawn()<<endl;

   if (verbose>3) {
      myclockcollector.printall();
      myclockcollector.resetall();
   }

   return total_links;
}

//=====================================================================
// main with program options
//=====================================================================
int main(int argc, char** argv) {
   namespace po = boost::program_options;
   po::options_description desc("Allowed options");

   string home_dir = getenv("HOME");

   desc.add_options()
      ("help", "show options")
      ("verbose", po::value<int>()->default_value(1), "")
      ("viztype", po::value<string>()->default_value("fastviz"),
         "Possible visualization types: fastviz (default), timewindow")
      ("input", po::value<string>()->default_value(""),"")
      ("inputformat", po::value<string>()->default_value(""),"")
      ("output", po::value<string>()->default_value(""), "")
      ("server", po::value<string>()->default_value(""),
         "Address to the updateGraph command of Gephi Streaming API server."
         "If not provided then output is printed to file pointed as argument"
         "of --output option.")
      ("maxstored", po::value<unsigned>()->default_value(2000), "")
      ("maxvisualized", po::value<unsigned>()->default_value(50), "")
      ("forgetevery", po::value<unsigned>()->default_value(10),
         "Influences only the fastviz algorithm.")
      ("forgetconst", po::value<double>()->default_value(0.75),
         "Influences only the fastviz algorithm.")
      ("timewindow", po::value<double>()->default_value(2000),
         "Influences only the timewindow algorithm.")
      ("edgemin", po::value<double>()->default_value(0.95), "")
      ("label1", po::value<string>()->default_value(""),"")
      ("label2", po::value<string>()->default_value(""),"")
      ("label3", po::value<string>()->default_value(""),"")
      ("hide_node", po::value<string>()->default_value(""),
         "Hide given node in the visualizations")
      ("hide_singletons", po::value<bool>()->default_value("true"),
         "Hide nodes without edges in the visualization")
      ("timecontraction", po::value<unsigned>()->default_value(3600), "")
      ("fps", po::value<unsigned>()->default_value(30), "")
      ;

   po::variables_map vm;
   po::store(po::parse_command_line(argc, argv, desc), vm);
   po::notify(vm);

   if (vm.count("help")) {
      cerr << desc << "\n";
      exit(1);
   }
   unsigned verbose = vm["verbose"].as<int>();

   string viztype = vm["viztype"].as<string>();

   string input = vm["input"].as<string>();
   string inputformat = vm["inputformat"].as<string>();
   string output = vm["output"].as<string>();
   string server = vm["server"].as<string>();
   if ( input=="" || (output=="" && server=="") ) {
      cout<<"Required arguments are: input and either output or server."<<endl<<endl;
      cerr << desc << "\n";
      exit(1);
   }
   if (server!="") cout<<"Data will be sent to: "<<server<<endl;
   else cout<<"Data will be saved to file: "<<output<<".json"<<endl;

   unsigned maxstored = vm["maxstored"].as<unsigned>();
   unsigned maxvisualized = vm["maxvisualized"].as<unsigned>();
   unsigned forgetevery = vm["forgetevery"].as<unsigned>();
   if (viztype=="timewindow") forgetevery=0;
   double forgetconst = vm["forgetconst"].as<double>();
   double timewindow = vm["timewindow"].as<double>();
   double edgemin = vm["edgemin"].as<double>();
   string label1 = vm["label1"].as<string>();
   string label2 = vm["label2"].as<string>();
   string label3 = vm["label3"].as<string>();
   string hidden_node = vm["hide_node"].as<string>();
   bool hide_singletons = vm["hide_singletons"].as<bool>();
   unsigned timecontraction = vm["timecontraction"].as<unsigned>();
   unsigned fps = vm["fps"].as<unsigned>();

   do_filter( verbose, viztype, input, inputformat, output, server,
              maxstored, maxvisualized,
              forgetevery, forgetconst, timewindow, edgemin,
              label1, label2, label3,
              hidden_node, hide_singletons,
              timecontraction, fps
              );
   return 0;
}
