#ifndef VIZ_CLIENT_GEPHI_HPP
#define VIZ_CLIENT_GEPHI_HPP

#include <fstream>
#include <iostream>
#include <string>

#include <boost/filesystem/operations.hpp>
#include <boost/network/protocol/http/client.hpp>
#include <json/json.h>

#include <viz/client.cpp>

using namespace std;

class client_gephi : public client_base {
public:
	client_gephi()
	:request("http://localhost:8080/workspace0?operation=updateGraph") {
		output.open(((string)"defaultout"+".json").c_str());
	}
	client_gephi(string host):request(host) {
		output.open(((string)"defaultout"+".json").c_str());
	}
	client_gephi(string host, string name):request(host) {
		output.open((name+".json").c_str());
	}
	
	void update(){
		output<<task<<endl;
		response = client.post(request, task);
		task="";
		//cout << body(response) << endl;
	}
	
private:
	boost::network::http::client client;
	boost::network::http::client::request request;
	boost::network::http::client::response response;
	ofstream output;
};

#endif
