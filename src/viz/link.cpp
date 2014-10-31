/*
 * Various link structures used in the main code
 */

#ifndef VIZ_LINK_HPP
#define VIZ_LINK_HPP

#include <string>
using namespace std;

struct link_base {
	unsigned eid; //id of the link
  string name1, name2; //ids of the nodes
  double weight;
  double orgweigth;

  link_base(string name1="", string name2="", double weight=-1) {
  	this->name1=name1;
  	this->name2=name2;
  	this->weight=weight;
    this->orgweigth=weight;
  }

  // bool operator<(const link_base &other) const {return nm<other.nm;}
  // bool operator==(const link_base &other) const {return nm==other.nm;}
  // bool operator<(const string &other) const {return nm<other;}
  // bool operator==(const string &other) const {return nm==other;}
};

struct link_timed:link_base {
  long ts;

  link_timed(string name1="", string name2="", double weight=-1, long ts=-1)
  	:link_base(name1, name2, weight) {
  		this->ts=ts;
  	}
};


template <class T0>
bool compare_link_strength ( T0 i, T0 j) { return (i.str<j.str); }


#endif