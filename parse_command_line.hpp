#ifndef PARSE_COMMAND_LINE_HH
#define PARSE_COMMAND_LINE_HH
/**
 * @file parse_command_line.hpp
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Oct 12, 2012
 */
#include <fstream>
#include <memory>

/**
 * non-instantiable class, used for template key-ing.  Used for a configuration singleton
 * which tells my main loop to skip printing the full XML tree and instead print some
 * summarizing statistics (checks the validity of my xml input WRT the grading criteria)
 */
class brief_report;

/**
 * prints out some help information then, regardless of other options, exits.
 * 
 * @param ignored Not used, just gives function the right signature for use
 *  with ReduceWithFunction.  
 */
void print_help() {
  using namespace std;
  cout << "Usage:\n"
       << "xml-parser [options] [file-name]\n"
       << "availible options:\n"
       << " --verbose [Branch|UntilGotoLabel|xml_grammar|xml_parser]\n"
       << " --summary : print summary of input stats rather than full json\n"
       << " -o (out-file-name|$): if out-file is not specified, but in-file is, out-file is set to in-file ~= s/\\.xml/\\.json/ \n"
       << " --help or -h: print this help and exit.\n"
       << "For usage please see Doxygen docs.\n"
       << endl;

  exit(0);
}

/**
 * parses the input given to main to set various singletons other prameters.  I am using a unique_pointer and stream* for both the input
 * and output stream. The unique_pointer manage the life-span of a stream, but the stream* objects are used to actually send and recieve
 * characters, this allows me to use cout and cin as defaults without worrying about them getting deleted (because the unique_pointers are left
 * as NULL in that case).
 * 
 * @param input : the input stream to use
 * @param input_cleanup : a unique_pointer (the c++11 version of auto_ptr) which cleans up input as needed (when it's not cin)
 * @param output : the output stream to use
 * @param output_cleanup : a unique_pointer which cleans up the output stream as needed
 * @param argc : the number of command line args + invoked name
 * @param argv : vector of command strings
 */
void parse_command_line(std::istream*& input, std::unique_ptr<std::ifstream>& input_cleanup
                        , std::ostream*& output, std::unique_ptr<std::ofstream>& output_cleanup
                        ,  int argc, char *argv[]) {
  using namespace grammar;
  using namespace std;

  Parser command_parse;         /* parsers the command-line input */
  /* Parser mangle_name;                /\* mangles the infile name so it works as an output file *\/ */
  DefineGrammar rule;
  string in_file_name              /* name of the input file */
    , out_file_name;               /* name of the output file */
  
  bool use_infile_name_for_outfile = false; /* if I need a name for an output file, but haven't been given one by the caller, use
                                               the input file's name (but change the extentiont) */
  rule 
   = rule.label("command-line")
    .branch(                    /* prints different kinds of status information while running  */
            re("--verbose")
	    .label("parse-verbose")
            .branch(re("Branch").thunk( &RunVerbose<Branch>::run_verbose )
		    /* prints infromation relating to Until objects (used by non-branching scans)  */
		    , re("Until").thunk( &RunVerbose<Until>::run_verbose )
                       
                    /* prints information about setting and following labels */
                    , re("GotoLabel").thunk( &RunVerbose<GotoLabel>::run_verbose )
                       
                    /* prints the grammar tree used to parse the XML  */
                    , re("xml_grammar").thunk( &RunVerbose<debug_xml_grammar>::run_verbose )
                       
                    /* prints some infromation about parser state on reduction  */
                    , re("xml_parser").thunk( &RunVerbose<debug_xml_parsing>::run_verbose )

		    , re("tree").thunk([]() {
			Singleton<bool,debug_xml_tree>::set(true);
			std::cout << "called --verbose tree" << std::endl;
		      })

		    /* if the scanned string doesn't match any of of the paterns, treat it as an in-file name. */
		    , re("").go("file-name" )

		    ).go("parse-verbose")
            
             /* summary option, which prints information about the parsed xml rather than the json form. */
	    , re("--summary").thunk( bind(Singleton<bool,brief_report>::set,true) )
             
             /* prints a brief overview of options. */
	    , re("--help").thunk( print_help )
	    , re("-h").thunk( print_help )

            /* output to a file */
            , re("-o")
            /* if I'm not given the name of the output file by the caller, I can still try to mangle the input file name  */
                .thunk([&]() { use_infile_name_for_outfile = true; })
                .branch( /* scan for a dash, which indicates my next string is a command rather than name, so use
                            the infile_name (if provided) with a mangled extention. */
                        re("-").put_back().go("command-line")
                        
                        /* if there are chars, scan to the end of the line and use that as my out file name. */
                        , re("(\\w)*").on_string( [&](const string& name) {
                            cout << "**getting outfile name: " << name << endl;
                            out_file_name = name;
                          } )
                         )
            /* if the string doesn't match any command, assume it the name of the input file */
	    , otherwise().label("file-name").re("([^[:space:]])*").on_string( [&](const string& s) { in_file_name = s; } )
             );
  /**** end command-line ****/
  
  /* build the parser using the grammar */
  command_parse.sink(rule);

  for(int i = 1; i < argc; ++i)
    command_parse(argv[i]);

  /* check if there's an input-file name */
  if(!in_file_name.empty()) {
    /* set the infile pointer and the cleanup object */
    input_cleanup.reset(new ifstream(in_file_name.c_str()));
    input = input_cleanup.get();

    if(!input_cleanup->is_open()) {
      cout << "Couldn't open " << in_file_name << endl;
      throw XmlException("no input");
    } 

    cout << "Opened " << in_file_name << " for input." << endl;
  }
  /* set the default input stream */
  else {
    cout << "No file specified; using cin." << endl;
    input = &cin;
  }

  /* if the user specifies a -o flag, but not an out_file name try to mangle the in_file_name and use that.  */
  if( out_file_name.empty() && use_infile_name_for_outfile ) {
    if(in_file_name.empty())
      throw runtime_error("-o option used, but no in or out file name provided.");

    /* build a mini-parser to strip the .xml off the end of the file name */
    Parser mangle_name;
    mangle_name.sink( re("(.*)\\.xml").on_string( [&](const string &input) { out_file_name = input; }) );
    mangle_name(in_file_name);

    out_file_name.append(".json");
    cout << "Implicit out-file name: " << out_file_name << endl;
  }

  /* see if user provided a name for the outfile.  */
  if(!out_file_name.empty()) {
    /* set the infile pointer and the cleanup object */
    output_cleanup.reset(new ofstream( out_file_name.c_str()) );
    output = output_cleanup.get();

    if(!output_cleanup->is_open()) {
      cout << "Couldn't open " << out_file_name << endl;
      throw XmlException("bad output file.");
    } else {
      cout << "Opened " << out_file_name << " for output " << endl;
    }
  }
  /* set the default output stream. */
  else {
    output = &cout;
  }
}

#endif
