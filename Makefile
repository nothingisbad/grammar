include grammar/compiler.mk

all: main

%.o: %.cpp %.hpp
	$(CXX) $(CPPFLAGS) $< -c -o $@

grammar/libgrammar.a: grammar/*.cpp grammar/*.hpp
	cd grammar; $(MAKE) grammar


xml2json: *.cpp *.hpp grammar/*.hpp
	 $(CXX) $(CPPFLAGS) -o xml2json xml2json.cpp

test: test.cpp
	$(CXX) $(CPPFLAGS) -o test test.cpp

clean:
	rm -f *.o
	rm -f main
	cd grammar; $(MAKE) clean
