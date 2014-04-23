/**
 * @file /home/ryan/uml/computing-IV/a05/main.cpp
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Nov 06, 2012
 *
 * mush together the DOM generating part of CreateDOMDocument with DOMPrint.  I'm going to use the STL libraries rather than the C includes
 * where I can.
 * 
 * See README.txt for usage and notes.
 * 
 */

#define DBG_GRAMMAR_BRANCH

#include <fstream>
#include <iostream>
#include <functional>
#include <deque>
#include <string>
#include <functional>

#include "./grammar/grammar.hpp"
#include "./grammar/utility.hpp"

#include "./XmlElement.hpp"
#include "./XmlSemanticAction.hpp"
#include "./XmlPrint.hpp"
#include "./JSONPrint.hpp"
#include "./debug.hpp"
#include "./grammar/grammar.hpp"

#include "./parse_command_line.hpp"

/**
 * @mainpage xml-parser
 * @section Summary
 * Parser Xml and translator
 *
 * @section Usage
 * ./xml-parser [options] xml_file_name
 * 
 * xml-parser is meant to be invoked on the command line with a file name argument.  If no file name is provided, xml-parse
 * will attempt to read xml from std::cin.
 * 
 * @section Options
 *
 * -o [name] : output file.  if name is not provided, but an input file name is, xml2json will strip the .xml extention from the input name and
 *    output to a file of the same name with a .json extention
 *
 * --verbose [Branch|Until|GotoLabel|xml_grammar|xml_parser|tree]: turns on verbose evaluation of one or more type.  The exact behavior
 *   varies, but this will generally cause information about the application of operator()(...) functions to print during
 *   evaluation
 *
 * --summary: prints a few statistics about the input XML file to std::cout
 *
 *
 * --help or -h: prints available options.
 *   
 */


/**
 * main routine, builds a grammar and uses the parser against input
 */
int main(int argc, char *argv[]) {
  using namespace std;
  using namespace std::placeholders;
  using namespace grammar;
  

  XmlPrint verbose_printer;
  JSONPrint printer(std::cout);

  istream *input_stream;	/* an input stream for my parser. */
  ostream *output_stream;	/* an output stream for my printer. */
  
  Parser xml_parser; 		/* object that does the actual parsing */
  unique_ptr<ifstream> in_file_cleanup; /* cleans up input file handle (set by parse_command_line)*/
  unique_ptr<ofstream> out_file_cleanup; /* cleans up output file handle. */
  
  /* check the command line for file name and other configuration */
  parse_command_line(input_stream, in_file_cleanup
		     , output_stream, out_file_cleanup
		     ,argc, argv);

  printer.set_out_stream(output_stream); /* set the stream to which I will print my json */

  /* set up some actions */
  XmlSemanticAction xml_action;

  auto on_open = std::bind(&XmlSemanticAction::on_open, ref(xml_action), _1);
  auto on_close = [&xml_action](const string& str) { xml_action.close(str); };

  auto content = std::bind(&XmlSemanticAction::content, ref(xml_action), _1);

  auto attribute_name = std::bind(&XmlSemanticAction::on_attribute_name, ref(xml_action), _1);
  auto attribute_value = std::bind(&XmlSemanticAction::on_attribute_value, ref(xml_action), _1);
  
  auto on_self_close = std::bind(&XmlSemanticAction::on_self_close, ref(xml_action));
  
  /* build my grammars.
   * block scoped for automatic cleanups.
   */
  {
    DefineGrammar
      xml_rules			/* defines the parsing rules for XML*/
      , xml_in_tree;            /* as soon as I define a root element, I'm in tree and can't have a Declare */
  
    /* rules for parsing once we've found the first tag' */
    xml_in_tree 
     = xml_in_tree.label("in-tree")
      .re("([^<]*)").on_string(content,1) /* grab the content we've passed since the opening tag */
      .branch( re("^\\s*</\\s*([^>[:space:]]*)\\s*>").on_string( on_close, 1)
	       .go("in-tree")  /* closing tags */

	       , re("^\\s*<!--").label("comment")
	       .branch( re(".*-->").go("in-tree")

			, re(".*").go("comment"))
	       
	       /** open tags **/
	       , re("\\s*<([^>/[:space:]]*)").on_string(on_open, 1).label("tag-loop").re("^\\s*").ignore()
	       .branch( re("^>").go("in-tree")

			, re("/>").thunk(on_self_close).go("in-tree")

			, otherwise()
			.re("(\\s*[^>=[:space:]]*?)\\s*?=").on_string(attribute_name,1)
			.re("\"(.*?)\"").on_string( attribute_value, 1)
			.go("tag-loop")
			)

	       , re("<\\?").error("xml declaration must be at top-level.")

	       , otherwise().error("Don't konw how to handle tag.")
	       ).go("in-tree");
    //<end xml_in_tree>********************
 
    /* rules for parsing when we first open the file */
    xml_rules
     = xml_rules.label("toplevel-rule")
      .re("[^<]*").thunk( [](){} ) /* discard */
      .branch( re("<\\?.*\\?>").go("toplevel-rule") /* ignore the xml declaration (assumes single line)
						       todo: count them, should only be one. */

	       , re("^\\s*<!--").label("toplevel-comment")
	       .branch( re(".*-->").go("toplevel-rule")
			, re(".*").go("toplevel-comment"))
	       
	      , re("^\\s*</").error("close tag with no open tags")

	       , otherwise().re("^\\s*").go( "in-tree" ) /* must be the root tag,
									  descend into the tree */
	       ).append( xml_in_tree );
    //<end xml_rules>********************

    xml_parser.sink( xml_rules ); /* set the parser to caputre my grammar before the
					Define objects are destroyed*/

    if( RunVerbose<debug_xml_grammar>::P() ) {
      /*** Print out current grammar ****/
      cout<<"My grammar: "<<endl;
      xml_parser.print(cout);
      cout<<endl;
    }
  }

  /* start off the seek for parser */
  (*input_stream) >> ws;

  try {  
    /* pass each line of input to the parser */
    foreach_line(*input_stream
		 , [&](std::string& input) {
		   xml_parser(input);  /* parse the current string, */
		   xml_action.line_end(); /* increment the line count. */
		 });
    
  } catch(SyntaxError &e) {
    cout << "Line " << xml_action.get_line() << ": Syntax Error. " << e.what() << endl;
    return 1;
  } catch(XmlException &e) {
    cout << "Line " << xml_action.get_line() << ": Unbalanced tags " << e.what() << endl;
    return 1;
  }

  cout << "XML File looks OK.  Printing collected element information. \n\n" << endl;

  /* did I ask to print out the xml_tree (in a non-json form)? */
  if( Singleton<bool,debug_xml_tree>::get())
    verbose_printer( xml_action.get_result() );

  /* did I pick the --summary option? */
  if( Singleton<bool,brief_report>::get() ) {
    xml_action.print_report();
  }
  
  printer.print_root( xml_action.get_result() );

  cout << "\nDone." << endl;    

  return 0;
}

