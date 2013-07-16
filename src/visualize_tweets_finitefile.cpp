/*
 * Filters desirable tweets, builds a network
 * out of chosen information and visualizes it.
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

#include <pms/clock_collector.cpp>
#include <pms/time_checker_intervals.hpp>

#include <viz/client.cpp>
// #include <viz/client_gephi.cpp>
#include <viz/client_gephi_light.cpp>
#include <viz/net_collector.cpp>
#include <viz/viz_selector.cpp>

using namespace std;
namespace pt = boost::posix_time;

// global variable used by signal handler
bool keep_going = true;

void handle_kill(int sig) {
   keep_going = false;
   // unregister myself
   signal(SIGINT, SIG_DFL);
}

// structure to store pack of links
typedef vector <string> linkpack_type;
typedef set <string> nodes_set;

// loads a pack of links from a line of file
void get_linkpack( const char *bufch, 
                   linkpack_type &linkpack, time_t &time ) {
   istringstream bufis(bufch);
   bufis>>time;
   string node;
   while (bufis.good()) {
      bufis>>node;
      linkpack.push_back(node);
   }
   if (linkpack.size()<2) {
      cout<<"Datafile has less than 3 collumns. Exiting..."<<endl;
      cout<<"Last buffered line: "<<bufch<<endl;
      exit(1);
   }
}

// main, reads sequentially lines of the input files
// output differential network files
// either to the output file, or to the gephi server
// if verbose>0 gives extra statics on the network reduction
int do_filter( string input, string server, string output, 
               int verbose,
               const unsigned maxstored, const unsigned maxvisualized,
               unsigned forgetevery, double forgetconst, 
               double edgemin, string keyword,
               unsigned timecontraction, unsigned fps, int scoretype
               ) {
   //++++++++++++++++++++ SYSTEM SIGNALS HANDLERS
   signal(SIGINT, handle_kill);
   
   //++++++++++++++++++++ LOAD DATA
   time_t linktime, prev_linktime;
   linkpack_type linkpack;
   nodes_set all_nodes; // this is solely for outputting statistics
   
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
   cout<<"  input: "<<input<<endl;
   cout<<"  output: "<<output<<endl;
   cout<<"  server: "<<server<<endl;
   cout<<"  maxstored: "<<maxstored<<endl;
   cout<<"  maxvisualized: "<<maxvisualized<<endl;
   cout<<"  forgetevery: "<<forgetevery<<endl;
   cout<<"  forgetconst: "<<forgetconst<<endl;
   cout<<"  edgemin: "<<edgemin<<endl;
   cout<<"  keyword: "<<keyword<<endl;
   cout<<"  timecontraction: "<<timecontraction<<endl;
   cout<<"  fps: "<<fps<<endl;
   
   cout<<"Derived:"<<endl;
   cout<<"  interval: "<<upd_interval<<endl;
   
   
   //++++++++++++++++++++ TIME CHECKERS INITS
   time_checker stats_checker(time(0), 10);
   pace_checker pace_check(linktime);
   pt::time_duration real_interval=microseconds(1000000/fps); //in microseconds
   time_checker_intervals_micro videotime_checker(pt::microsec_clock::local_time(), real_interval);
   
   //++++++++++++++++++++ CPU CLOCK COLLECTORS
   clock_collectors myclockcollector;
   myclockcollector.addnamedcollectors(11, "TTTTdatareading",
      "TTTTadd_linkpack",    "TTTThashtagextractor",
      "TTTTfindinstored", "TTTTaddedtostored",
      "TTTTmtrtend", "TTTTnonmatchingkeywords+forgetting",
      "TTTTselect_nodes", "TTTTadddelete_nodes",
      "TTTTupdate_nodes_edges", "TTTTgcupdate");
   
   //++++++++++++++++++++ SELECTORS AND VISUALIZERS CLASSES
   client_base *gc;
   if (server!="") 
   // gc= new client_gephi(server,output);
      gc= new client_gephi_light(server, output);
   else 
      gc= new client_file(output);
   
   vector <net_collector*> nets; // could be more than one networks
   viz_selector_base *viz1;
   
   nets.push_back( new net_collector( maxstored, myclockcollector ) );
   viz1=new viz_selector( *nets[0], *gc, myclockcollector );
   viz1->add_labels( pt::to_simple_string(pt::from_time_t(linktime)), 
                     keyword, "WICI data challenge" );
   
   //++++++++++++++++++++ TIME TO START
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
            nets[0]->add_linkpack( linkpack, scoretype );
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
      if (frame%forgetevery==0) nets[0]->forget_connections(forgetconst);
      myclockcollector.collect("TTTTnonmatchingkeywords+forgetting");
      
      //=====================================================================
      // visualize selected set of nodes (creates data for a frame)
      //=====================================================================
      viz1->change_label_datetime(
         pt::to_simple_string(pt::from_time_t(long(ts))));
      viz1->draw(maxvisualized, edgemin, verbose, keyword);
      
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
          <<", total nodes drawn: "<<viz1->get_how_many_drawn()<<endl;
   
   if (verbose>3) {
      myclockcollector.printall();
      myclockcollector.resetall();
   }
   
   return total_links;
}

int main(int argc, char** argv) {
   namespace po = boost::program_options;
   po::options_description desc("Allowed options");
   
   string home_dir = getenv("HOME");
   
   desc.add_options()
      ("help", "show options")
      ("verbose", po::value<int>()->default_value(1), "")
      ("input", po::value<string>()->default_value(""),"")
      ("output", po::value<string>()->default_value(""), "")
      ("server", po::value<string>()->default_value(""), 
       "address to the updateGraph command of Gephi Streaming API server. If not provided then output is printed to file pointed as argument of --output option.")
      ("maxstored", po::value<unsigned long>()->default_value(2000), "")
      ("maxvisualized", po::value<unsigned long>()->default_value(100), "")
      ("forgetevery", po::value<unsigned>()->default_value(10), "")
      ("forgetconst", po::value<double>()->default_value(0.99), "")
      ("edgemin", po::value<double>()->default_value(0.5), "")
      ("keyword", po::value<string>()->default_value(""),"")
      ("timecontraction", po::value<unsigned>()->default_value(100), "")
      ("fps", po::value<unsigned>()->default_value(30), "")
      ("scoretype", po::value<int>()->default_value(1), "")
      ; 
   
   po::variables_map vm;
   po::store(po::parse_command_line(argc, argv, desc), vm);
   po::notify(vm);       

   if (vm.count("help")) {
      cerr << desc << "\n";
      exit(1);
   }
   unsigned verbose = vm["verbose"].as<int>();
   
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
   double edgemin = vm["edgemin"].as<double>();
   string keyword = vm["keyword"].as<string>();
   unsigned timecontraction = vm["timecontraction"].as<unsigned>();
   unsigned fps = vm["fps"].as<unsigned>();
   int scoretype = vm["scoretype"].as<int>();
  
   do_filter( input, server, output, verbose,
              maxstored, maxvisualized, 
              forgetevery, forgetconst, edgemin, keyword, 
              timecontraction, fps, scoretype
              );
   return 0;
}
