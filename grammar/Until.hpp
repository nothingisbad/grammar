#ifndef UNTIL_HH
#define UNTIL_HH
/**
 * @file grammar/Until.hh
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Oct 09, 2012
 */

#include "./SimpleGetSetDefault.hpp"
#include "./Pattern.hpp"

namespace grammar {
  /**
   * builds the scan string until a pattern is found (discards matching part).
   * 
   * Branch has the capacity to represent this action, but adding a primitive case as it's significantly simpler
   * to test and build on.
   */
  class Until : public SimpleGetSetDefault {
    Pattern *_pattern;		/**< a pattern (wrapped regex)*/
  public:
    /**
     * default constructs an Until form
     */
    Until() : _pattern(nullptr) {}

    /**
     * construct a copy of another Until
     * 
     * @param orig instance to copy
     */
    Until(const Until& orig) { _pattern = orig._pattern; }

    /**
     * creates an Until matching pattern m
     *
     * @param m pattern to match
     */
    Until(Pattern *m) { _pattern = m; }

    /**
     * trivial destructor
     */
    ~Until() {}
  
    /**
     * implements scanning by looking for Pattern _pattern, if it's found scan the string up to the match, otherwise keep
     * appending input to scanned
     * 
     * @param scanned input scanned so far
     * @param input un-scanned string
     * @return next rule to apply
     */
    Rule* operator()(Match &match, std::string& input, bool &more_chars) {
      using namespace std;
    
#ifdef DEBUG_UNTIL
      if(RunVerbose<Until>::P() ) {
	cout << "\n**Until: " << hex << dynamic_cast<void*>(this) << "***\n"
	     << " looking for " << _pattern->str() << "\n"
	     << " in " << input
	     << endl;
      }
#endif
      match.set_input(input);

      if( _pattern->find(match) ) {
	/* keep string after match as input */
	input = match.suffix();
	more_chars = false;
#ifdef DEBUG_UNTIL
	if(RunVerbose<Until>::P() ) {	
	  cout << " and split string: |" << match[0] << "|" << input << "|" << endl;
	}
#endif
	return get_default();
      }

      /* no match, ask for more input */
      else {
#ifdef DEBUG_UNTIL
	if(RunVerbose<Until>::P() ) {
	  cout << "couldn't find it " << endl;
	}
#endif

	more_chars = true;
	return dynamic_cast<Rule*>(this);
      }
    }

    /**
     * name string for Until
     * 
     * @return string representation
     */
    std::string str() {
      using namespace std;
      return string("<until {").append(_pattern->str()).append("}>");
    }

    /**
     * sets _pattern to a different value
     * 
     * @param input new value for pattern
     */
    void set_pattern(Pattern *input) {
      _pattern = input;
    }

    /**
     * getter for _pattern
     * 
     * @return the pattern I scan until
     */
    Pattern* get_pattern() {
      return _pattern;
    }
  };
}

#endif
