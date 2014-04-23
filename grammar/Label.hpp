#ifndef GRAMMAR_LABEL_HH
#define GRAMMAR_LABEL_HH
/**
 * @file grammar/Label.hh
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Oct 09, 2012
 */
#include "./SimpleGetSetDefault.hpp"

namespace grammar {
  /**
   * type for building grammars, labels a point in the grammer which GotoRule can use
   */
  class Label : public SimpleGetSetDefault {
    std::string name_;		/**< name of the Label*/
  public:
    /**
     * initialize an empty Label with a default name.
     */
    Label() {
      name_.clear();
      _default = NULL;
    }

    /**
     * copy constructor.
     * 
     * @param orig label to copy
     */
    Label(const Label &orig) {
      name_ = orig.name_;
      _default = orig._default;
    }

    /**
     * initialize an empty Label with provided name
     * 
     * @param name sets the label name
     */
    Label(const std::string &name) {
      name_ = name;
      _default = NULL;
    }

    /**
     * gets the name of label
     * 
     * @return copy of the name string
     */
    std::string get_name() {
      return name_;
    }
  
    /**
     * sets the name of label to a new value
     * 
     * @param new_name new name
     */
    void set_name(const std::string &new_name) {
      name_ = new_name;
    }

    /**
     * type labeled name string
     * 
     * @return string representation of instance
     */
    std::string str() {
      using namespace std;
      stringstream out;
      out << "<label " << hex << static_cast<void*>(this) <<":"<< name_ << ">";
      return out.str();
    }
    /**
     * Returns the default rule, does nothing to the strings.  This could be optomized out on parser generation.
     * 
     * @param scanned string scanned so far
     * @param input un-processed input
     * @return next rule to apply
     */
    Rule* operator()(Match &scanned, std::string &input, bool &more_chars) {
      more_chars = false;
      return get_default();
    }
  };
}

#endif
