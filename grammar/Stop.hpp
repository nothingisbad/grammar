#ifndef STOP_HH
#define STOP_HH
/**
 * @file /home/ryan/uml/computing-IV/a07/grammar/Stop.hh
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Dec 06, 2012
 */
#include "./Label.hpp"
namespace grammar {
  /**
   * Just inherits because label is the simplest class I can think of right now
   */
  class Stop : public Label {
  public:
    Rule* operator()(Match &scanned, std::string &input, bool &more_chars) {
      more_chars = true;
      return get_default();
    }
  };
}

#endif
