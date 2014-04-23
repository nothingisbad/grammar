#ifndef GRAMMAR_BRANCH_HPP
#define GRAMMAR_BRANCH_HPP
/**
 * @file grammar/Branch.hh
 * @author Ryan Domigan < ryan_domigan@students.uml.edu >
 *  Created on Sunday 30 2012  
 */

#include <vector>
#include <utility>

#include "./Pattern.hpp"
#include "./Rule.hpp"
#include "./SimpleGetSetDefault.hpp"
#include "./SyntaxError.hpp"
#include "./Singleton.hpp"

namespace grammar {
  /* a default value; treated as always matched by Branch */
  class Otherwise : public SimpleGetSetDefault {
  public:
    Otherwise() = default;
    virtual std::string str() { return std::string("Otherwise"); }
    virtual Rule* operator()(Match &scanned, std::string &input, bool &more_chars_) {
      return get_default();
    }
  };
  
  /**
   * The general branch type.  If a match is found the string to that point is pushed to scanned and a rule corrisponding to that match
   * is passed up to Parser if a match is not found, the entire input string is appended to scanned (unless dont_capture() has been called)
   * and a re-scan rule is passed up to Parser.
   */
  class Branch : public SimpleGetSetDefault {
  private:
    /**
     * Simple data type, tells branch how to make a match and which rule to use with it 
     */
    class TestAndScan {
    public:
      /* Pattern *meta_char_; */
      Pattern *pattern;	/**< pattern the scan matches. */
      /* todo: the regex match works differently; get rid of do_capture */
      bool do_capture;		/**< if true, save the matched string, otherwise discard it. */
      Rule *rule;		/**< rule this pattern implies*/
    };
    
    typedef std::vector<TestAndScan > match_vec_type;
    match_vec_type match_rules_;
    
    bool do_capture_
      , more_chars_;
    
    /**
     * copies the non-rescanner members of orig.
     * @param orig original Branch I'm copying from.
     */
    void copy_members(const Branch& orig) {
      match_rules_ = orig.match_rules_;
      do_capture_ = orig.do_capture_;
      more_chars_ = orig.more_chars_;
    }

  public:
    typedef match_vec_type::iterator iterator;  /**< iterator type for the 
						   match/rule pairs */
    /**
     * constructs an empty Branch.
     */
    Branch() {
      match_rules_.clear();
      do_capture_ = true;
      more_chars_ = false;
    }
  
    /**
     * copy constructs Branch.  Shallow copy.
     *
     * @param orig branch I'm copying
     */
    Branch(const Branch& orig) { copy_members(orig); }
  
    /**
     * destructor.
     * todo: free the sub branches
     */
    ~Branch() { }

    /**
     * instructs the branch to ignore scanned characters.
     */
    void dont_capture() { do_capture_ = false; }

    /**
     * lets the Parser know if the rule can use more chars.
     */
    bool more_charsP() { return more_chars_; }
  
    /**
     * add a conditional branch to this.
     * 
     * @param delim on this delim
     * @param rule apply this rule
     * @param capture indicate if I should continue capturing input or discard (scan vs seek)
     */
    void add_branch(Pattern *delim, Rule *rule, bool capture = true) {
      TestAndScan ts;
      ts.pattern = delim;
      ts.rule = rule;
      ts.do_capture = capture;
    
      match_rules_.push_back( ts );
    }

    /* this action (not it's default) will always occure if reached by the branch. */
    void add_default(Rule *rule) {
      TestAndScan ts;
      ts.pattern = nullptr;
      ts.rule = rule;
      ts.do_capture = false;
    
      match_rules_.push_back( ts );
    }

    /**
     * append the TestAndScans of branch subsequent to your own.
     *
     * @param b branch to append
     */
    void append(Branch &b) {
      match_vec_type::iterator i;
      for(i = b.match_rules_.begin(); i != b.match_rules_.end(); ++i)
	match_rules_.push_back( *i );
    }
  
    /**
     * The principle scanning routine; Branch checks raw against each of it's string matching members, if a match is found
     * branch sets it's parameter best to reflect that pattern.  Best will always reset best, even if it does not find a match.
     *
     * @param scanned: the characters matched by prior scan calls
     * @param raw: the input characters that have not been part of any matches
     * @return the next rule the parser should use in evaluating input
     */
    Rule* operator()(Match &best, std::string &raw, bool &more_input) {
      using namespace std;
      /* if the input is empty, I'm not going to try and make a match.  Signal the parser I need more chars and
	 return. */
      if(raw.empty()) {
	more_input = true;
	return this;
      }

      Match pos;
      pos.set_input(raw);
      best = pos;
    
      iterator best_rule = match_rules_.end();

#ifdef DBG_GRAMMAR_BRANCH
      if( RunVerbose<Branch>::P() ) {
	std::cout<<"\n**Branch:"<<std::hex<<dynamic_cast<void*>(this)<<"**";
	std::cout<<"  looking at: |" << raw<< "|" << std::endl;
      }
#endif

      /* test each rule against raw input for a match, and pick the match closest to
	 the beginning of raw */
      for(auto rule = match_rules_.begin(); rule != match_rules_.end(); ++rule) {
	/* find a canidate match */
	if( rule->pattern == nullptr ) { /* hit an otherwise */
	  return rule->rule;

	} else if( rule->pattern->find(pos) ) {
	  if(pos.match.position() == 0) { /* best case; take first matching rule */
	    best = pos;
	    best_rule = rule;
	    break;
	  }

	  if( best.match.empty() || pos.match.position() < best.match.position() )  {	    
	      best = pos;
	      best_rule = rule;
	    }
	}}

      /* if I found a pattern match, scan the string and return the assosiated rule */
      if(best_rule != match_rules_.end()) {
	raw = best.suffix(); 	/* update raw to contain only the un-matched portion. */
	
#ifdef DBG_GRAMMAR_BRANCH
	if( RunVerbose<Branch>::P() ) {
	  std::cout<<"found match to "<< best_rule->pattern->str() <<std::endl;
	  std::cout<<" and split string:"<< best[1] <<"|"<< raw <<std::endl;
	  std::cout<<"    and rule type: "<< best_rule->rule->str() <<std::endl;
	}
#endif
	
	return best_rule->rule;
      } else if( more_charsP() ) {
	more_input = true;	/* signal the parser I want more chars */
	return this;
      } else if( _default != nullptr ) {
	return _default;
      }

      else	/* if I don't accept more_charsP(), throw a SyntaxError ( *todo: should probably check default too) */
	/* branch_underflow_error: */
	throw SyntaxError(std::string("scanning ")
			  .append( str() )
			  .append(" expected a delim before the newline.  Got: ")
			  .append(raw) );
    }
  
    /**
     * simple string indicating the object's type
     */
    std::string str() { return std::string("Branch"); }

    /**
     * using PrintRecursiveRule print out every scan and parse rule in the branch.
     *
     * @param out the printer object
     */
    void print(PrintRecursiveRule& out) {
      out.print("<Branch:");
      out.print(static_cast<void*>(this));
      out.print(">");
    
      /* print the conditional branches */
      for(match_vec_type::iterator i = match_rules_.begin();
	  i != match_rules_.end();
	  ++i) {
	out.indent_more();
	out.newline();
	out.print("scans: ");
	if(i->pattern == nullptr)
	  out.print("Otherwise ");
	else
	  out.print(i->pattern->str());

	out.indent_more();
	out.newline();
	out.print(i->rule);
	out.indent_less();
    
	out.indent_less();
      }

      /* print the default */
      if( get_default() ) {
	out.indent_more();
	out.newline();
	out.print("default: ");
	out.print( get_default() );
	out.indent_less();
      }
    }

    /**
     * If the Branch has only a default and no conditional rules it may be redundant, so I provide this test.
     *
     * @return: true if only the default is defined
     */
    bool default_only() { return (get_default() != nullptr) && match_rules_.empty(); }

    /**
     * @return true if there are no default rules, otherwise false
     */
    bool rules_empty() { return match_rules_.empty(); }

    /**
     * @return true if the branch is empty 
     */
    bool empty() {
      return ( get_default() == NULL) && match_rules_.empty();
    }

    /**
     * Rather than re-implement all the vector accessors so I can use Branch with the stl algorithms, I'm
     * using a reference accessor.
     * 
     * @return case vector of 'this' branch
     */
    match_vec_type& get_case_vector() { return match_rules_; }
  };
}

#endif
