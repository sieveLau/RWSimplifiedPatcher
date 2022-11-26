#pragma once
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <plog/Log.h>
#include <memory>
#include <exception>

std::string getXPath(xmlNodePtr node);

std::string getText(xmlDocPtr doc, xmlNodePtr node);

std::unique_ptr<xmlXPathObject, void (*)(xmlXPathObjectPtr)> getByXPath(xmlDocPtr doc,
                                                                        const std::string &xpath);

std::vector<xmlNodePtr> getNodeSet(const xmlXPathObject* xpath_result);

std::string getDefNameFromXPath(xmlDocPtr doc, const std::string& node_xpath);

std::string getliParentTagName(const std::string& xpath_containing_li);
bool getliNumber(const std::string& xpath_containing_li, long *result);

/**
 * generate corresponding output directory for this xpath
 * @param xpath xpath starts with "/Defs/"
 * @return corresponding output directory
 */
std::string getOutputDirectory(const std::string& xpath);

template<typename T>
bool str_contains(const std::string& str, const T& what_to_find){
    return (str.find(what_to_find) != std::string::npos);
}