#ifndef JSONPRINT_HH
#define JSONPRINT_HH
#include "./XmlElement.hpp"

/**
 * @file JSONPrint.hpp
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Oct 15, 2012
 *
 * Object (similar to XmlPrint) which prints a tree as JSON.
 */

/**
 * an object for recursivly printing perfect trees in the JSON format
 * @pre tree must be acyclic.
 */
class JSONPrint {
  friend class Scope;
  std::ostream *out_;
  
  /**
   * A formatter for a given scope of code.  Calls to members allow me to track when to write a comma
   * and how much to indent the given line.
   */
  class Scope {
    JSONPrint *master_;		/**< enclosing object. */
    Scope *up_;			/**< enclosing scope. */
    std::string indent_;	/**< The current indentation level. */
    bool first_;		/**< true if this is the first invocation of listing(). */
    XmlElement *prev_;
  public:
    /**
     * increments the indent count
     * @param idnt: reference to the current block
     */
    Scope(Scope &scp) : master_(scp.master_) , indent_(scp.indent_) {
      first_ = true;
      indent_.append("  ");
      master_->current_ = this;
      up_ = &scp;
    }

    /**
     * constructor for bottom_ Scope (sets up to push enclosing).
     */
    Scope(JSONPrint *mstr) : master_(mstr) {
      up_ = NULL;
      master_->current_ = this;
      first_ = true;
    }

    /**
     * decrement the indent ocunt
     */
    ~Scope() {
      master_->current_ = up_; 		/* effectivly pops the stack */
    }

    /**
     * prints a comma and newline (where applicable in a listing context).
     * @pre the beginning of a listing
     * @post a comma (after the first call) and newline are printed, then ostream out is indented
     * @return a ostream reference to print the next listing
     */
    std::ostream& listing() {
      using namespace std;
      if(first_) {
	*master_->out_ << '\n';
	first_ = false;    
      }
      else
	*master_->out_ << ",\n";
  
      return indent();
    }

    /**
     * just indent, nothing else.
     * @return ostream for further output.
     */
    std::ostream& indent() {
      return *master_->out_ << indent_ << std::flush;
    }

  };
  
  Scope bottom_;
  Scope* current_;
  
  /**
   * returns the Scope formatter for the current block.
   * @return 
   */
  Scope& current_block() { return *current_; }
  
  /**
   * print the body of an element in the manner of a JSON value.
   * @param elem the element whos body I should print.
   */
  void print_body(XmlElement *elem) {
    using namespace std;
    /* name of the current element */
    string name = elem->get_tag_name();
    /* vector of the (element) children  */
    vector<XmlElement*> children = elem->children;

    /* put the children in alphabetical order */
    sort(children.begin(), children.end());

    /* see if the element has content  */
    if( elem->have_contentP() 
	&& !(elem->have_attributesP() || elem->have_childrenP() ))
      {
	*out_ << "\"" << elem->get_content() << "\"";
	return;
      }
  
    *out_ << "{";
    {
      Scope s( current_block() );
    
      if( !elem->get_content().empty() ) {
	/* pretty sure I can just pick any name... */
	s.listing() << "\"content\" : \"" << elem->get_content() << "\"";
      }
  
      /* if I have attributes, printthem */
      if( elem->have_attributesP() ) {
	elem->foreach_attribute( *this );
      }

      for(XmlElement::ElementContainer::iterator itr = children.begin();
	  itr != children.end();
	  ++itr) 
	{
	  /* print elements with matching names */
	  if(((itr + 1) != children.end()) && ((*itr)->get_tag_name() == (*(itr + 1))->get_tag_name())) {
	    current_block().listing() << "\"" << (*itr)->get_tag_name() << "\" : [" ;
	    {
	      Scope s2(current_block());
	      do {
		current_block().listing(); /* indent and comma */
		print_body(*itr);
		++itr;
		/* if the next next element is the end of list, treat that as a special case */
		if((itr + 1) == children.end()) {
		  current_block().listing();
		  print_body(*itr); /* print the current element*/
		  break;		   /* and quit the loop */
		}
	      } while( ((*itr)->get_tag_name() == (*(itr + 1))->get_tag_name()));
	    }
	    *out_ << "\n";
	    current_block().indent() << "]";
	  }
	
	  /* prints one-off elements */
	  else {
	    print_elem(*itr);
	  }
	}
    }
    *out_ << '\n';
    current_block().indent() << "}";
  }

  
public:
  /**
   * constructor for JSONPrint.  
   * @out ostream to use for output.
   */
  JSONPrint(std::ostream& out = std::cout)
    : out_(&out)
    , bottom_(this) { current_ = &bottom_; }



  /**
   * recursively print root and its children.  I may yet make a format-tree based printer, but for now I can
   * practice the Big Switch approach.
   * 
   * @param elem the element to print
   */
  void print_elem(XmlElement *elem) {
    current_block().listing() << "\"" << elem->get_tag_name() << "\" : " ;
    print_body(elem);
  } 

  /**
   * prints the root element with the javascript testing harness.
   * 
   * @param elem Xml-tree's root element.
   */
  void print_root(XmlElement *elem) {
    using namespace std;
    current_block().listing() << "var " << elem->get_tag_name() << " = ";
    print_body(elem);
    *out_ << "\nconsole.log( \"JSON data loaded properly.\" )" << endl ;
  }


  /**
   * Print out an attribute
   * 
   * @param attrib attribute to print
   */
  void operator()(XmlAttribute &attrib) {
    using namespace std;
    current_block().listing() << '"' << attrib.get_name() << "\" : "
			      << '"' << attrib.get_value() << "\"";
  }

  /**
   * set output stream.
   * @param out stream to use
   */
  void set_out_stream(std::ostream* out) {
    out_ = out;
  }
};



#endif
