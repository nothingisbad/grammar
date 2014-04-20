#ifndef GOTOLABEL_HH
#define GOTOLABEL_HH
/**
 * @file grammar/GotoLabel.hh
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Oct 09, 2012
 */
#include "./Label.hpp"
#include "./Reduce.hpp"
#include "Singleton.hpp"

namespace grammar {
  /**
   * goto a Label, not get_default() next.
   */
  class GotoLabel : public Rule {
    Label *label_;		/**< the label this Goto will connect to. */
    Rule *_default; 		/**< isn't reached by get_default, but can be used to add elements to a grammar (which may make sense so long
				   as the added elemenets have a label I can jump into) */
  public:
    /**
     * construct a GotoLabel with a given Label
     */
    GotoLabel(Label *l = nullptr) {
      label_ = l;
    }

    /**
     * label_ setter
     * 
     * @param l new label value
     */
    void set_label(Label *l) {
      label_ = l;
    }

    /**
     * ignores scanned string, returns the label->get_destination() value
     *
     * @param scanned ignored string, complies with Reduce interface.
     * @return Rule I'm going to.
     */
    Rule* operator()(Match &scanned, std::string &input, bool &more_chars) {
      using namespace std;
      more_chars = false;
    
      if( RunVerbose<GotoLabel>::P() ) {
	cout << " Going to: " << label_->str() <<endl;
      }
      return label_->get_default();
    }

    /**
     * constructs a string representation based on the name of the Label I'm referring to and the typename
     * of the class
     * 
     * @return assembled string
     */
    std::string str() {
      if(label_)
	return std::string("<goto ").append(label_->str() ).append(">");
      else
	return "<goto UNRESOLVED>";
    }

    /**
     * instead of returning my own default, return Label's
     * 
     * @return the action following my label
     */
    Rule* get_default() {
      if( RunVerbose<GotoLabel>::P() ) {
	std::cout << " GotoLabel returning default: " << label_->str() <<std::endl;
      }
    
      if(label_)
	return label_->get_default();
      else
	return NULL;
    }

    /**
     * Requred as a child of Rule.  The 'default' of the Goto will never be reached, but may be used to link in branches which are part of the
     * grammar.
     * 
     * @see GotoLabel#set_label
     * @param r the rule to link in
     */
    void set_default(Rule *r) {
      _default = r;
    }
  };
}

#endif
