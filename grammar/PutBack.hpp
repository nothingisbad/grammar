#ifndef PUTBACK_HPP
#define PUTBACK_HPP
/**
 * @file PutBack.hh
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Oct 02, 2012
 */

#include "./SimpleGetSetDefault.hpp"

namespace grammar {
  /**
   * puts characters back from scanned into the beginning of input string.
   * invoked by the Parser as a Scanner, overloads
   * Rule* operator()(std::string &scanned, std::string &raw);
   */
  class PutBack : public SimpleGetSetDefault {
  public:
    /**
     * reverse scanning, empties scanned and prefixes raw with scanned contents
     */
    Rule* operator()(Match &scanned, std::string &raw, bool &more_chars) {
      using namespace std;
      more_chars = false;
      string tmp = scanned[0];

      raw = tmp + raw;
      return get_default();
    }

    /**
     * string representation of PutBack
     */
    std::string str() { return "<PutBack>"; }
  };


  /**
   * copies characters into the input string from an internal string.
   * 
   * invoked by the Parser as a Scanner, overloads
   * Rule* operator()(std::string &scanned, std::string &raw);
   */
  class PutBackLiteral : public SimpleGetSetDefault {
  private:
    std::string putting_back_;	/**< characters to prepend to input */
  public:

    /**
     * forms a PutBackLiteral with contents
     * @param str contents
     */
    PutBackLiteral(const std::string& str) : putting_back_(str) {}

    /**
     * reverse scanning, prefixes raw with object contents
     */
    Rule* operator()(Match &scanned, std::string &raw, bool &more_chars) {
      /* std::cout<< "Putting back "<<putting_back_<<std::endl; */
      more_chars = false;

      std::string tmp = putting_back_;
      tmp.append(raw);    
      raw = tmp;

      return get_default();
    }

    /**
     * string representation of PutBack
     */
    std::string str() {
      return "<PutBackLiteral>";
    }
  };
}

#endif
