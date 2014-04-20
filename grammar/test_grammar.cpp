/**
 * @file /home/ryan/uml/computing-IV/a07/test_grammar.cc
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Nov 29, 2012
 *
 * Some simple test routines for the grammar (now using regexs)
 */

#include <iostream>  // for cout and friends
#include <sstream>   // for string streams
#include <string>    // for the STL string class

#define DEBUG_GRAMMAR_BRANCH
#include "./grammar.hpp"

using namespace std ;     // to eliminate the need for std::

// standard C++ main function
int main( int argc, char* argv[] ) {
  using namespace grammar;
  using namespace std;

  /* prints out some information pertaining to Branch */
  //RunVerbose<Branch>::run_verbose();
  
  // set up the strings to be tested
  vector<string> str ;
  str.push_back( "quit" );
  str.push_back( "exit" );
  str.push_back( "Quit" );
  str.push_back( "quite" );
  str.push_back( "unrequited" );
  str.push_back( "unreQUITed" );

  auto match = [](const std::string& str) {
    cout << str << " is a match " << endl;
  };

  auto not_match = [](const string & str) {
    cout << str << " is _not_ a match " << endl;
  };

  cout << "Match test: " << boost::regex_search("quitting", boost::regex(".*")) << endl;

  cout << "\n\n Case sensitive: \n\n";

  { // case sensitive
    Parser parse;
    DefineGrammar rule;
    rule.label("begin").branch( re("(.*quit.*)").on_string( match )
  				, re("$").on_string(not_match)
  				).go("begin");

    rule.print(  );
    parse.sink( move(rule));
    
    for_each(str.begin(), str.end()
  	     , [&](const string &str) {
  	       cout << "In " << str << endl;
  	       parse(str);
  	     });
  }

    cout << "\n\nOK, now case insensitive:\n" << endl;
  //equivalent to test1_BasicAndCaseInsensitiveMatches
  { // case sensitive
    Parser parse;
    DefineGrammar rule;
    rule.label("begin").branch( re_i("(.*quit.*)").on_string(match)
    				, re("$").on_string(not_match) )
      .go("begin");

    parse.sink( move(rule) );
    
    
    for_each(str.begin(), str.end()
  	     , [&](const string &str) {
  	       cout << "In " << str << "\n ";
  	       parse(str);
  	     });
  }


  cout << "\n\ntry some captures\n\n" << endl;
  vector<string> svec;
  svec.push_back("add element root student a");
  cout << "**Taking match as a string:" << endl;
  {
    Parser parse;
    parse.sink(re("add(\\s*)(\\w*)").on_string( [&](const string &str) {
	  cout << "got matching part: " << str << endl;
	}
	, 2));

    for_each(svec.begin(), svec.end() 
	     , [&](const string& s) { parse(s);});
  }

  cout << "**Taking match as an smatch: " << endl;
  {
  
    Parser parse;
    parse.sink(re("add(\\s*)(\\w*)").on_match( [&](Match &what) {
	  int count = 0;
	  for_each(what.begin(), what.end()
		   , [&](decltype(*what.begin())& s) {
		     cout << "Match [" << count << "]: |" << s << "|" << endl;
		     ++count;
		   });
	}
	));

    for_each(svec.begin(), svec.end() 
	     , [&](const string& s) { parse(s);});
  }

  return 0 ;
}
