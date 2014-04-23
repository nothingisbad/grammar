#ifndef GRAMMAR_IF_HH
#define GRAMMAR_IF_HH
/**
 * @file /home/ryan/uml/computing-IV/a06/grammar/If.hh
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Nov 20, 2012
 */

#include <functional>
#include <string>

#include "./Reduce.hpp"
#include "./SimpleGetSetDefault.hpp"

namespace grammar {
  /**
   * Follow a branch if some C++ expression evaluates to true.
   */
  class If : public SimpleGetSetDefault {
    std::function<bool ()> _test; /**< predicate function. */
    Rule *_consiquent;		  /**< rule to follow if the predicate is satisfied. */
  public:
    /**
     * Construct an If object.
     */
    If(std::function<bool ()> test, Rule *consiquent) 
      : _test(test) , _consiquent(consiquent) {}

    /**
     * Uses the Reduce interface to implement a condional.
     * 
     * @param ignored string input is ignored
     * @return the next Rule to evaluate
     */
      Rule* operator()(Match &ignored, std::string &ignored1, bool &more_chars) {
      more_chars = false;
      if( _test() ) return _consiquent;
      else return get_default();
    }

    /**
     * string representation
     * @return "<if>"
     */
    std::string str() { return "<if>"; }

  };
}

#endif
