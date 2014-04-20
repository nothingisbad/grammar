A somewhat convenient and not too inefficient way to make a recursive decent parser in C++11

The approach is to implement a little DSL which ties together regular expressions with user-defined callbacks.
The DSL is constructed with the DefineGrammar class, and the basic building blocks are regular expressions, callbacks, branches, labels, and gotos.

The DefineGrammar can then be passed into a Parser, which can in turn be used to process strings and streams.

If a DefineGrammar is meant to be temporary; if a DefineGrammar object is passed into a Parser or another DefineGrammar it's internal state is transferred.

If you need a quick and dirty parser with relatively few external dependencies (C++11 and boost::regex (I'll switch to std::regex if g++ ever gets decent support)), this might be handy.
However, resources are not correctly freed by delete (the generated grammars may contain cycles, rather than safely freeing them they simply leak) and performance is probably not great as the parsing process it to try a regular expression match then use virtual function dispatch and a trampoline to get the next rule.

# grammar::DefineGrammar :

When a regular expression is matched, control is passed to the following expression i.e.

```C++
#include <iostream>
#include "./grammar/grammar.hpp"

int main() {
  using namespace grammar;
  DefineGrammar example = re(".*").thunk([&]() { std::cout << "Hello world!" << std::endl; });
  Parser parse;
  parse.sink(example);
  parse("foo");

  example = re("bar").thunk([&]() { std::cout << "Hello world!" << std::endl; });
  parse.sink(example);
  parse("foo");

  return 0;
}
```
=> Hello world!

The expression following the re(..) can take the matched string as an argument:
```C++
int main() {
  using namespace grammar;
  DefineGrammar example = re(".*").thunk([&]() { std::cout << "Hello world!" << std::endl; });
  Parser parse;
  parse.sink(example);
  parse("foo");

  return 0;
}
```
=> Captured foo!

Branches can be used to tie re(..) expressions together.
In a branch, the matches are tried in order and only the first match is executed.

```C++
int main() {
  using namespace grammar;
  using namespace std;
  DefineGrammar example = label("loop").branch( re(".*foo").thunk([&]() { cout << "Foo!" << endl; })
                                                , re(".*bar").thunk([&]() { cout << "Bar!" << endl; }) );
  Parser parse;
  parse.sink(example);
  parse("bar");

  return 0;
}
```
=> Bar!

A point in the parsing can be labeled with the label(<label-string>) command, and a call to go(<label-string>) will bring execution to that point with the currently built input string.
An exception is raised at runtime when a DefineGrammar with unresolved go(...) destinations is passed into a Parser, which makes debugging a little easier.

For more details, see grammar/DefineGrammar.hpp and grammar/NamelessGrammar.hpp

# Example
This library includes a utility that converts from xml formatted files to json.
I'll include the parser generation here as an example; for the complete code (which has to manage the AST for the XML) see see xml2json.cpp and XmlSemanticAction.hpp.

```C++
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

	       , otherwise().error("Don't know how to handle tag.")
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
``` 