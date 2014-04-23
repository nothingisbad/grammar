#ifndef GRAMMAR_GRAMMARCHAIN_HH
#define GRAMMAR_GRAMMARCHAIN_HH
/**
 * @file grammar/GrammarChain.hh
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Oct 09, 2012
 */

#include <iostream>
#include "./Rule.hpp"

namespace grammar {
  /**
   * a stack like object which uses the Rule* Rule::(get_|set_)default() members to create sequential rule chains.
   * this class is meant to abstract the creation of non-branching sequences of rules, it is used internally by GrammarTree
   * to create more complex structures and is probably less useful to other callers.
   *
   * todo: I could make most of the members private so only GrammarTree (a friend class) can monkey with this.
   *
   * @see GrammarTree
   */
  class GrammarChain {
    friend class GrammarTree;
    Rule *_head 		/**< head of the list (what gets returned) */
      , *_tail;			/**< tail of the list (what gets appended to) */

    /* , *_terminator;		/\**< ends of the list (tail_'s->get_default(), NULL by default)*\/ */
    /**
     * push provided rule to the chain.  This version is not meant for public consumption.
     * 
     * @param rule to append to chain
     * @return input rule that was just pushed
     */
    Rule* push_back(Rule *r) {
      using namespace std;
    
      if(_tail) {
	_tail->set_default(r);
	_tail = r;
      }
      /* first node, it's my head and tail */
      else {
	_head = _tail = r;
      }
  
      r->set_default( NULL );
      return r;
    }
  public:
    GrammarChain() : _head(nullptr), _tail(nullptr) {}

    /**
     * delete all data represented by this structure.
     * note: doesn't delete terminator as that might be a many-to-one reference.. I should define the relationship better.
     * todo: this is a mess.  I have branching, possibly circular grammars, I need to track every element added to a set
     * and merge sets when I append or add branches to a tree.
     */
    ~GrammarChain() {
      using namespace std;
      /* Rule *tmp; */
      /* /\* could really use a garbage collector here. *\/ */
      /* set<Rule*> deferred; 	/\* this may cause destruction to throw, (dynamically sized), which would be all kinds of a mess. *\/ */
      /* while(_head != NULL) { */
      /*   tmp = _head; */
      /*   _head = _head->get_default(); */
      /*   /\* labels may have a many to one relationship, therefore defer the free-ing *\/ */
      /*   if(dynamic_cast<Label*>(tmp)) { */
      /* 	deferred.insert(tmp); */
      /*   } */
      /*   /\* otherwise, I should be OK to go. *\/ */
      /*   else { */
      /* 	delete tmp; */
      /*   } */
      /* } */

      /* /\* now clean up the deferred objects. *\/ */
      /* for_each(deferred.begin(),deferred.end() */
      /* 	     , [](Rule *r) { delete r; }); */
      _head = _tail = NULL;
    }

    /**
     * element at front (evaluation order) of queue
     * @return front element
     */
    Rule* front() { return _head; }

    /**
     * element at the back (last in evaluation order, most recently added)
     * @return back element
     */
    Rule* back() { return _tail; }


    /**
     * hands out the first evaluation order rule of the existing list, then clears the interenal representation
     * 
     * @return head of old list
     */
    Rule* release() {
      Rule *tmp = _head;
      _head = _tail = NULL;
      return tmp;
    }

    /**
     * printer (debugging version, flat iteration)
     */
    void print() {
      using namespace std;
      Rule *itr = front();
      while(itr != NULL ) {
	cout << itr->str() << " " << flush;
	itr = itr->get_default();
      }
    }

    /**
     * takes the data from chain using release and appends it to the current _tail
     * 
     * current terminator of this object
     * 
     * @param chain list to append
     */
    void append(GrammarChain* chain) {
      if( chain->empty()  )  return;
    
      if(empty()) {
	_head = chain->_head;
	_tail = chain->_tail;
      }

      else {
	_tail->set_default( chain->_head );
	_tail = chain->_tail;
      }
      _tail->set_default( NULL );

      /* release the stuff I just appended so it doesn't get deleted on me. */
      chain->release();
    }

    /**
     * tests whether or not there are nodes in this chain.
     * 
     * @return true if empty, else false
     */
    bool empty() { return _head == NULL; }


    /* templated members must be defined at point of call */
    template<class T>
    T* push_back() {
      T *result = new T();
      push_back( dynamic_cast<Rule*>(result) );
      return result;
    }

    /**
     * push a new element to stack, using a argument to initialize via copy
     * construction.
     * 
     * @tparam T type of objcet to push
     * @param orig instance to use for copy constructin 
     * @return the object that was just pushed
     */
    template<class T>
    T* push_back(const T& orig) {
      T* result = new T(orig);
      push_back(dynamic_cast<Rule*>(result));
      return result;
    }
  };
}

#endif
