#ifndef SYNTAXERROR_HH
#define SYNTAXERROR_HH
/**
 * @file grammar/SyntaxError.hh 
 * @author Ryan Domigan < ryan_domigan@students.uml.edu >
 *  Created on Monday 01 2012  
 */

#include "./Reduce.hpp"
#include <string>

namespace grammar {
  /**
   * Simple class implementing an operator()(std::string) so that it can be wrapped by
   * Reduce_wrap_ref and used to signal syntax errors
   *
   * Uses Reduce defaults except for operator()
   */
  class SyntaxError : public std::exception {
    std::string message_;
  public:
    /**
     * construct a SyntaxError with provided message
     */
    SyntaxError(const std::string &msg) { message_ = msg; }

    /**
     * trivial destructor, no throw contract
     */
    ~SyntaxError() throw() {}

    /**
     * throw a syntax error with the scanned string appended to message_
     *
     * @param scanned message to throw
     */
    void operator()(std::string &scanned) {
      message_.append(" {with input: ").append(scanned).append("}");
      throw *this;
    }

    /**
     * string representation, common interface with most of my other objects
     * 
     * @return the string representation
     */
    std::string str() const { return message_; }
  
    /**
     * c-string representation implementing the exceptoin::what() virtual
     * 
     * @return string representation of SyntaxError
     */
    const char* what() const throw() { return str().c_str(); }
  };

}

#endif
