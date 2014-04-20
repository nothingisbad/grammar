#ifndef PATTERN_HPP
#define PATTERN_HPP

#include <string>

#include "./Match.hpp"

namespace grammar {

  /***************************************/
  /*  ____       _   _                   */
  /* |  _ \ __ _| |_| |_ ___ _ __ _ __   */
  /* | |_) / _` | __| __/ _ \ '__| '_ \  */
  /* |  __/ (_| | |_| ||  __/ |  | | | | */
  /* |_|   \__,_|\__|\__\___|_|  |_| |_| */
  /***************************************/
  /**
   * Parent class for patterns.  Patterns are functors which implement a find
   * function for scanning input strings for matches.
   * 
   * Behavior implemented through overload at the call
   */
  class Pattern {
  private:
    boost::regex _pattern;	/**< the regular expression Pattern is wrapping. */
    std::string _str;		/* keep a note of the string I've used to make the regex */
  public:
    Pattern() = delete;
    Pattern(const Pattern& pat) = default;
    ~Pattern() = default;
    
    /**
     * simple constructor, build a pattern for regex based on str
     * @param str regular expression
     */
    Pattern(const std::string &str) : _pattern(str), _str(str) {}
  
    /**
     * check Pattern property
     * @return true if the pattern can match anywhere, false if it only matches
     * the start of a string
     */
    bool scanningP() { return true; }
  
    /**
     * Patterns should implement a find function which identifies the start of 
     * a match.
     * @param str input string
     * @return match position (or std::string::npos if no match)
     */
    bool find(Match &match) {
      return boost::regex_search(match._input, match.match, _pattern);
    }

    /**
     * a string representation of the Pattern, useful for printing and 
     * debugging.
     * @return the string representation.
     */
    std::string str() {
      /* todo: something more useful to print out  */
      std::string s("/");
      s.append( _str ).append("/");

      if( _pattern.flags() & boost::regex_constants::icase) s.append("i");

      return s;
    }

    /**
     * set _pattern
     * @param input pattern to use
     */
    void set_regex(const std::string &input) { _pattern = input; }

    /**
     * set the flags
     * @param flag flag to use
     */
    void set_flag(boost::regex::flag_type flag) { _pattern = boost::regex(_str, flag); }
  };
}

#endif
