#ifndef XMLPRINT_HH
#define XMLPRINT_HH
/**
 * @file XmlPrint.hpp
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Oct 12, 2012
 */
#include "./XmlElement.hpp"


/**
 * an object for recursivly printing perfect trees of XmlElements
 *
 * @pre tree must be acyclic
 */
class XmlPrint {
  int indent_;			/**< indentation level of current element*/

  /**
   * increments a reference to indent_ on construction, decrements on destruction (makes keeping track of
   * indentation levels easy, also cares for non-local exits and the like, although that's not a big concern
   * here)
   */
  class Indent {
    int &indent_;
  public:
    Indent(int &indt)  : indent_(indt) { indent_ += 2; }
    ~Indent() { indent_ -= 2; }
  };

  void do_indent() {
    using namespace std;
    for(int i = 0; i < indent_; ++i) {
      if(i % 2)
	cout << ' ';
      else
	cout << '.';
    }
    cout << flush;
  }
public:
  XmlPrint() : indent_(0) {}
  
  void operator()(XmlElement* root) {
    using namespace std;
    if(root == nullptr) {
      cout << "Null Element." << endl;
      return;
    }
  
    do_indent();
    cout << "Element \"" << root->get_tag_name()
	 << "\" begins on line " << root->get_line_number();
    if( root->get_content().empty() ) {
      cout  << " with no content ";
    } else {
      cout << " with content \"" << root->get_content() << "\" ";
    }

    /* if I have attributes, printthem */
    if(root->have_attributesP() ) {
      cout << "and attributes:";
      Indent i(indent_);
      root->foreach_attribute( *this );
      /* cout << "\n" */
    }
    /* otherwise say I don't (have attributes) */
    else {
      cout << "and no attributes.";
    }

    cout << endl;
  
    /* If I have children, recursively print them */
    Indent i(indent_);
    do_indent();
  
    cout << "with ";
    switch( root->count_children() ) {
    case 0:
      cout << "no children.";
      break;
    
    case 1:
      cout << "one child:";
      break;
    
    default:
      cout << root->count_children() << " children:";
    }
    cout << endl;

    root->foreach_child(*this);
  }

  void operator()(XmlAttribute& attrib) {
    using namespace std;
    cout << "\n";
    do_indent();
    cout << "{ Name:" << attrib.get_name()
	 << " Value: " << attrib.get_value()
	 << " }";
  }

  void operator()(const std::string& attrib) {  std::cout << " " << attrib; }
};

#endif
