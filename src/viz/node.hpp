/*
 * Various node structures used in the main code
 */

#ifndef VIZ_NODE_HPP
#define VIZ_NODE_HPP

#include <string>
using namespace std;

struct node_base {
  string nm; //name
  unsigned pos; //position
  
  node_base() {nm=""; pos=-1; }
  node_base(string name, unsigned position) {nm=name; pos=position;}
  
  bool operator<(const node_base &other) const {return nm<other.nm;}
  bool operator==(const node_base &other) const {return nm==other.nm;}
  bool operator<(const string &other) const {return nm<other;}
  bool operator==(const string &other) const {return nm==other;}
};

struct node_the:node_base {
  double str; //strength
  unsigned long time;
  
  node_the():node_base() {str=0; time=0; }
  node_the(string name, unsigned position, double strength, unsigned timestamp)
		:node_base(name, position){	  
	  str=strength; time=timestamp;
  }
  
  bool operator<(const node_the &other) const {return nm<other.nm;}
  bool operator==(const node_the &other) const {return nm==other.nm;}
  bool operator<(const string &other) const {return nm<other;}
  bool operator==(const string &other) const {return nm==other;}
};


template <class T0>
bool compare_node_strength ( T0 i, T0 j) { return (i.str<j.str); }


#endif