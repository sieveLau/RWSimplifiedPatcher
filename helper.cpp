#include "helper.hpp"
#include <regex>
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
    result.reserve(nodeNumbers);
for (int i = 0; i < nodeNumbers; ++i) {
        result.emplace_back(nodeTab[i]);
    }
    return result;
}

std::string getDefNameFromXPath(xmlDocPtr doc, const std::string &node_xpath) {
    const std::string kDefNameStr = "defName";
    std::string current_xpath = node_xpath;
    auto index_of_slash = current_xpath.rfind('/');
    while (index_of_slash != std::string::npos) {
        auto guessStr = current_xpath.erase(index_of_slash + 1) + kDefNameStr;
        auto guessResult = getByXPath(doc, guessStr);
        auto guessNodeSet = getNodeSet(guessResult.get());
        if (!guessNodeSet.empty()) {
            return getText(doc, guessNodeSet[0]);
        }
        current_xpath = current_xpath.erase(index_of_slash);
        index_of_slash = current_xpath.rfind('/');
    }
    return "";
}

std::string getliParentTagName(const std::string &xpath_containing_li) {
    std::regex li_pattern("/(\\w*)/li(\\[|/)");
    std::smatch li_match;
    if (std::regex_search(xpath_containing_li, li_match, li_pattern)) {
        return li_match[1];
    }
    throw std::runtime_error("Invalid xpath: no li found");
}

bool getliNumber(const std::string &xpath_containing_li, long *result) {
    std::regex li_number_pattern(R"(/\w*/li\[(\d*)\]/)");
    std::smatch li_match;
    if (std::regex_search(xpath_containing_li, li_match, li_number_pattern)) {
        *result = atol(li_match[1].str().c_str()) - 1;
        return true;
    }
    return false;
}

std::string getOutputDirectory(const std::string &xpath) {
    auto *prefix = "/Defs/";
    const auto prefix_length = strlen(prefix);
    // 要输出的文件所在的目录名，是根据xpath里的"/Defs/"后面暴露出来的"MyNameSpace.MyCustomDef"来确定的
    // 首先把"/Defs/"去掉
    auto directory = xpath.substr(xpath.find_first_of(prefix) + prefix_length);
    if (directory.empty()) {
        auto info = "Invalid xpath: " + xpath;
        PLOG_FATAL << info;
        throw std::runtime_error(info);
    }
    // 然后准备裁掉后面的部分，但是可能会遇到带[]的Def
    // 例如/Defs/ResearchProjectDef[1]/label
    const auto index_of_square = directory.find('[');
    const auto index_of_slash = directory.find('/');
    // 需要取二者较小的是因为xpath里面可能会li有[0]而Def部分没有，导致定位错
    // 例如/Defs/AlienRace.ThingDef_AlienRace/tools/li[1]/label
    directory = index_of_square == std::string::npos
        ? directory.erase(index_of_slash)
        : directory.erase(std::min(index_of_square, index_of_slash));
    PLOGD << "directory: " << directory;
    return directory;
}