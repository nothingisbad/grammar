#ifndef GRAMMAR_SIMPLEDEFAULT_HH
#define GRAMMAR_SIMPLEDEFAULT_HH
/**
 * @file /home/ryan/uml/computing-IV/a07/grammar/SimpleDefault.hh
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Dec 02, 2012
 */

#include "./Rule.hpp"

namespace grammar {
  /**
   * defines a basic faculty for getting and setting a default follow rule.
   */
  class SimpleGetSetDefault : public Rule {
  protected:
    Rule *_default;
  public:
    virtual Rule* get_default() { return _default; }
    virtual void set_default(Rule* rule) { _default = rule; };
  };
}

#endif
