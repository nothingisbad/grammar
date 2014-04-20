#ifndef XMLSTATS_HH
#define XMLSTATS_HH
/**
 * @file XmlStats.hpp
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Oct 14, 2012
 */
#include "XmlElement.hpp"

/**
 * object with some members for collecting stats on XmlElements
 * passed through the parser. 
 */
class XmlStats {
  std::map<int,int> attribute_count_distribution_ /**< how many elements have 'n' attributes */
    , child_count_distribution_;		  /**< how many elements have 'n' children */
public:
    /**
     * constructs a XmlStats object which is ready to be passed elements for
     * analysis
     */
  XmlStats() {}
  
  void analyze(XmlElement *elem) {
    ++(attribute_count_distribution_[elem->count_attributes()]);
    ++(child_count_distribution_[elem->count_children()]);
  }
  
  void print_report(std::ostream& out) {
    using namespace std;
    map<int,int>::iterator itr;
  
    out << "There are " << endl;
    for(itr = attribute_count_distribution_.begin(); itr != attribute_count_distribution_.end(); ++itr) {
      out << "  " << itr->second << " elements with " << itr->first << " attributes " << endl;
    }
    out << "and" << endl;
    for(itr = child_count_distribution_.begin(); itr != child_count_distribution_.end(); ++itr) {
      out << "  " << itr->second << " elements with " << itr->first << " children " << endl;
    }
  }

  void print_report() { print_report(std::cout); }
};

#endif
