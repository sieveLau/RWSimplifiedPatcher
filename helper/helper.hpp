#pragma once

#ifdef WIN32
#pragma warning(disable : 4996)
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <algorithm>
#include <exception>
#include <filesystem>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <memory>
#include <plog/Log.h>
#include <regex>

std::vector<std::filesystem::path> file_walker(const std::string &dir,
                                               const std::string &extension = ".xml");

std::string getXPath(xmlNodePtr node);

std::string getText(xmlDocPtr doc, xmlNodePtr node);

std::unique_ptr<xmlXPathObject, void (*)(xmlXPathObjectPtr)> getByXPath(xmlDocPtr doc,
                                                                        const std::string &xpath);

std::vector<xmlNodePtr> getNodeSet(const xmlXPathObject *xpath_result);

std::string getDefNameFromXPath(xmlDocPtr doc, const std::string &node_xpath);

std::string get_li_parent_tag_name(const std::string &xpath_containing_li);

bool get_li_number(const std::string &xpath_containing_li, long *result);

/**
 * generate corresponding output directory for this xpath
 * @param xpath xpath starts with "/Defs/"
 * @return corresponding output directory
 */
std::string getOutputDirectory(const std::string &xpath);

template<typename T> bool str_contains(const std::string &str, const T &what_to_find) {
    return (str.find(what_to_find) != std::string::npos);
}

struct empty_node_text : public std::runtime_error {
    empty_node_text() : runtime_error("Empty Node Text.") {}
};