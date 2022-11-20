#ifndef RWSIMPLIFIEDPATCHER_HELPER_H
#define RWSIMPLIFIEDPATCHER_HELPER_H
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <plog/Log.h>
#include <memory>

std::string getXPath(xmlNodePtr node);

std::string getText(xmlDocPtr doc, xmlNodePtr node);

std::unique_ptr<xmlXPathObject, void (*)(xmlXPathObjectPtr)> getByXPath(xmlDocPtr doc,
                                                                        const std::string &xpath);

std::vector<xmlNodePtr> getNodeSet(const xmlXPathObject* xpath_result);

std::string getDefName(xmlDocPtr doc, const std::string& node_xpath);
#endif