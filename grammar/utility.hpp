#ifndef UTILITY_HH
#define UTILITY_HH
/**
 * @file utility.hpp
 * @author Ryan Domigan 
 * ryan_domigan@students.uml.edu 
 *  Created on Monday 01 2012  
 *
 * basic utility functions  that don't belong anywhere else.
 */

#include <fstream>
#include <string>
/**
 * the famous trim function.  Removes white space from the beginning and end of input.
 * 
 * @param input the input strind which needs a trim.
 * @return the string with leading and trailing white-space removed.
 */
std::string trim(const std::string& input) {
  using namespace std;
  size_t begin = input.find_first_not_of(" \t\n");
  size_t end = input.find_last_not_of(" \t\n");
  
  /* if begin is invalid, there is no non-whitespace.  Return an empty string.  */
  if(begin == string::npos)
    return string();
    
  return input.substr(begin, end - begin + 1);
}

/**
 * uses a specified stream to open a file based on arguments provided by command line style array (count + char array)
 * 
 * @param file_stream the file stream which holds result specified by user arg
 * @param argc count for param args
 * @param argv vector for param args
 */
void get_file_stream_from_user(std::ifstream &file_stream, int argc, char *argv[]) {
  using namespace std;
  string file_name;

  //see if I have an argument, o
  if(argc > 1) {
    file_name = argv[1];
    file_stream.open(file_name.c_str());
  }

  while(!file_stream.is_open()) {
    cout<<"Please enter a file to open: "<<endl;
    cin>>file_name;
    file_stream.open(file_name.c_str());
  }
}

/**
 * a generic quick-sort function
 * 
 * @param vecInput the input to be sorted.  Passed by ref, order of input is not preserved.
 * @param cmp comparitor used to sort.
 for a < b: cmp(a,b) < 0
 , a > b: cmp(a,b) > 0
 , a = b: cmp(a,b) = 0
 * @return the sorted input
 */
template<class Vector, class Compare>
Vector& quicksort(Vector &vecInput, Compare cmp) {
  typedef typename Vector::iterator iterator;
  typedef typename Vector::value_type value_type;
  struct local {
    void swap_values(iterator a, iterator b) {
      value_type tmp = *a;
      *a = *b;
      *b = tmp;
    }
    
    // if the value of a > b, cmp(a,b) should return an integer >0.  I'm making a little test function to make
    // this code slightly more readable
    bool gt(int value) {
      return value > 0;
    }

    // same idea as above, a slightly more readable test
    bool lt(int value) {
      return value < 0;
    }
    
    // wikipedia sugests picking the median value as the pivot
    iterator choose_pivot(iterator left, iterator right, Compare cmp) {
      size_t span = right - left;
      iterator middle = left + (span / 2);
      
      if( gt(cmp(*left, *right))
	  && gt(cmp(*left, *middle)) ) {
	if( gt(cmp(*middle, *right) )) 
	  return middle;
	else 
	  return right;
      } else if( gt(cmp(*right, *left))
		 && gt(cmp(*right, *middle)) ) {
	if( gt( cmp(*middle, *left)) )
	  return middle;
	else 
	  return left;
      } else { 			// middle must be greater than or equal to left and rigth
	if( gt(cmp(*left,*right) ) )
	  return left;
	else 
	  return right;
      }
    }
    
    /* pseudo code from wikipedia:
    // left is the index of the leftmost element of the array
    // right is the index of the rightmost element of the array (inclusive)
    //   number of elements in subarray = right-left+1
    function partition(array, 'left', 'right', 'pivotIndex')
    'pivotValue' := array['pivotIndex']
    swap array['pivotIndex'] and array['right']  // Move pivot to end
    'storeIndex' := 'left'
    for 'i' from 'left' to 'right' - 1  // left ≤ i < right
    if array['i'] < 'pivotValue'
    swap array['i'] and array['storeIndex']
    'storeIndex' := 'storeIndex' + 1
    swap array['storeIndex'] and array['right']  // Move pivot to its final place
    return 'storeIndex'
    */
    iterator partition(iterator left, iterator right, iterator pivot_itr, Compare cmp) {
      value_type store_value = *pivot_itr;
      iterator store_index = left;
      // move the pivot value out of the range I'm sorting
      swap_values(pivot_itr, right);
      
      for(iterator itr = left; itr < right; ++itr) {
	if( lt(cmp(*itr, store_value)) ) {
	  swap_values(itr, store_index);
	  ++store_index;
	}
      }
      // right contains the old pivot value, store_index should now be pointing to the midpoint
      swap_values(store_index, right);
      return store_index;
    }

    /* implement quicksort based on the wikipedia definition
       function quicksort(array, 'left', 'right')
     
       // If the list has 2 or more items
       if 'left' < 'right'
    
       // See "Choice of pivot" section below for possible choices
       choose any 'pivotIndex' such that 'left' ≤ 'pivotIndex' ≤ 'right'
    
       // Get lists of bigger and smaller items and final position of pivot
       'pivotNewIndex' := partition(array, 'left', 'right', 'pivotIndex')
    
       // Recursively sort elements smaller than the pivot
       quicksort(array, 'left', 'pivotNewIndex' - 1)
    
       // Recursively sort elements at least as big as the pivot
       quicksort(array, 'pivotNewIndex' + 1, 'right')
    */
    void quicksort(iterator left, iterator right, Compare cmp) {
      if(left < right) {
	iterator pivot_index = choose_pivot(left, right, cmp);
	pivot_index = partition(left, right, pivot_index, cmp);
	quicksort(left, pivot_index - 1
		  , cmp);

	quicksort(pivot_index + 1, right
		  , cmp);
      }
    }
  };

  local l;
  /* check for empty vector */
  if(vecInput.empty()) return vecInput;
  
  /* otherwise sort it */
  l.quicksort(vecInput.begin(), vecInput.end() - 1, cmp);
  return vecInput;
}

/**
 *  foreach_line: applies functor of LineHandler to each line in input.
 * @param input string source
 * @param handle_line string sink
 */
template<class LineHandler>
inline void foreach_line(std::istream &input, LineHandler&& handle_line) {
  using namespace std;
  std::string line;
  // stl implements an implicit bool cast for istreams, if there are more characters in the stream
  // it will cast to (bool)true, if there are not it will cast to (bool)false
  while(input) {
    std::getline(input, line);
    handle_line(line);
  }
}

/**
 * object to take in lines of string, and calls two objects on the same arg
 */
template<class T0, class T1>
class call_both {
  T0 &obj0;			/* first functor */
  T1 &obj1;			/* second functor */
public:
  /**
   * assign values to both functors
   */
  call_both(T0 &a, T1 &b) {
    obj0 = a;
    obj1 = b;
  }

  /**
   * apply both functors to the input
   */
  void operator()(std::string& input) {
    obj0(input);
    obj1(input);
  }
};

/**
 * generate a function object which takes one param and calls two functors on it
 *
 * @param a first functor to apply
 * @param b second functor to apply
 */
template<class T0, class T1>
call_both<T0,T1> make_call_both(T0 &a, T1 &b) {
  return call_both<T0,T1>(a,b);
}

#endif
