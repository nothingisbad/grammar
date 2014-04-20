#ifndef DEFINEREGULARLANGUAGE_HH
#define DEFINEREGULARLANGUAGE_HH

/**
 * @file grammar/DefineGrammar.hh
 * @author Ryan Domigan < ryan_domigan@students.uml.edu >
 * ryan_domigan@students.uml.edu 
 *  Created on Saturday 29 2012  
 */

#include <functional>
#include <utility>

#include "./Match.hpp"
#include "./Rule.hpp"
#include "./Branch.hpp"
#include "./Reduce.hpp"
#include "./PutBack.hpp"
#include "./If.hpp"
#include "./Stop.hpp"

#include "./SyntaxError.hpp"

#include "./GrammarChain.hpp"
#include "./GrammarTree.hpp"



namespace grammar {
  class Parser;

  /**
   * Can be used to construct a regular grammer which in turn can construct a ParseStrings object.
   * wraps the type GrammarTree with all kinds of accessors to make defining a grammar with that class
   * more practical.
   *
   * DefineGrammar behaves somewhat like smart_ptr.  When it is copy constructed it takes the internal grammar definition from
   * the DefineGrammar it is copying.  __The 'copied' DefineGrammar is left with its internal data structure NULLed out__.
   *
   * DefineGrammar is cheap to create/destroy as long as it's internal Grammar is consumed (ie when a stack allocated DefineGrammar object is
   * consumed by DefineGrammar::branch(...) ).
   *
   * Modified to act like an auto_ptr based on notes I googled up at http://www.informit.com/guides/content.aspx?g=cplusplus&seqNum=390
   * 
   * Note: I think I can define a context free rather language with this.
   *
   * @see GrammarTree
   */
  class DefineGrammar {
  public:
    GrammarTree *_grammar;	/**< the actual grammar I'm defining'*/
  private:
    /* static Pattern *non_ws; /\**< static instance for pattern matching (basically a singleton class) *\/ */
    
    friend class Parser; 		/* so I can share the internal classes */
    /* mutable bool owns_grammar_;	/\**< ownership of contained pointer *\/ */
    Branch *branch_start();
    
    GrammarTree* release_grammar() {
      GrammarTree *result = _grammar;
      _grammar = NULL;
      /* owns_grammar_ = false;	/\* mutable for this reason *\/ */
      return result;
    }

    /**
     * Templatized member adds variable number of cases to the current branch.
     * 
     * @tparam T must always be DefineGrammar (reqiured to be a template for the syntax to work)
     * @param car
     * @param cdr
     */
    template<typename... T>
    void push_cases(DefineGrammar &&car, T... cdr) {
      _grammar->push_case( car.release_grammar() );
      push_cases( std::forward<T>(cdr)... );
    }

    /* degenerate case. */
    void push_cases() { return; }

    /* ! rule to reduce pereviously scanned string     */
    DefineGrammar&& reduce(Reduce::ActionType act) {
      _grammar->reduce(act);
      return std::move(*this);
    }

  public:
    DefineGrammar() { _grammar = new GrammarTree(); }

    ~DefineGrammar() {
      if(_grammar) delete _grammar;
    }

    /* Stealing constructor */
    DefineGrammar(DefineGrammar &&orig) : _grammar( orig.release_grammar() ) {}

    DefineGrammar(const DefineGrammar&) = delete; /* I don't want copy construction (at least while I don't have cloning utility)*/

    /* Swaps */
    DefineGrammar& operator=(DefineGrammar&& src) {
      /* strip casting */
      if((void*)&src != (void*)this) {
	if(_grammar)
	  delete _grammar;
	/* owns_grammar_ = src.owns_grammar_; */
	_grammar = src.release_grammar();
      }
      return *this;
    }

    //! make a regular expression
    DefineGrammar&& re(const std::string &re) {
      _grammar->scan(new Pattern(re));
      return std::move(*this);
    }
    
    //! make a case insensitive regular expression
    DefineGrammar&& re_i(const std::string &re) {
      Pattern *reg = new Pattern(re);
      reg->set_flag(boost::regex::icase);
      _grammar->scan(reg);
      return std::move(*this);
    }
    
    /**
     * Call hook on the last string accumulator, then clear the string accumulator.
     * 
     * @param hook the hook to call
     * @param index index of the match to pass into hook (like Match, 0 is whole match, 1 is first capture etc)
     * @return: this
     */
    DefineGrammar&& on_string(std::function<void(const std::string&)> hook, int index = 0) {
      return reduce( [=](Match &scanned) mutable {
	  hook( scanned[index] );
	});
    }

    DefineGrammar&& on_match(std::function<void(Match&)> hook) {
      return reduce( hook );
    }

    /**
     * Call thunk.  This does not consume input, append to accumulator, or have any other effect on the parser (other than transitioning to the
     * next Rule)
     *
     * @see DefineGrammar#call
     * @tparam F type of the hook to call
     * @param hook the hook to call
     * @param fake ignored value (required for template resolution)
     * @return 
     */
    template<class F>
    DefineGrammar&& thunk( F hook ) {
      /* wrap the hook with a string-taking closure.  I'm ignoring 'ignore', so it will not be modified through over this call. */
      return reduce( [=](Match& ignored) mutable { hook(); });
    }

    DefineGrammar&& ignore() {
      return reduce( [=](Match& ignored) { });
    }
    
    //! when this branch is reached, put the scanned string back into input
    DefineGrammar&& put_back() {
      _grammar->grammar.push_back<PutBack>();
      return std::move(*this);
    }

    DefineGrammar&& put_back(const std::string& input) {
      _grammar->grammar.push_back( PutBackLiteral(input) );
      return std::move(*this);
    }

    /**
     * Variable arity branch.  As far as I can tell, you can only use the new c++11 variadic form with
     * templatized functions.  Therefor this function is templatized, however it will only work correctly for
     * is_same<T, DefineGrammar>.
     */
    template<typename... T>
    DefineGrammar&& branch(T&&... lst) {
      using namespace std;
      _grammar->push_branch();
      push_cases( forward<T&&>(lst)... ); /* the arguments must be forwarded to preserve rvalue references */
      return std::move(*this);
    }
    
    //syntax for labeling and going to labels
    DefineGrammar&& go(const std::string& label) {
      _grammar->go(label);
      return std::move(*this);
    }

    DefineGrammar&& label(const std::string& name) {
      _grammar->label(name);
      return std::move(*this);
    }
    
    //! tells the grammar to print an error if no matches found
    DefineGrammar&& error(const std::string& msg) {
      std::string &capture = const_cast<std::string&>(msg);
      return reduce( [=](Match &context) mutable { throw SyntaxError( capture.append(context[0]) ); } );
    }
    
    DefineGrammar&& append(DefineGrammar &input) {
      _grammar->append( input.release_grammar() );
      return std::move(*this);
    }

    DefineGrammar&& append(DefineGrammar &&input) {
      _grammar->append( input.release_grammar() );
      return std::move(*this);
    }

    /* //sets the name of the toplevel label */
    /* DefineGrammar&& set_name(const std::string& name ); */

    //! debugging helpers    
    std::string str() { return _grammar->str(); }
    static SyntaxError* get_default_error() { return nullptr; }

    //! Print current state of the branch
    void print() { _grammar->print(std::cout); }

    //! follow a branch if a std::function thunk evaluates to true.
    DefineGrammar&& _if( std::function<bool ()> test, DefineGrammar &&consiquent) {
      _grammar->append_free_list(consiquent._grammar);
      _grammar->merge_tables(consiquent._grammar);
      _grammar->grammar.push_back( If(test, consiquent.release_grammar()->begin()) );
    
      return std::move(*this);
    }

    //! stop the parsing (tell parser to wait for more input, current input, but not the current capture, is discarded)
    DefineGrammar&& stop() {
      _grammar->grammar.push_back( Stop() );
      return std::move(*this);
    }
  };
}
#endif
