#include <fstream>
#include <iostream>
#include <functional>
#include <deque>
#include <string>

#include "./grammar/grammar.hh"
#include "./utility.hh"

#include "./XmlElement.hh"
#include "./XmlSemanticAction.hh"
#include "./XmlPrint.hh"
#include "./JSONPrint.hh"
#include "./debug.hh"
#include "./grammar/grammar.hh"

#include "./parse_command_line.hh"

/* letting exceptions throw through the toplevel lets GDB show me where the error is being emitted.  There may be an easier way to get
   the same information, but this works. */
#define CATCH_AND_PRINT_EXCEPTIONS

/**
 * @file main.cc
 * @author Ryan Domigan  ryan_domigan@students.uml.edu 
 * Created on Friday 28 2012  
 */



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
  using namespace grammar;
  

  XmlPrint verbose_printer;
  JSONPrint printer;

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

  ReduceAction *on_open
    = new ReduceWithMember<XmlSemanticAction
			   , &XmlSemanticAction::on_open >(&xml_action);

  ReduceAction *on_close
    = new ReduceWithMember<XmlSemanticAction
			   , &XmlSemanticAction::on_close > (&xml_action);

  ReduceAction *on_content = xml_action.action<&XmlSemanticAction::on_content>();
  ReduceAction *attribute_name = xml_action.action<&XmlSemanticAction::on_attribute_name>();

  ReduceAction *attribute_value
    = xml_action.action<&XmlSemanticAction::on_attribute_value>();
  
  ReduceAction *on_self_close
    = xml_action.action<&XmlSemanticAction::on_self_close>();
  
  /* build my grammars.
   * block scoped for automatic cleanups.
   */
  {
    DefineGrammar
      xml_rules			/* defines the parsing rules for XML*/
      , xml_in_tree;            /* as soon as I define a root element, I'm in tree and can't have a Declare */
  
    /* rules for parsing once we've found the first tag' */
    xml_in_tree 
     = xml_in_tree.set_name("in-tree")
      .scan("<").reduce(on_content) /* grab the content we've passed since the opening tag */
      .branch( match("/").scan(">").reduce(on_close)  /* closing tags */
	       , match("!--").scrub("-->").discard()  /* comments */
	       , match("?").error("xml declaration must be at top-level.")
	       
	       /** open tags **/
	       , scrub().branch(scan("/>").reduce(on_open).reduce(on_self_close) /* self closing (doesn't modify stack)*/
				, scan(">").reduce(on_open) /* no attributes*/
				/* with attributes*/
				/* todo: note: was scan_ws(), temporarily */
				, scan( WhiteSpace() ).reduce(on_open) /* capture the name */
				.scrub()
				.label("attributes-loop").scrub().branch( scan("=").reduce(attribute_name).scrub("\"").scan("\"").reduce(attribute_value) /* an attribute*/
									  , scan(">").go("in-tree")
									  , scan("/>").reduce(on_self_close).go("in-tree")
									  
									  ).discard().go("attributes-loop")
				)
	       ).scrub().go("in-tree");
    //<end xml_in_tree>********************
 
    /* rules for parsing when we first open the file */
    xml_rules
     = xml_rules.set_name("toplevel-rule")
      .scan("<")
      .branch( match("?").scrub("?>").discard()	    /* ignore the xml declaration
						       todo: count them, should only be one. */
	      , match("!--").scrub(">").discard()   /* comments allowed at toplevel */
	      , match("/").error("close tag with no open tags")	   /* closing tag, which  is an error */
	      , scrub().put_back("<").loop( xml_in_tree )           /* must be the root tag, descend into the tree */
	      );
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
#ifdef CATCH_AND_PRINT_EXCEPTIONS
  try {  
#endif
  
    /* pass each line of input to the parser */
    foreach_line(*input_stream
		 /* lambda function */
		 , [&](std::string& input) {
		   xml_parser(input);  /* parse the current string, */
		   xml_action.line_end(); /* increment the line count. */
		 });
    
#ifdef CATCH_AND_PRINT_EXCEPTIONS
  } catch(SyntaxError &e) {
    cout << "Line " << xml_action.get_line() << ": Syntax Error. " << e.what() << endl;
    return 1;
  } catch(XmlException &e) {
    cout << "Line " << xml_action.get_line() << ": Unbalanced tags " << e.what() << endl;
    return 1;
  }
#endif

  cout << "XML File looks OK.  Printing collected element information. \n\n" << endl;

  /* did I ask to print out the xml_tree (in a non-json form)? */
  if( Singleton<bool,debug_xml_tree>::get())
    verbose_printer(xml_action.get_result() );

  /* did I pick the --summary option? */
  if( Singleton<bool,brief_report>::get() ) {
    xml_action.print_report();
  }
  
  printer.print_root( xml_action.get_result() );

  cout << "\nDone." << endl;    

  return 0;
}

