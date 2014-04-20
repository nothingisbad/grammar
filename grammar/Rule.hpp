#ifndef RULE_HH
#define RULE_HH
/**
 * @file grammar/Rule.hh
 * @author Ryan Domigan < ryan_domigan@students.uml.edu >
 *  Created on Sunday 30 2012  
 */

#include <string>
#include <set>
#include <ostream>

#include "./Match.hpp"

namespace grammar {
  class PrintRecursiveRule;
  class Branch;
  class DetectCycle;

  /**
   * a grammar rule, may be a reduction, scanner, branch, or linking rule.  The
   * Rules implement an intrusive list which is abstracted by GrammarChain and the GrammarTree.
   *
   * @see GrammarChain
   * @see GrammarTree
   */
  class Rule {
  public:
    virtual ~Rule() {}    /* destructor only exists to create a vtable entry. */

    /**
     * Returns the Rule which this object expects to be followed by.
     * @return the following Rule
     */
    virtual Rule* get_default()=0;
    
    /**
     * Sets the Rule which should follow this one.
     * @param next the following Rule
     */
    virtual void set_default(Rule* next)=0;

    /**
     * string representation of the Rule.  Used for printing and
     * debugging.
     * @return the string representation.
     */
    virtual std::string str() = 0;

    /**
     * Implements parsing states.  When a class inherits Rule it's expected to overload and perform scanning with
     * Rule* operator()(std::string&,std::string&, bool&).  The Rule takes characters from input and
     * put them into scanned if they require further processing.
     * 
     * @param scanned characters which have been scanned
     * @param input characters provided by the Parser
     * @param more_input_required If set to true, the Parser should stop what it's doing and wait for more characters from its source.
     * @return the next rule for parser to apply
     */
    virtual Rule* operator()(Match &scanned, std::string &input, bool &more_input_required)=0;

    virtual void print(PrintRecursiveRule &out_);
  };

  /* Checks to see if I've visited rule while printing */
  class DetectCycle {
    std::set<Rule*> seen_;
  public:
    /**
     * default constructor.
     */
    DetectCycle() {}

    /**
     * check if I've seen a rule before, if I have just return true.  If I haven't update the internal datastructure to say that I have and
     * return false
     */
    bool seen_beforeP(Rule *r) {
      if( seen_.find(r) == seen_.end() ) {
	seen_.insert(r);
	return false;
      }
      return true;
    }
  };

  /**
   * Prints elements of Rule, sub-elements of those and so on, detecting cycles during the process
   */
  class PrintRecursiveRule : public DetectCycle {
    std::ostream &out_;

    int indent_;

    void print_spaces() {
      for(int i = 0; i < indent_; ++i) 	out_ << ' ';
    }
  public:
    /**
     * Cannot default construct because osteam cannot default construct
     */
    PrintRecursiveRule(std::ostream &out) : out_(out) { indent_ = 0; }

    /**
     * print a newline followed by some indentation (as indicated by indent_ member)
     */
    void newline() {
      out_ << "\n";
      print_spaces();
    }

    /**
     * print an address
     */
    void print(const void* pntr) { out_ << std::hex << pntr << std::flush; }
  
    /**
     * sent a c style string to member ostream
     */
    void print(const char* str) { out_ << str << std::flush; }

    /**
     * sent a string to member ostream
     */
    void print(const std::string& str) { print (str.c_str()); }
  
    /**
     * for use with recursive decent, checks the seen_ set to see if I've already printed the argument,
     * if not, then pass myself into it's print function, otherwise use it's str() function (which had better not be
     * recursive).
     *
     * @param r: the rule in need of printing
     */
    void print(Rule *r) {
      if(r == NULL)
	print("NULL");
    
      else if( !seen_beforeP(r) ) {
	print("++");
	r->print(*this);
      }
      /* if we've seen the rule arlready, don't print some identifying things, but don't recurse. */
      else {
	print("<");
	print( static_cast<void*>(r) );
	print(":");
	print( r->str() );
	print(">");
      }
    }

    /**
     * increase the indentation after a call to newline()
     */
    void indent_more() { indent_ += 4; }

    /**
     * decrease the indentation after a call to newline()
     */
    void indent_less() { indent_ -= 4; }
  };
  
  /**
   * default print action leveraging str
   */
  void Rule::print(PrintRecursiveRule &out_) {
    out_.print( str() );
    if( this->get_default() )
      out_.print( this->get_default() );
    else
      out_.print("->NULL");
  }
}

#endif
