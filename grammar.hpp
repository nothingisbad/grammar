#ifndef GRAMMAR_ALL_HH
#define GRAMMAR_ALL_HH
/**
 * @namespace grammar Defines objects and functions useful in generating a parser.
 * Modeled loosely on LEX/YACC, grammar defines some functors which can match strings, some and some functors that
 * callback or branch when a match is found
 * 
 * @file grammar/grammar.hh
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Oct 04, 2012
 * includes all the files defined by grammar
 *
 */

#include "DefineGrammar.hpp"
#include "Pattern.hpp"
#include "PutBack.hpp"
#include "Branch.hpp"
#include "Until.hpp"
#include "Parser.hpp"
#include "Rule.hpp"
#include "NamelessGrammar.hpp"
#include "Reduce.hpp"
#include "SyntaxError.hpp"
#include "Singleton.hpp"
#endif
