	include compiler.mk

top=../

CXX_COMPILE=$(CXX) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)

all: test_grammar

test_grammar: *.hpp *.cpp
	$(CXX) -o test_grammar *.cpp

tags: 


clean:
	rm -f test_grammar

.PHONY: dist

dist:
	mkdir $(DISTDIR)
	cp $(grammar_SOURCES) $(test_grammar_SOURCES) $(DISTDIR)

# End of Makefile
