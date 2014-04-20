#ifndef GRAMMARTREE_HH
#define GRAMMARTREE_HH
/**
 * @file grammar/GrammarTree.hh
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Oct 09, 2012
 */

#include <map>
#include <vector>
#include <string>
#include <memory>

#include "./GrammarChain.hpp"
#include "./Label.hpp"
#include "./GotoLabel.hpp"
#include "./Pattern.hpp"
#include "./Reduce.hpp"
#include "./Until.hpp"


namespace grammar {
  /**
   * basic interface to a branching grammar tree.  This class wraps a GrammarChain and adds facilities for
   * branching (using the Branch type).  It is further wraped by DefineGrammar which adds a variety of convinience functions
   * and provisions for stack-based managment of heap allocated trees (trees defined on stack may become the branches of other
   * trees in a transparent way)
   *
   * @see DefineGrammar
   * @see GrammarChain
   */ 
  class GrammarTree {
  public:
    GrammarChain grammar;	/**< toplevel grammar defined in the tree */

  private:
    Branch *active_branch_;	/**< active_branch data */

    typedef std::map<std::string, Label*> SymbolTable;
    SymbolTable environment_;           /**< explicit labels which I have definitions for. */
    
    typedef std::vector<GotoLabel*> PatchList;
    typedef std::map<std::string, PatchList > UnresolvedSymbolTable;
    UnresolvedSymbolTable _unresolved; 	/**< labels which need, but haven't seen, definitions. */


    //! retrieve the tree's current active branch
    Branch *active_branch() { return active_branch_; }

    /**
     * adds a goto to the stack of goto's assosiated with name.  When a Label with name is defined, update all
     * the goto's in the stack to point at it.
     * 
     * @param name name of Label I'm waiting for.
     * @param unresolved GotoLabel which needs the Label I'm waiting for.
     */
    void push_unresolved(const std::string& name, GotoLabel *pending) {
      _unresolved[name].push_back(pending);
    }
    
    void resolve(UnresolvedSymbolTable &reslv, SymbolTable &env) {
      SymbolTable::iterator found; 		/* hold the result of searches */
      UnresolvedSymbolTable::iterator temp;	/* used to un-chain intermediate */

      if(reslv.empty()) return; 	/* nothing to be done. */
    
      /* iterate through the reslv table to find pending resolutions */
      UnresolvedSymbolTable::iterator resolving = reslv.begin();
      while( resolving != reslv.end()) {
	/* Is three now a defintion for the unresolved symbol?  */
	if( (found = env.find( resolving->first ))
	    != env.end() )
	  {
	    /* if there is, set the destination for every element in the unresolved list */
	    for(auto patching : resolving->second)
	      patching->set_label( found->second );
	
	    temp = resolving;
	    ++resolving;
	    /* as the symbol is now resolved.  I can erase it. */
	    reslv.erase(temp);
	  }
      
	/* if I didn't resolve the symbol, I still need to move onto the next. */
	else ++resolving;
      }}
  public:
     /**
     * constructs an empty tree with default values
     * @see GrammarChain#GrammarChain
     */
    GrammarTree() = default;

    /**
     * destructs grammar and frees any data structures that haven't been passed out with release
     */
    ~GrammarTree() = default;
  
    /**
     * Adds a reuction rule to the back of the current tree
     * 
     * @param s reduction rule to add
     * @return the current GrammarTree
     */
    void reduce(Reduce::ActionType s) { grammar.push_back<Reduce>()->set_action( s ); }

    //! add a rule to scan for a particular pattern
    void scan(Pattern *m) {
      Until *til = grammar.push_back<Until>();
      til->set_pattern(m);
    }

    /**
     * returns the data of the current tree.
     * The caller of this member is responsible for memory managment of returned Rules.
     * todo: unique_pntr?
     * 
     * @return root rule of the current grammar
     * @pre a grammar expects to have all symbols resolved _before_ it is 
     * @post this grammar is empty and will not free the Rules it contained prior to releasegrammar
     */
    Rule *release_grammar() {
      Rule *result = grammar.release();
      return result;
    }

    /**
     * push a branch onto the internal grammar.
     *
     * @post active_branch_ points to a newly allocated branch and the sequence now contains
     *   old_top->New_Branch->post-branch
     */
    void push_branch() {
      active_branch_ = grammar.push_back<Branch>();
      grammar.push_back<Label>(Label("post-branch"));
    }
    
    void push_case(Rule *r) {
      Until *unt = nullptr;
      Branch *brn = nullptr;
      Otherwise *other = nullptr;

      /* if r is a branch, adopt all its cases */
      if( (brn = dynamic_cast<Branch*>(r)) )
	copy( brn->get_case_vector().begin(), brn->get_case_vector().end()
	      , back_inserter( active_branch()->get_case_vector() ) );
    
      /* if r is an Until, use that as a case */
      else if( (unt = dynamic_cast<Until*>(r))  ) {
	active_branch()->add_branch(unt->get_pattern()
				    , unt->get_default() );
      }

      else if( (other = dynamic_cast<Otherwise*>(r) ) ) {
	active_branch()->add_default( r->get_default() );
      }

      /* if it's not an Until or a Branch, I don't know how to add it on as a case. */
      else throw std::runtime_error("Case being pushed to a grammar branch must begin with a scanner");
    
      if(unt != nullptr) delete unt;    /* I've got the patterns and rule from my scanner, it should be safe to free it. */
      if(brn != nullptr) delete brn;
      if(other != nullptr) delete other;
    }

    /**
     * (destructively) sinks a case to the most recent branch assosiated with this tree.  The input is a tree which must have a scan
     * or branch action as its first user defined node (ie, before doing anything else the user must have called some
     * form of push_branch() or re() on the argument tree)
     * 
     * @param tree the grammar of the case I'm adding.  First item after the root_ label _must_ be a re type.
     * @post the branch has been added, and it will continue to the active_branch_ "post-branch" label
     */
    void push_case(GrammarTree *tree) {
      using namespace std;
      push_case( tree->grammar.front() );
    
      /* send the tail of input case to the Rule following branch (after the branch evaluates, go to the following block rather than looping)
	 Enclosed branches may rely on this label, so do not get rid of it at this point. */
      tree->grammar.back()->set_default( active_branch_->get_default() );
    
      /* resolve whatever links possible in tree and this */
      merge_tables(tree);
    
      /* keep the tree from deleting my newly aquired branch */
      tree->release_grammar();
    }

    /**
     * Merge the symbols tables of tree with this*, resolving label links in both.
     * 
     * @param tree
     */
    void merge_tables(GrammarTree *tree) {
      UnresolvedSymbolTable::iterator merging;
	
      /* resolve any gotos with the enclosed labels */
      resolve(_unresolved, tree->environment_);
      /* now resolve any of the new branches goto's at the current scope */
      resolve(tree->_unresolved, environment_);

      /* finish by merging symbol tables */
      environment_.insert(tree->environment_.begin(), tree->environment_.end());
    
      /* the unresolved table is more complex to merge because I have to grow the patch list as I go. */
      for(auto itr = tree->_unresolved.begin(); itr != tree->_unresolved.end(); ++itr ) {
	  merging = _unresolved.find(itr->first);
	
	  /* if unresolved already has this key, I need to merge the patch-lists  */
	  if( merging != _unresolved.end() ) {
	    copy(itr->second.begin(), itr->second.end(),
		 back_inserter( merging->second ));
	  }

	  /* if the key doesn't exist, insert it. */
	  else (_unresolved[itr->first]).swap(itr->second);
	}
    }

    /**
     * the tree's typename:name string
     * 
     * @return string representing tree
     */
    std::string str() { return std::string("<tree>"); }

    /**
     * beginning of the chain which this grammar tree encapsulates.
     * 
     * @return the first rule of the chain
     */
    Rule* begin() { return grammar.front(); }

    /**
     * takes tree's internal data using release()
     * 
     * @param tree the tree to append
     */
    void append(GrammarTree *tree) {
      merge_tables(tree);
      grammar.append(&tree->grammar);
    }

    /**
     * inserts a goto into the rule sequence.
     * 
     * @param label name of the label to which I should jump
     */
    void go(const std::string& label) {
      SymbolTable::iterator resolved = environment_.find(label);

      /* if the symbol already exists, make a new goto with that as the destination */
      if( resolved != environment_.end() )
	grammar.push_back( GotoLabel(resolved->second) );

      /* if the symbol doesn't exist, still make the goto, but push it to unresolved and for
	 back-patching */
      else
	push_unresolved( label, grammar.push_back<GotoLabel>() );
    }


    /**
     * Creates a label for the current point in the grammar.  Labels have more or less global scope.  There is currently no
     * logic to deal with more than one label in the same grammar with the same name.
     * 
     * @param name the label name for the current point in the tree
     */
    void label(const std::string& name) {
      UnresolvedSymbolTable::iterator pending = _unresolved.find(name);
      Label *lab = grammar.push_back( Label(name) );

      /* check if the name has pending gotos */
      if( pending != _unresolved.end() ) {
      
	/* if it does, re-point all of them to the new label */
	for_each(pending->second.begin(), pending->second.end()
		 , [=](decltype(*pending->second.begin()) go) { go->set_label(lab); });

	/* remove the link from the unresolved table: */
	_unresolved.erase(pending);
      }
      /* stick the label into my symbol table for future gotos */
      environment_[name] = lab;
    }

    /**
     * Invoke the recursive printer on every in-tree element and (breifly) print out
     * all unresolved symbols (Goto's pending a destination).
     * 
     * @param out stream to print to
     */
    void print(std::ostream &out) {
      using namespace std;
    
      /* checks for cycles and conditionally prints each item in my tree. */
      PrintRecursiveRule do_print(out);
      do_print.print( begin() );
    
      if(!_unresolved.empty()) {
	out << "\nWith unresolved items: ";
	/* print the names of each unresolved item */
	for(UnresolvedSymbolTable::iterator itr = _unresolved.begin(); itr != _unresolved.end(); ++itr)
	  out << itr->first << " ";
      } else {
	out << "\nAll links resolved.";
      }
      out << endl;
    }

    /**
     * Recursively prints internals to std::cout.
     * 
     * @see GrammarTree#print
     */
    void print() { print(std::cout); }

    /**
     * query the unresolved table.  If there are unresolved labels (go statements with no valid label's), return false.
     * @return false if there are unresolved labels.
     */
    bool fully_resolvedP() { return _unresolved.empty(); }

    /**
     * apply a function to each element of the _unresolved table.
     * @param fn the function to apply
     */
    void for_unresolved(std::function<void (const std::string&)> fn) {
      for(auto item : _unresolved)  fn(item.first);
    }

    /**
     * Doesn't do anything yet, but I'm thinking of ways to build a parser which will correctly free all its rules.
     * 
     * @param tree
     */
    void append_free_list(GrammarTree *tree) { }
  };
}

#endif

