#ifndef XML_DEBUG_HH
#define XML_DEBUG_HH
/**
 * @file debug.hpp
 * @author Ryan Domigan <ryan_domigan@sutdents@uml.edu>
 * Created on Oct 13, 2012
 */

/**
 * non-instantialble template argument, for use with RunVerbose when I want to print my produced grammar for
 * main.
 */
struct debug_xml_grammar;

/**
 * non-instantialble template argument.  My parser will usually sort the children of each XmlElement before printing them,
 */
struct xml_element_sort_order;

/**
 * non-instantialble template argument.  Direct print-out of parsed xml tree using XmlPrint
 */
struct debug_xml_tree;

#endif
