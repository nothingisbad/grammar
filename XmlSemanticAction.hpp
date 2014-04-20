#ifndef XMLPARSERSEMANTICS_HPP
#define XMLPARSERSEMANTICS_HPP
/**
 * @file XmlSemanticAction.hpp
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Oct 07, 2012
 */

#include <vector>
#include <string>

#include "./XmlElement.hpp"
#include "./XmlStats.hpp"

#include "./utility.hpp"

/**
 * non-instantiable class, used for template key-ing
 */
class debug_xml_parsing;

/**
 * maintatins a stack of XmlElements and has call back generating functions
 * meant to facilitate stack interaction
 */
class XmlSemanticAction {
  //! structure containing currently open XmlElements 
  std::vector<XmlElement*> stack_;
  
  XmlElement *result_; 		/**< when the root tag closes, put it here so I can reference it */
  int line_count_; 	        /**< current input line, as reported by line_end() */
  XmlStats stats_;		/**< tracks some statiscs on XmlElements defined with this action */
public:
  XmlSemanticAction() : line_count_(1) {}
  
  XmlElement* top() { return stack_.back(); }
  XmlElement* get_result() { return result_; }

  //! returns the current line number.
  int get_line() { return line_count_; }
  
  //! print short description of stack to std::cout
  void print_stack_brief() {
    using namespace std;
    std::vector<XmlElement*>::iterator itr;
    for(itr = stack_.begin(); itr != stack_.end(); ++itr) {
      cout << "|" << (*itr)->get_tag_name() << "| ";
    }
    cout << endl;
  }

  //! callback when I've got an open tag
  void on_open(const std::string& name) {
    using namespace std;
    stack_.push_back(new XmlElement(line_count_,name));

    if( grammar::RunVerbose<debug_xml_parsing>::P() ) {
      cout << "  ***     Parser: " << this << endl;
      cout << "  ***Opening tag: " << name << endl;
      cout << "  ***       line: " << line_count_ << endl;
      cout << "  ***      Stack: ";
      print_stack_brief();
      cout << endl;
    }
  }

  //! action to take when I've got a close tag
  void close(const std::string& name) {
    using namespace std;
    XmlElement *tmp; 		/* take the top element then re-push it as a child of the next node down.  This'll hold
				   the value between the pop and push */
    if(grammar::RunVerbose<debug_xml_parsing>::P() ) {
      cout << "  ***     Parser:" << this << "\n";
      cout << "  ***closing tag: " << name << endl;
      cout << "  ***       line: " << line_count_ << endl;
      cout << "  ***      Stack: ";
      print_stack_brief();
      cout << endl;
    }  

    if(stack_.back()->get_tag_name() != name)
      throw XmlException( string("Unbalanced open/close tags: got ").append(name).append(" expected ").append(stack_.back()->get_tag_name() ));

    tmp = stack_.back();
    stack_.pop_back();

    /* log some stats on the element I just closed */
    stats_.analyze(tmp);
  
    /* keep the last element in the stack so I can retrieve it */
    if( stack_.empty() ) {
      result_ = tmp;
    }
    else {
      stack_.back()->push_child(tmp);
    }
  }

  //! action to take when I've got content
  void content(const std::string& cc) {
    using namespace std;
  
    if( grammar::RunVerbose<debug_xml_parsing>::P()) {
      cout << "  ***              Parser:" << this << "\n";
      cout << "  *** Getting new content: " << cc << endl;
      cout << "  ***                line: " << line_count_ << "\n";
      cout << "  ***           Stack: ";
      print_stack_brief();
      cout << endl;
    }

    /* this reduction will occure before any elements have been added to the stack, so check first.  */
    if(!stack_.empty())
      top()->add_content(cc);
  }

  //! got name of attribute
  void on_attribute_name(const std::string &name) {
    if( grammar::RunVerbose<debug_xml_parsing>::P()) {
      std::cout << "  *** attribute: " << name << "\n";
      std::cout << "  ***      line: " << line_count_ << "\n";
    }
    top()->push_attribute(name);
  }

  //! got value of attribute
  void on_attribute_value(const std::string &value) { top()->set_attribute_value(value); }

  void on_self_close() {
    using namespace std;
    std::string name = stack_.back()->get_tag_name();
    XmlElement *tmp;
				   
    tmp = stack_.back();
    stack_.pop_back();

    if(grammar::RunVerbose<debug_xml_parsing>::P() ) {
      cout << "  ***self closing tag: " << name << endl;
      cout << "  ***            line: " << line_count_ << endl;
      cout << "  ***           Stack: ";
      print_stack_brief();
      cout << endl;
    }  


    /* log some stats on the element I just closed */
    stats_.analyze(tmp);
  
    /* keep the last element in the stack so I can retrieve it */
    if( stack_.empty() ) {
      result_ = tmp;
    }
    else {
      stack_.back()->push_child(tmp);
    }
  }
  
  /**
   * Tells the parser that the current input line is finished, and to increment the line_count_;
   */
  void line_end() { ++line_count_; }

  void print_report() {
    using namespace std;
    stats_.print_report();
    cout << "Toplevel element has " << get_result()->count_children() << " children." <<endl;
  }
};


#endif
