#include "helper.hpp"

std::string getXPath(xmlNodePtr node) {
    auto internal_str = xmlGetNodePath(node);
    std::string rval(reinterpret_cast<const char *>(internal_str));
    xmlFree(internal_str);
    return rval;
}
std::string getText(xmlDocPtr doc, xmlNodePtr node) {
    auto internal_str = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
    std::string rval(reinterpret_cast<const char *>(internal_str));
    xmlFree(internal_str);
    return rval;
}
std::unique_ptr<xmlXPathObject, void (*)(xmlXPathObjectPtr)> getByXPath(xmlDocPtr doc,
                                                                        const std::string &xpath) {
    xmlXPathContextPtr context;
    context = xmlXPathNewContext(doc);
    return {xmlXPathEvalExpression(BAD_CAST xpath.c_str(), context), &xmlXPathFreeObject};
}
std::vector<xmlNodePtr> getNodeSet(const xmlXPathObject *xpath_result) {
    auto nodeset_origin = xpath_result->nodesetval;
    if (xmlXPathNodeSetIsEmpty(nodeset_origin))
        return {};
    auto nodeTab = nodeset_origin->nodeTab;
    auto nodeNumbers = nodeset_origin->nodeNr;
    std::vector<xmlNodePtr> result;
    for (int i = 0; i < nodeNumbers; ++i) {
        result.emplace_back(nodeTab[i]);
    }
    return result;
}

std::string getDefName(xmlDocPtr doc, const std::string &node_xpath) {
    const std::string kDefNameStr = "defName";
    std::string current_xpath = node_xpath;
    auto index_of_slash = current_xpath.rfind('/');
    while (index_of_slash != std::string::npos) {
        auto guessStr = current_xpath.erase(index_of_slash+1)+kDefNameStr;
        auto guessResult = getByXPath(doc,guessStr);
        auto guessNodeSet = getNodeSet(guessResult.get());
        if (!guessNodeSet.empty()){
            return getText(doc,guessNodeSet[0]);
        }
        current_xpath = current_xpath.erase(index_of_slash);
        index_of_slash= current_xpath.rfind('/');
    }
    return "";
}
