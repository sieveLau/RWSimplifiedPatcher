#pragma once

#ifdef WIN32
#pragma warning(disable : 4996)

#include <Windows.h>

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <algorithm>
#include <codecvt>
#include <exception>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <locale>
#include <memory>
#include <plog/Log.h>
#include <regex>

std::wstring xmlCharToWideString(const xmlChar *xmlString);

std::wstring s2ws(const std::string &str);

std::string ws2s(const std::wstring &wstr);

std::wstring getXPath(xmlNodePtr node);

std::wstring getText(xmlDocPtr doc, xmlNodePtr node);

std::unique_ptr<xmlXPathObject, void (*)(xmlXPathObjectPtr)> getByXPath(xmlDocPtr doc,
                                                                        const std::wstring &xpath);

std::vector<xmlNodePtr> getNodeSet(const xmlXPathObject *xpath_result);

std::wstring getDefNameFromXPath(xmlDocPtr doc, const std::wstring &node_xpath);

std::wstring getliParentTagName(const std::wstring &xpath_containing_li);

bool getliNumber(const std::wstring &xpath_containing_li, long *result);

/**
 * generate corresponding output directory for this xpath
 * @param xpath xpath starts with "/Defs/"
 * @return corresponding output directory
 */
std::wstring getOutputDirectory(const std::wstring &xpath);

template<typename T> bool str_contains(const std::wstring &str, const T &what_to_find) {
    return (str.find(what_to_find) != std::wstring::npos);
}