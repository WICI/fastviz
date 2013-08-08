#ifndef VIZ_NODE_HPP
#define VIZ_NODE_HPP

#include <string>
using namespace std;

/*
 * Various node structures used in the main code
 */

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

struct node_viz {
	string label; //name
	double size; //strength
	double r, g, b; //color
	double counter;
	
	node_viz() {label=""; size=5; g=r=b=0.5;}
	bool operator<(const node_viz &other) const {return label<other.label;}
	bool operator==(const node_viz &other) const {return label==other.label;}
};


struct node {
  string nm; //name
  double str; //strength
  unsigned pos; //position
  
  node() {nm=""; str=0; pos=-1; }
  node(string name, unsigned position, double strength) {
	  nm=name; pos=position; str=strength;
  }
  
  bool operator<(const node &other) const {return nm<other.nm;}
  bool operator==(const node &other) const {return nm==other.nm;}
  bool operator<(const string &other) const {return nm<other;}
  bool operator==(const string &other) const {return nm==other;}
//   void operator=(string &name) const {nm=name;}
};

struct node_with_counter:node {
	double counter;
	
	node_with_counter():node() {counter=0;}
	node_with_counter(string name, unsigned position, double strength, 
						 double countervalue):node(name, position,strength)
	{ counter=countervalue; }
	node_with_counter(node_the &other) {
		nm=other.nm; pos=other.pos; str=other.str; counter=0;
	}
	
	void operator=(node_the &other) {
		nm=other.nm; pos=other.pos; str=other.str; counter=0;
	}
};


template <class T0>
bool compareNodeStrength ( T0 i, T0 j) { return (i.str<j.str); }


#endif