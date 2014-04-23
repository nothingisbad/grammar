#ifndef XMLELEMENT_HH
#define XMLELEMENT_HH

/**
 * @file XmlElement.hpp
 * @author Ryan Domigan ryan_domigan@students.uml.edu
 * 2012-9-13
 *
 * The XmlElement type and simple parse and print helpers.
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "./grammar/utility.hpp"
#include "./grammar/Singleton.hpp"
/**
 * @internal
 * non-instantiable class used for template parametrization.
 */
struct sort_child_elements;

/**
 * represents an attribute of an XML element.
 */
class XmlAttribute {
  std::string _name;		/**< attribute name */
  std::string _value;		/**< attribute value */
public:
  XmlAttribute(const std::string& n, const std::string& v = "") : _name(n), _value(v) {}
  XmlAttribute();

  std::string get_name() const { return _name;  }


  std::string get_value() const { return _value; }
  
  void set_value(const std::string& str) { _value = str; }
};

/**
 * class represents an Xml element, it can be an open or close tag
 */
class XmlElement {
public:
  /** container type for children (which are XmlElements) */
  typedef std::vector< XmlElement* > ElementContainer;
  ElementContainer children;	/**< children of this element */
private:

  /** container type for attributes */
  typedef std::vector<XmlAttribute> AttributeContainer;

  std::string tag_name_ 	/**< name of tag */
    , content_;			/**< contents of tag */
  int line_number_;		/**< line number on which tag
				   was opened */

  AttributeContainer attributes_; /**< attributes of this element instance */
  typedef AttributeContainer::value_type AttributePair;
  std::string top_attribute_name_; /**< most recently defined attribute name (anticipates a value will come next)*/
public:
  /**
   * @return true if this < b in a lexical ordering; line number is used for equivalent tag-names.
   */
  bool operator<(const XmlElement& b) const {
    int result;

    if( (result = tag_name_.compare( b.get_tag_name() ))
	== 0 )
      return line_number_ < b.line_number_;

    return result < 0;
  }

  template<class Func>
  void foreach_attribute(Func& fn) {
    using namespace std;
    for(AttributeContainer::iterator itr = attributes_.begin();
	itr != attributes_.end();
	++itr) {
      fn(*itr);
    }
  }

  template<class Func>
  void foreach_child(Func &fn) {
    using namespace std;

    /* sort the children (should be handy for JASON output later) */
    /* JSONPrint requires child elements to be sorted for correct behaviour, so this flag is moot when outputing JSON. */
    if( grammar::Singleton<bool, sort_child_elements>::get() )
      std::sort(children.begin(), children.end()
		, [&](XmlElement *a, XmlElement *b) -> bool { return *a < *b; });

    for(auto itr : children) fn(itr);
  }

  /**
   * constructs an element with line number and tag name defined.
   * @param line line number on which element was defined
   * @param name name of the element
   */
  XmlElement(int line, const std::string& name) {
    tag_name_ = name;
    line_number_ = line;
  }

  /**
   * constructs an empty element
   */
  XmlElement(){
    line_number_ = 0;
    tag_name_.clear();
  }

  /**
   * copy value of input
   */
  XmlElement(const XmlElement& input){
    set_line_number(input.get_line_number());
    set_tag_name(input.get_tag_name());
  }

  /**
   * (trivial) cleanup of element
   */
  ~XmlElement(){}

  /**
   * gets the line number assosiated with the current element
   */
  int get_line_number() const { return line_number_; }

  /**
   * sets the line number assosiated with the current element
   */
  int set_line_number(int num){
    line_number_ = num;
    return line_number_;
  }

  /**
   * gets the tag name of element
   */
  std::string get_tag_name() const { return tag_name_; }

  /**
   * sets the tag name of element
   */
  void set_tag_name(const std::string& new_name){ tag_name_ = new_name; }

  /**
   * returns a string representation of element
   */
  std::string toString(){ return tag_name_; }


  /**
   * pushes a new attribute onto local stack
   * 
   * @param name name of the attribute
   */
  void push_attribute(const std::string& name) {
    attributes_.push_back( XmlAttribute(name) );
  }

  /**
   * sets the value of the last-defined attribute.
   * @param value
   */
  void set_attribute_value(const std::string& value) {
    attributes_.back().set_value(value);
  }

  /**
   * append new content to current value.  Escapes quotes the input content.
   * 
   * @param cont content to append
   */
  void add_content(const std::string& cont) {
    using namespace std;
    size_t pos;
    content_.append(cont);
  
    /* look for quotes in string, and prepend with backslash. */
    pos = content_.find("\"");
    while(pos != string::npos) {
      /* check that the " is not already escaped */
      if( (pos > 1) && (content_[pos - 1] != '\\'))
	  content_.insert(pos, "\\");      

      pos = content_.find("\"", pos + 2);
    }
  }

  /**
   * check if this element has content
   * @return true if the XmlElement has content, else false
   */
  bool have_contentP() { return !content_.empty(); }

  /**
   * retrieve the content string for this element
   *
   * @return string value
   */
  const std::string& get_content() { return content_; }

  /**
   * push a child to this element - used to form tree representation of xml
   * 
   * @param elem child to push
   */
  void push_child(XmlElement *elem) { children.push_back(elem); }


  /**
   * does the element have attribute?
   * @return true if it has attributes, else false
   */
  bool have_attributesP() { return !attributes_.empty(); }

  /**
   * count of element attributes
   * @return # of attributes
   */
  int count_attributes() { return attributes_.size(); }

  /**
   * does the element have children?
   * @return true if it has children, else false
   */
  bool have_childrenP() { return !children.empty(); }

  /**
   * count of element children
   * @return # of children
   */
  int count_children() { return children.size(); }
};

/****************************** helper classes ******************************/
/**
 * exception for bad Xml reads (unbalanced open/close etc)
 */
class XmlException : public std::exception {
  /** what message  */
  std::string message_; 
public:
  XmlException() = default;
  XmlException(const std::string& msg) throw() : message_(msg) {}
  virtual ~XmlException() throw() {}
  
  virtual const char* what() const throw() { return message_.c_str(); }
};

#endif
