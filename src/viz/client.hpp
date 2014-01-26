/*
 * Save differential changes between consequtive states of a network
 * in JSON format corresponding to the Gephi Streaming API format
 */

#ifndef VIZ_CLIENT_HPP
#define VIZ_CLIENT_HPP

#include <fstream>
#include <iostream>
#include <string>

#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>
#include <json/json.h>

using namespace std;
using boost::lexical_cast;


class client_base {
public:	
	
	virtual void update()=0 ;
	
	template <class TT0> void add_node(TT0 id) {produce_event("an", id);}
	template <class TT0> void change_node(TT0 id) {produce_event("cn", id);}
	template <class TT0> void delete_node(TT0 id) {produce_event_no_attributes("dn", id);}
	
	template <class TT0> void add_edge(TT0 id) {produce_event("ae", id);}
	template <class TT0> void change_edge(TT0 id) {produce_event("ce", id);}
	template <class TT0> void delete_edge(TT0 id) {produce_event_no_attributes("de", id);}
	
	template <class TT0> void add_label(TT0 id) {produce_event("al", id);}
	template <class TT0> void change_label(TT0 id) {produce_event("cl", id);}
	template <class TT0> void delete_label(TT0 id) {produce_event_no_attributes("dl", id);}
	
	template <class TT0, class T2> void add_node(TT0 id, T2 &extattr) 
   {produce_event("an", id, extattr);}
	template <class TT0, class T2> void change_node(TT0 id, T2 &extattr) 
   {produce_event("cn", id, extattr);}
	template <class TT0, class T2> void delete_node(TT0 id, T2 &extattr) 
   {produce_event("dn", id, extattr);}
	
	template <class TT0, class T2> void add_edge(TT0 id, T2 &extattr) 
   {produce_event("ae", id, extattr);}
	template <class TT0, class T2> void change_edge(TT0 id, T2 &extattr) 
   {produce_event("ce", id, extattr);}
	template <class TT0, class T2> void delete_edge(TT0 id, T2 &extattr) 
   {produce_event("de", id, extattr);}
	
	template <class TT0, class T2> void add_label(TT0 id, T2 &extattr) 
   {produce_event("al", id, extattr);}
	template <class TT0, class T2> void change_label(TT0 id, T2 &extattr) 
   {produce_event("cl", id, extattr);}
	template <class TT0, class T2> void delete_label(TT0 id, T2 &extattr) 
   {produce_event("dl", id, extattr);}
	
	/* attributes implemented with std::map (it's slower)
	template <class TT1>
	void set_attributes(string a1, TT1 v1) {
		attributes[a1]=lexical_cast<string>(v1);
	}
	template <class TT1, class TT2>
	void set_attributes(string a1, TT1 v1, string a2, TT2 v2) {
		attributes[a1]=lexical_cast<string>(v1);
		attributes[a2]=lexical_cast<string>(v2);
	}
	template <class TT1, class TT2, class TT3>
	void set_attributes(string a1, TT1 v1, string a2, TT2 v2, string a3, TT3 v3) {
		attributes[a1]=lexical_cast<string>(v1);
		attributes[a2]=lexical_cast<string>(v2);
		attributes[a3]=lexical_cast<string>(v3);
	}
	template <class TT1, class TT2, class TT3, class TT4>
	void set_attributes(string a1, TT1 v1, string a2, TT2 v2, string a3, TT3 v3,
							  string a4, TT4 v4) {
		attributes[a1]=lexical_cast<string>(v1);
		attributes[a2]=lexical_cast<string>(v2);
		attributes[a3]=lexical_cast<string>(v3);
		attributes[a4]=lexical_cast<string>(v4);
	}
	template <class TT1, class TT2, class TT3, class TT4, class TT5>
	void set_attributes(string a1, TT1 v1, string a2, TT2 v2, string a3, TT3 v3,
							  string a4, TT4 v4, string a5, TT5 v5) {
		attributes[a1]=lexical_cast<string>(v1);
		attributes[a2]=lexical_cast<string>(v2);
		attributes[a3]=lexical_cast<string>(v3);
		attributes[a4]=lexical_cast<string>(v4);
		attributes[a5]=lexical_cast<string>(v5);
	}
	template <class TT1, class TT2, class TT3, class TT4, class TT5, class TT6>
	void set_attributes(string a1, TT1 v1, string a2, TT2 v2, string a3, TT3 v3,
							  string a4, TT4 v4, string a5, TT5 v5, string a6, TT6 v6) {
		attributes[a1]=lexical_cast<string>(v1);
		attributes[a2]=lexical_cast<string>(v2);
		attributes[a3]=lexical_cast<string>(v3);
		attributes[a4]=lexical_cast<string>(v4);
		attributes[a5]=lexical_cast<string>(v5);
		attributes[a6]=lexical_cast<string>(v6);
	}
	template <class TT1, class TT2, class TT3, class TT4, class TT5, class TT6, 
				 class TT7>
	void set_attributes(string a1, TT1 v1, string a2, TT2 v2, string a3, TT3 v3,
							  string a4, TT4 v4, string a5, TT5 v5, string a6, TT6 v6,
							  string a7, TT7 v7) {
		attributes[a1]=lexical_cast<string>(v1);
		attributes[a2]=lexical_cast<string>(v2);
		attributes[a3]=lexical_cast<string>(v3);
		attributes[a4]=lexical_cast<string>(v4);
		attributes[a5]=lexical_cast<string>(v5);
		attributes[a6]=lexical_cast<string>(v6);
		attributes[a7]=lexical_cast<string>(v7);
	}
	template <class TT1, class TT2, class TT3, class TT4, class TT5, class TT6, 
				 class TT7, class TT8>
	void set_attributes(string a1, TT1 v1, string a2, TT2 v2, string a3, TT3 v3,
								string a4, TT4 v4, string a5, TT5 v5, string a6, TT6 v6,
								string a7, TT7 v7, string a8, TT8 v8) {
		attributes[a1]=lexical_cast<string>(v1);
		attributes[a2]=lexical_cast<string>(v2);
		attributes[a3]=lexical_cast<string>(v3);
		attributes[a4]=lexical_cast<string>(v4);
		attributes[a5]=lexical_cast<string>(v5);
		attributes[a6]=lexical_cast<string>(v6);
		attributes[a7]=lexical_cast<string>(v7);
		attributes[a8]=lexical_cast<string>(v8);
	}*/
	
	template <class TT1>
	void set_attributes(string a1, TT1 v1) {
		attributes.clear();
		attributes.push_back(pair<string, string>(a1,lexical_cast<string>(v1)));
	}
	template <class TT1, class TT2>
	void set_attributes(string a1, TT1 v1, string a2, TT2 v2) {
		attributes.clear();
		attributes.push_back(pair<string, string>(a1,lexical_cast<string>(v1)));
		attributes.push_back(pair<string, string>(a2,lexical_cast<string>(v2)));
	}
	template <class TT1, class TT2, class TT3>
	void set_attributes(string a1, TT1 v1, string a2, TT2 v2, string a3, TT3 v3) {
		attributes.clear();
		attributes.push_back(pair<string, string>(a1,lexical_cast<string>(v1)));
		attributes.push_back(pair<string, string>(a2,lexical_cast<string>(v2)));
		attributes.push_back(pair<string, string>(a3,lexical_cast<string>(v3)));
	}
	template <class TT1, class TT2, class TT3, class TT4>
	void set_attributes(string a1, TT1 v1, string a2, TT2 v2, string a3, TT3 v3,
								string a4, TT4 v4) {
		attributes.clear();
		attributes.push_back(pair<string, string>(a1,lexical_cast<string>(v1)));
		attributes.push_back(pair<string, string>(a2,lexical_cast<string>(v2)));
		attributes.push_back(pair<string, string>(a3,lexical_cast<string>(v3)));
		attributes.push_back(pair<string, string>(a4,lexical_cast<string>(v4)));
	}
	template <class TT1, class TT2, class TT3, class TT4, class TT5>
	void set_attributes(string a1, TT1 v1, string a2, TT2 v2, string a3, TT3 v3,
								string a4, TT4 v4, string a5, TT5 v5) {
		attributes.clear();
		attributes.push_back(pair<string, string>(a1,lexical_cast<string>(v1)));
		attributes.push_back(pair<string, string>(a2,lexical_cast<string>(v2)));
		attributes.push_back(pair<string, string>(a3,lexical_cast<string>(v3)));
		attributes.push_back(pair<string, string>(a4,lexical_cast<string>(v4)));
		attributes.push_back(pair<string, string>(a5,lexical_cast<string>(v5)));
	}
	template <class TT1, class TT2, class TT3, class TT4, class TT5, class TT6>
	void set_attributes(string a1, TT1 v1, string a2, TT2 v2, string a3, TT3 v3,
								string a4, TT4 v4, string a5, TT5 v5, string a6, TT6 v6) {
		attributes.clear();
		attributes.push_back(pair<string, string>(a1,lexical_cast<string>(v1)));
		attributes.push_back(pair<string, string>(a2,lexical_cast<string>(v2)));
		attributes.push_back(pair<string, string>(a3,lexical_cast<string>(v3)));
		attributes.push_back(pair<string, string>(a4,lexical_cast<string>(v4)));
		attributes.push_back(pair<string, string>(a5,lexical_cast<string>(v5)));
		attributes.push_back(pair<string, string>(a6,lexical_cast<string>(v6)));
	}
	template <class TT1, class TT2, class TT3, class TT4, class TT5, class TT6, 
	class TT7>
	void set_attributes(string a1, TT1 v1, string a2, TT2 v2, string a3, TT3 v3,
								string a4, TT4 v4, string a5, TT5 v5, string a6, TT6 v6,
								string a7, TT7 v7) {
		attributes.clear();
		attributes.push_back(pair<string, string>(a1,lexical_cast<string>(v1)));
		attributes.push_back(pair<string, string>(a2,lexical_cast<string>(v2)));
		attributes.push_back(pair<string, string>(a3,lexical_cast<string>(v3)));
		attributes.push_back(pair<string, string>(a4,lexical_cast<string>(v4)));
		attributes.push_back(pair<string, string>(a5,lexical_cast<string>(v5)));
		attributes.push_back(pair<string, string>(a6,lexical_cast<string>(v6)));
		attributes.push_back(pair<string, string>(a7,lexical_cast<string>(v7)));
	}
	template <class TT1, class TT2, class TT3, class TT4, class TT5, class TT6, 
	class TT7, class TT8>
	void set_attributes(string a1, TT1 v1, string a2, TT2 v2, string a3, TT3 v3,
								string a4, TT4 v4, string a5, TT5 v5, string a6, TT6 v6,
								string a7, TT7 v7, string a8, TT8 v8) {
		attributes.clear();
		attributes.push_back(pair<string, string>(a1,lexical_cast<string>(v1)));
		attributes.push_back(pair<string, string>(a2,lexical_cast<string>(v2)));
		attributes.push_back(pair<string, string>(a3,lexical_cast<string>(v3)));
		attributes.push_back(pair<string, string>(a4,lexical_cast<string>(v4)));
		attributes.push_back(pair<string, string>(a5,lexical_cast<string>(v5)));
		attributes.push_back(pair<string, string>(a6,lexical_cast<string>(v6)));
		attributes.push_back(pair<string, string>(a7,lexical_cast<string>(v7)));
		attributes.push_back(pair<string, string>(a8,lexical_cast<string>(v8)));
	}
	
protected:
	string task;
private:
	//map <string, string> attributes;
	vector <pair <string, string> > attributes;
	Json::FastWriter jsonwriter;
	
	// note that T2 has to be a container of pairs
	//TODO is this as optimal as passing pointers as arguments?
	template <class TT0, class T2>
	void produce_event(string type, TT0 id, T2 &extattr) {
		Json::Value jsonroot;
		jsonroot[type][lexical_cast<string>(id)]=Json::Value(Json::objectValue);
		typedef typename T2::iterator ittype;
		for (ittype it=extattr.begin(); it!=extattr.end(); it++)
			jsonroot[type][lexical_cast<string>(id)][lexical_cast<string>((*it).first)]=(*it).second;
		task+=jsonwriter.write(jsonroot);
		*task.rbegin()='\r';
		
	}
	
	template <class TT0> 
	void produce_event(string type, TT0 id) {
		Json::Value jsonroot;		
		//decltype(attributes)::iterator it; //
		jsonroot[type][lexical_cast<string>(id)]=Json::Value(Json::objectValue);
		for (vector <pair <string, string> >:: iterator it=attributes.begin();
			it!=attributes.end(); it++)
			jsonroot[type][lexical_cast<string>(id)][(*it).first]=(*it).second;
		task+=jsonwriter.write(jsonroot);
		*task.rbegin()='\r';
	}
	
	template <class TT0> 
	void produce_event_no_attributes(string type, TT0 id) {
		Json::Value jsonroot;
		jsonroot[type][id]=Json::Value(Json::objectValue);
		task+=jsonwriter.write(jsonroot);
		*task.rbegin()='\r';
	}
};

class client_file : public client_base {
public:
	client_file() {
		output.open(((string)"defaultout"+".json").c_str());
	}
	client_file(string name) {
		output.open((name+".json").c_str());
      cout<<"Opening file "<<(name+".json").c_str()<<endl;
      if (output.fail()) {
         cout<<"Uuuups, could not open the file! Terminated."<<endl;
         exit(1);
      }
	}
	
	void update(){
		output<<task;
		if (task.size()==0) output<<"{}";
		output<<"\n";
		task="";
	}
	
private:
	ofstream output;
};

#endif