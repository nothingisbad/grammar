#ifndef GENERICS_HH
#define GENERICS_HH
/**
 * @file generics.hpp
 * @author Ryan Domigan < ryan_domigan@students.uml.edu >
 * 
 * Generic functions.
 * todo: merge with utility.hpp
 */

/**
 * Sort a randomly accesable container using quicksort algorithm.
 * @param vecInput vector reference to be sorted
 * @param cmp element comparison functor
 * @return reference to vecInput
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
  l.quicksort(vecInput.begin(), vecInput.end() - 1, cmp);
  return vecInput;
}

/**
 * feeds a functor strings from a stream, one line at a time.
 * @param input string source
 * @param handle_line string sink
 */
template<class LineHandler>
inline void foreach_line(std::istream &input, LineHandler &handle_line) {
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
   * apply both functors to the input.
   * @param input input to call the functors on.
   */
  void operator()(std::string& input) {
    obj0(input);
    obj1(input);
  }
};

/**
 * A call_both factory function. 
 * @param a first functor to apply
 * @param b second functor to apply
 * @return creates a pseudo closure which calls two objects on the same
 *  input when invoked.
 */
template<class T0, class T1>
call_both<T0,T1> make_call_both(T0 &a, T1 &b) {
  return call_both<T0,T1>(a,b);
}


#endif

