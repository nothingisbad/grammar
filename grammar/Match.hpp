#ifndef GRAMMAR_MATCH_HH
#define GRAMMAR_MATCH_HH
/**
 * @file /home/ryan/uml/computing-IV/a07/grammar/Match.hh
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Dec 06, 2012
 */
#include <boost/regex.hpp>
#include <string>

namespace grammar {
  /**
   * boost::.match are invalidated when the string to which they refer is mutated, therefore I'm bundling together string and smatch
   */
  class Match {
    friend class Pattern;
  private:
    std::string _input;
  public:
    boost::smatch match;

    Match() {}
    Match(const Match& m) : _input(m._input) , match(m.match) {}

    Match& set_input(const std::string &str) {
      _input = str;
      return *this;
    }

    std::string operator[](int index) {
      return match[index].str();
    }

    std::string str() { return match.str(); }
    std::string suffix() { return match.suffix(); }

    /** trivial wrapper of boost::match begin */
    decltype(match.begin()) begin() { return match.begin(); }
    
    /** trivial wrapper of boost::match end */
    decltype(match.end()) end() { return match.end(); }
  };
}

#endif
