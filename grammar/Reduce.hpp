#ifndef GRAMMAR_REDUCE_HH
#define GRAMMAR_REDUCE_HH
/**
 * @file grammar/Reduce.hh
 * @author Ryan Domigan < ryan_domigan@students.uml.edu >
 *  Created on Sunday 30 2012  
 */
#include "./Rule.hpp"
/* #include "./ReduceAction.hpp" */

#include <iostream>
#include <functional>
#include <string>

namespace grammar {
  /**
   * Base class for reducing semantic actions
   */
  class Reduce : public Rule {
  public:
    typedef std::function<void (Match&) > ActionType;
  protected:
    Rule* default_;		/**< following rule */
    ActionType action_;	/**< action to take on scanned string (does not specify follow up Rule)*/
  public:
    /**
     * reduce default constructor, zero's default
     */
    Reduce() {
      default_ = NULL;
      action_ = NULL;
    }
  
    /**
     * trivial destructor
     */
    ~Reduce() {}

    /**
     * reduce types are reductions, return true
     *
     * @return true
     */
    bool reductionP() { return true; }
  
    /**
     * string representation of Reduce
     *
     * @return a string
     */
    std::string str() {
      using namespace std;
      stringstream out;
      out << "<reduction-rule:" << hex << (void*)this << ">";
      return out.str();
    }
  
    /**
     * default getter for children to use
     *
     * @return default next action
     */
    Rule* get_default() {
      return default_;
    }
  
    /**
     * default action setter for children to use
     *
     * @param r default follow up action
     */
    void set_default(Rule *r) {
      default_ = r;
    }

    /**
     * Sets the action to call on reduction
     * 
     * @param r
     */
    void set_action(ActionType r) {
      action_ = r;
    }

    /**
     * applies the action_ to the scanned string
     * 
     * @param scanned
     * @return next (get_default()) rule
     */
    Rule* operator()(Match &scanned, std::string &input, bool &more_chars) {
      action_( scanned );
      more_chars = false;		/* reduce never needs more chars in input*/

      return get_default();
    }
  };
}

#endif
