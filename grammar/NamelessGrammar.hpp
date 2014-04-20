#ifndef NAMELESSGRAMMAR_HH
#define NAMELESSGRAMMAR_HH
/**
 * @file grammar/NamelessGrammar.hh
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Oct 02, 2012
 *
 * functions for creating grammars which are heap allocated but not
 * intended for lvalue assignment
 */

#include "./DefineGrammar.hpp"

namespace grammar {
  /**
   * makes a grammar which immeditaly jumps to a label.
   * @return 
   */
  DefineGrammar go(const char* label) {
    return (DefineGrammar()).go(label);
  }

  /**
   * build a regular expression attached to an anonymouse DefineGrammar
   * 
   * @param match_patttern the pattern I will be matching
   * @return the anonymouse grammar
   */
  DefineGrammar re(const std::string &match_patttern) {
    return (DefineGrammar()).re(match_patttern);
  }

  /**
   * regular expression, case insensitive.
   * 
   * @param match_patttern
   * @return 
   */
  DefineGrammar re_i(const std::string &match_patttern) {
    return (DefineGrammar()).re_i(match_patttern);
  }

  DefineGrammar label(const std::string &ll) { return (DefineGrammar()).label(ll); }

  DefineGrammar otherwise() {
    DefineGrammar result;
    result._grammar->grammar.push_back<Otherwise>( );
    return result;
  }
}

#endif
