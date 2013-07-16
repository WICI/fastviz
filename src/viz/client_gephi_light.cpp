#ifndef VIZ_CLIENT_GEPHI_HPP
#define VIZ_CLIENT_GEPHI_HPP

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <viz/client.cpp>

using namespace std;

class client_gephi_light : public client_base {
public:
   
	client_gephi_light(string host, string name) {
      hostname=host;
// 		output.open((name+".json").c_str());
	}
	
	void update(){
// 		output<<task<<endl;
      string command="curl 'http://"+hostname+
                     ":8080/workspace0?operation=updateGraph' -d "+task;
      int result = system(command.c_str());
		task="";
	}
	
private:
   string hostname;
// 	ofstream output;
};

#endif
