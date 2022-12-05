#include "helper.hpp"

std::string getXPath(xmlNodePtr node) {
    auto internal_str = xmlGetNodePath(node);
    std::string rval(reinterpret_cast<char*>(internal_str));
    xmlFree(internal_str);
    return rval;
}

std::string getText(xmlDocPtr doc, xmlNodePtr node) {
    auto internal_str = xmlNodeGetContent(node);
    if(internal_str== nullptr)return "";
    std::string rval(reinterpret_cast<char*>(internal_str));
    xmlFree(internal_str);
    return rval;
}
std::unique_ptr<xmlXPathObject, void (*)(xmlXPathObjectPtr)> getByXPath(xmlDocPtr doc,
                                                                        const std::string &xpath) {
    xmlXPathContextPtr context;
    context = xmlXPathNewContext(doc);

    return {xmlXPathEvalExpression(BAD_CAST(xpath).c_str(), context), &xmlXPathFreeObject};
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

std::string get_li_parent_tag_name(const std::string &xpath_containing_li) {
    std::regex li_pattern(R"(/(\w*)/li(\[|/))");
    std::smatch li_match;
    if (std::regex_search(xpath_containing_li, li_match, li_pattern)) {
        return li_match[1];
    }
    throw std::runtime_error("Invalid xpath: no li found");
}

/**
 * 从xpath中提取li后面的数字
 * @brief 从xpath中提取li后面的数字
 * @param xpath_containing_li 含有形似li[1]的xpath
 * @param result 用于存储结果的指针
 * @return li后面的数字，注意需要-1才是能够作为dot notation的tag名字的
*/
bool get_li_number(const std::string &xpath_containing_li, long *result) {
    std::regex li_number_pattern(R"(/\w*/li\[(\d*)\]/)");
    std::smatch li_match;
    if (std::regex_search(xpath_containing_li, li_match, li_number_pattern)) {
        *result = strtol(li_match[1].str().c_str(), nullptr, 10);
        return true;
    }
    return false;
}

std::string getOutputDirectory(const std::string &xpath) {
    std::string prefix("/Defs/");
    const auto prefix_length = prefix.size();
    // 要输出的文件所在的目录名，是根据xpath里的"/Defs/"后面暴露出来的"MyNameSpace.MyCustomDef"来确定的
    // 首先把"/Defs/"去掉
    auto directory = xpath.substr(xpath.find_first_of(prefix) + prefix_length);
    if (directory.empty()) {
        auto info = "Invalid xpath: " + xpath;
        PLOG_FATAL << info;
        throw std::runtime_error("Invalid xpath");
    }
    // 然后准备裁掉后面的部分，但是可能会遇到带[]的Def
    // 例如/Defs/ResearchProjectDef[1]/label
    const auto index_of_square = directory.find('[');
    const auto index_of_slash = directory.find('/');
    // 需要取二者较小的是因为xpath里面可能会li有[0]而Def部分没有，导致定位错
    // 例如/Defs/AlienRace.ThingDef_AlienRace/tools/li[1]/label
    directory = index_of_square == std::string::npos
        ? directory.erase(index_of_slash)
        : directory.erase(index_of_slash < index_of_square ? index_of_slash : index_of_square);
    PLOGD << "directory: " << directory;
    return directory;
}

std::vector<std::filesystem::path> file_walker(const std::string &dir,
                                               const std::string &extension) {
    std::vector<std::filesystem::path> result;
    using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
    for (const auto &dirEntry : recursive_directory_iterator(dir))
        if (std::filesystem::is_regular_file(dirEntry.path())) {
            if (dirEntry.path().filename().extension() == extension) {
                result.push_back(dirEntry.path());
                PLOGD << "source file found: " << dirEntry.path().filename();
            }
        }
    return result;
}
