#ifndef GRAMMAR_PARSER_HH
#define GRAMMAR_PARSER_HH
/**
 * @file grammar/Parser.hh
 * @author Ryan Domigan <ryan_domigan@students.uml.edu>
 *  Created on Saturday 29 2012  
 *
 */

#include "./DefineGrammar.hpp"

namespace grammar {
  /**
   * Should be constructed by DefineGrammar.  A generated parser which expects
   * to be fed strings.  It will apply the rules defined by DefineGrammar
   * until the string it is applied to is empty.
   *
   * Parser is a sink for a DefineGrammar's GrammarTree.  Parser expects to have exclusive non-transferable
   * ownership of the GrammarTree object.  When the parser dies, it simply deletes the grammar.
   *
   * It maintains state between application so that it can be fed files one line at a time.
   */
  class Parser {
    friend class DefineGrammar;
    GrammarTree *_root;	   /**< starting point for the grammar, used for resets and printing. */
    Rule *_rule;	   /**< current rule to scan or reduce with */
    Match _scanned; /**< between invocations the Parser may have scanned some characters which have not yet 
		       been reduced  */
  public:
    Parser(const Parser&) = delete; 	/**< forbidden. */
    /**
     * default construct empty
     */
    Parser() : _root(nullptr), _rule(nullptr) {}

    /**
     * destructor destroys the grammar object.
     */
    ~Parser() { delete _root; }

    /**
     * parse a string untill it is consumed using the rules definined by my grammar.
     * Implements a dispatching trampoline for operator() overloaded Rule objects.
     *
     * @param input the string to parse
     */
    void operator()(std::string& input) {
      using namespace std;
      bool more_input_needed = false;
      while(_rule && !more_input_needed)
	_rule = (*_rule)(_scanned, input, more_input_needed);
    }
  
    /**
     * alternate form of operator(), parses a c-style string
     * @param input 
     */
    void operator()(const std::string &input) {
      std::string copy = input;
      (*this)(copy);
    }

    /**
     * print representation to a steam
     *
     * @param out stream to print into
     */
    void print(std::ostream& out) {
      /* checks for cycles and conditionally prints each item in my tree. */
      PrintRecursiveRule do_print(out);
      do_print.print( _rule );
    }

    /**
     * print to default stream (for debugging)
     */
    void print() { print(std::cout); }

    /**
     * begins parsing the next line with the root of my GrammarTree object.
     * 
     * @post the parser will begin parsing the next input with its initial state.
     */
    void reset() { _rule = _root->begin(); }
  
    /**
     * true if _rule is NULL (can't do anything with more strings until reset)
     * @return true if _rule == NULL
     */
    bool is_leaf() { return _rule == NULL; }


    /**
     * Takes control of a GrammarTree pointer from a DefineGrammar
     * the && won't collapse unless I'm taking Grammar as a template parameter.
     * @param def DefineGrammar object I'm releasing
     */
    template<class Grammar>
    void sink(Grammar &&def) {
      using namespace std;
      /* it would be nice if I could do this checking at compile time...  */
      if(!def._grammar->fully_resolvedP()) {
	std::string msg("Parser cannot sink grammar; symbols are unresolved: ");
	/* append the names of all the unresolved symbols to the error message */
	def._grammar->for_unresolved([&](const std::string& s) { msg.append(s).append(" ");});
	throw std::runtime_error(msg);
      }
    
      _root = def.release_grammar();
      _rule = _root->begin();
      reset();
    }
  };
}
#endif
