#include "helper.hpp"

std::wstring s2ws(const std::string &str) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring &wstr) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}

std::wstring xmlCharToWideString(const xmlChar *xmlString) {
    if (!xmlString) {
        PLOGF << "provided string was null";
        throw std::runtime_error("[helper.cpp xmlCharToWideString]Provided string was null!");
    }
    try {
        return s2ws((const char *) xmlString);
    } catch (const std::exception &e) {
        PLOGF << e.what();
        abort();//wstring_convert failed
    }
}

std::wstring getXPath(xmlNodePtr node) {
    auto internal_str = xmlGetNodePath(node);
    std::wstring rval = xmlCharToWideString(internal_str);
    xmlFree(internal_str);
    return rval;
}

std::wstring getText(xmlDocPtr doc, xmlNodePtr node) {
    auto internal_str = xmlNodeGetContent(node);
    std::wstring rval;
    try {
        rval = xmlCharToWideString(internal_str);
    } catch (std::exception &e) {
        PLOGD << e.what();
        throw empty_node_text();
    }
    xmlFree(internal_str);
    return rval;
}
std::unique_ptr<xmlXPathObject, void (*)(xmlXPathObjectPtr)> getByXPath(xmlDocPtr doc,
                                                                        const std::wstring &xpath) {
    xmlXPathContextPtr context;
    context = xmlXPathNewContext(doc);

    return {xmlXPathEvalExpression(BAD_CAST(ws2s(xpath)).c_str(), context), &xmlXPathFreeObject};
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

std::wstring getDefNameFromXPath(xmlDocPtr doc, const std::wstring &node_xpath) {
    const std::wstring kDefNameStr = L"defName";
    std::wstring current_xpath = node_xpath;
    auto index_of_slash = current_xpath.rfind(L'/');
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
    return L"";
}

std::wstring get_li_parent_tag_name(const std::wstring &xpath_containing_li) {
    std::wregex li_pattern(L"/(\\w*)/li(\\[|/)");
    std::wsmatch li_match;
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
bool get_li_number(const std::wstring &xpath_containing_li, long *result) {
    std::wregex li_number_pattern(LR"(/\w*/li\[(\d*)\]/)");
    std::wsmatch li_match;
    if (std::regex_search(xpath_containing_li, li_match, li_number_pattern)) {
        *result = std::wcstol(li_match[1].str().c_str(), nullptr, 10);
        return true;
    }
    return false;
}

std::wstring getOutputDirectory(const std::wstring &xpath) {
    std::wstring prefix(L"/Defs/");
    const auto prefix_length = prefix.size();
    // 要输出的文件所在的目录名，是根据xpath里的"/Defs/"后面暴露出来的"MyNameSpace.MyCustomDef"来确定的
    // 首先把"/Defs/"去掉
    auto directory = xpath.substr(xpath.find_first_of(prefix) + prefix_length);
    if (directory.empty()) {
        auto info = L"Invalid xpath: " + xpath;
        PLOG_FATAL << info;
        throw std::runtime_error("Invalid xpath");
    }
    // 然后准备裁掉后面的部分，但是可能会遇到带[]的Def
    // 例如/Defs/ResearchProjectDef[1]/label
    const auto index_of_square = directory.find(L'[');
    const auto index_of_slash = directory.find(L'/');
    // 需要取二者较小的是因为xpath里面可能会li有[0]而Def部分没有，导致定位错
    // 例如/Defs/AlienRace.ThingDef_AlienRace/tools/li[1]/label
    directory = index_of_square == std::wstring::npos
        ? directory.erase(index_of_slash)
        : directory.erase(index_of_slash < index_of_square ? index_of_slash : index_of_square);
    PLOGD << "directory: " << directory;
    return directory;
}

std::vector<std::filesystem::path> file_walker(const std::wstring &dir,
                                               const std::wstring &extension) {
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
