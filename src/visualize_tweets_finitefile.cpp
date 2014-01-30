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

//=====================================================================
// the main function, reads sequentially lines of the input files
// output differential network files
// either to the output file, or to the gephi server
// if verbose>0 gives extra statics on the network reduction
//=====================================================================
int do_filter( int verbose, string viztype, string input, string output, 
               string server,
               const unsigned maxstored, const unsigned maxvisualized,
               unsigned forgetevery, double forgetconst, 
               unsigned long timewindow,
               double edgemin, string label1, string label2,
               unsigned timecontraction, unsigned fps, int weighttype
               ) {
   // system signals handlers
   signal(SIGINT, handle_kill);
   
   //=====================================================================
   // load data
   //=====================================================================
   time_t linktime, prev_linktime;
   vector <string> linkpack;
   set <string> all_nodes; // used solely for gathering additional statistics
   
   ifstream inputnet(input.c_str());
   {static char bufch[100000];
   inputnet.getline(bufch,100000);
   get_linkpack(bufch, linkpack, linktime);}
   
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
   cout<<"  output: "<<output<<endl;
   cout<<"  server: "<<server<<endl;
   cout<<"  maxstored: "<<maxstored<<endl;
   cout<<"  maxvisualized: "<<maxvisualized<<endl;
   cout<<"  forgetevery: "<<forgetevery<<endl;
   cout<<"  forgetconst: "<<forgetconst<<endl;
   cout<<"  edgemin: "<<edgemin<<endl;
   cout<<"  label1: "<<label1<<endl;
   cout<<"  label2: "<<label2<<endl;
   cout<<"  timecontraction: "<<timecontraction<<endl;
   cout<<"  fps: "<<fps<<endl;
   cout<<"  weighttype: "<<weighttype<<endl;
   
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
   
   // debugging
   if (viztype=="fastviz") 
      mynet=new net_collector( maxstored, myclockcollector );
   else 
      mynet=new net_collector_timewindow( maxstored, timewindow,  myclockcollector );
   
   myviz=new viz_selector( *mynet, *myoutput, myclockcollector, verbose );
   
   if (server=="") 
      myviz->add_labels( pt::to_simple_string(pt::from_time_t(linktime)), 
                        label1, label2, "WICI data challenge" );
   
   //=====================================================================
   // time to start
   //=====================================================================
   long total_read = 0, total_links = 0, total_malformed = 0;
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
            if (verbose==9) cout<<total_read-1;
            if ( linkpack.size()>1 ) 
               mynet->add_linkpack( linkpack, weighttype, linktime );
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
         get_linkpack(bufch, linkpack, linktime);}
         if (prev_linktime>linktime) {
            cout<<"Data is not sorted in increasing order of the timestamps, exiting."
                <<endl;
            keep_going=0;
         }
      }
      
      //=====================================================================
      // forgetting
      //=====================================================================
      //if (total_links%10==0)
      if (frame%forgetevery==0) mynet->forget_connections(forgetconst);
      myclockcollector.collect("TTTTnonmatchingkeywords+forgetting");
      
      //=====================================================================
      // visualize selected set of nodes (creates data for a frame)
      //=====================================================================
      if (server=="") 
         myviz->change_label_datetime(
            pt::to_simple_string(pt::from_time_t(long(ts))));
      
      // update adjeciency matric if needed and draw
      mynet->update_net_collector_base();
      myviz->draw(maxvisualized, edgemin, label2);
      
      // output additional statistics
      unsigned nodes_number = mynet->get_nodes_number();
      double total_score = mynet->get_total_score();
      unsigned nodes_visualized = myviz->get_nodes_visualized();
      unsigned nodes_not_visualized = myviz->get_nodes_not_visualized();
      unsigned total_score_viz = myviz->get_total_score();

      // debugging
      if (frame%50==0) {
         for (int i=0; i<10; i++) cout<<mynet->names[i]<<" ";
         cout<<endl;
         for (int i=0; i<10; i++) {
            for (int j=0; j<10; j++)
               cout<<mynet->net[i][j]<<" ";
            cout<<endl;
         } 

         printf("Frame stats: nodes buffered=%5d, total score=%5.0f, "
            "nodes visualized=%4d, not=%4d, total score=%4.0f.\n", 
            nodes_number, total_score, 
            nodes_visualized, nodes_not_visualized, total_score_viz);
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

   // debugging
   // if (verbose>5) {
   //    for (int i=0; i<10; i++)
   //       cout<<mynet->names[i]<<" "<<mynet->net[i][i]<<" ";
   // }
   
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
         "Possible visualization types: fastviz (default), time-window")
      ("input", po::value<string>()->default_value(""),"")
      ("output", po::value<string>()->default_value(""), "")
      ("server", po::value<string>()->default_value(""), 
         "Address to the updateGraph command of Gephi Streaming API server."
         "If not provided then output is printed to file pointed as argument" 
         "of --output option.")
      ("maxstored", po::value<unsigned long>()->default_value(2000), "")
      ("maxvisualized", po::value<unsigned long>()->default_value(50), "")
      ("forgetevery", po::value<unsigned>()->default_value(10), 
         "Influences only the fastviz algorithm.")
      ("forgetconst", po::value<double>()->default_value(0.75), 
         "Influences only the fastviz algorithm.")
      ("timewindow", po::value<unsigned long>()->default_value(2000), 
         "Influences only the timewindow algorithm.")
      ("edgemin", po::value<double>()->default_value(0.95), "")
      ("label1", po::value<string>()->default_value(""),"")
      ("label2", po::value<string>()->default_value(""),"")
      ("timecontraction", po::value<unsigned>()->default_value(3600), "")
      ("fps", po::value<unsigned>()->default_value(30), "")
      ("weighttype", po::value<int>()->default_value(1), "")
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
   string output = vm["output"].as<string>();
   string server = vm["server"].as<string>();
   if ( input=="" || (output=="" && server=="") ) {
      cout<<"Required arguments are: input and either output or server."<<endl<<endl;
      cerr << desc << "\n";
      exit(1);
   }
   if (server!="") cout<<"Data will be sent to: "<<server<<endl;
   else cout<<"Data will be saved to file: "<<output<<".json"<<endl;
   
   unsigned maxstored = vm["maxstored"].as<unsigned long>();
   unsigned maxvisualized = vm["maxvisualized"].as<unsigned long>();
   unsigned forgetevery = vm["forgetevery"].as<unsigned>();
   double forgetconst = vm["forgetconst"].as<double>();
   unsigned timewindow = vm["timewindow"].as<unsigned long>();
   double edgemin = vm["edgemin"].as<double>();
   string label1 = vm["label1"].as<string>();
   string label2 = vm["label2"].as<string>();
   unsigned timecontraction = vm["timecontraction"].as<unsigned>();
   unsigned fps = vm["fps"].as<unsigned>();
   int weighttype = vm["weighttype"].as<int>();
  
   do_filter( verbose, viztype, input, output, server, 
              maxstored, maxvisualized, 
              forgetevery, forgetconst, timewindow, edgemin, label1, label2, 
              timecontraction, fps, weighttype
              );
   return 0;
}
