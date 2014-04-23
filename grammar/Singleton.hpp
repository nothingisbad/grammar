#ifndef GRAMMAR_SINGLETON_HH
#define GRAMMAR_SINGLETON_HH
/**
 * @file grammar/Singleton.hh
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Oct 13, 2012
 * 
 * Implements some classes using the singleton pattern.
 */
#include <string>

namespace grammar {
  /**
   * Templatized implementation of the Singleton model.
   * The code is modeled closely on http://en.wikibooks.org/wiki/C%2B%2B_Programming/Code/Design_Patterns#Singleton
   * 
   * @tparam R data type of Singleton.  R must be assignable and default constructable.
   * @tparam Key you can have multiple Singleton of the same data type, but one for each key/data-type pair
   */
  template<class R, class Key>
  class Singleton {
    R value_;
    Singleton() {
      /* rely on default constructor of value_ */
    }

    Singleton(const Singleton&); /* forbidden */
    const Singleton& operator=(const Singleton&); /* also forbidden */
    ~Singleton() {}				  /* prevent free-ing */
  public:
    /** fetch instance, in Singleton pattern */
    static Singleton& instance() {
      //supposedly, the static keywords will make this run once
      static Singleton *my_instance = new Singleton();
      return *my_instance;
    }

    /**
     * getter
     */
    static R& get() {
      return instance().value_;
    }

    /**
     * setter 
     */
    static void set(const R& value) {
      instance().value_ = value;
    }
  };

  /**
   * Templatized model of the singleton pattern.  Allows me to configure
   * the parser for verbose evaluation at run time.
   *
   * class is meant to be used in conjunction with ReduceAction, therefor its static members take
   * string references, which they ignore.
   *
   * @tparam T Key type assosiated with this Singleton (one instance of the Singleton per Key)
   * @see Singleton for a more general templatized Singleton type.
   */
  template<class T>
  class RunVerbose {
    bool verbose_;
    RunVerbose() {
      verbose_ = false;
    }

    RunVerbose(const RunVerbose<T>&) = delete; /* forbidden */
    const RunVerbose& operator=(const RunVerbose&) = delete; /* also forbidden */
    ~RunVerbose() {}				  /* prevent freeing */
  public:
    /** fetch instance, in Singleton pattern */
    static RunVerbose<T>& instance() {
      //supposedly, the static keywords will make this run once
      static RunVerbose<T> *my_instance = new RunVerbose();
      return *my_instance;
    }

    /**
     * I want compatibility with ReduceAction wrappers, string is ignored
     * @param input Ignored.  Needed to match up with ReduceWithFunction signature
     */
    static void run_verbose() {
      instance().verbose_ = true;
    }

    /**
     * I want compatibility with ReduceAction wrappers, string is ignored
     * @param input Ignored.  Needed to match up with ReduceWithFunction signature
     */
    static void run_quiet(std::string& input) {  instance().verbose_ = false;  }

    /**
     * predicate form, as in RunVerbose<foo>::P()
     * 
     * @return true if I want to run T verbosely
     */
    static bool P() {
      return instance().verbose_;
    }
  };
}
#endif
