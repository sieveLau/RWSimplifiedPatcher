#include "helper.hpp"
#include <regex>
#include <locale>

std::wstring xmlCharToWideString(const xmlChar *xmlString)
{    
    if (!xmlString) { 
        PLOGF<<"provided string was null";
        abort(); 
    } //provided string was null
    try
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
        return conv.from_bytes((const char*)xmlString);
    }
    catch(const std::range_error& e)
    {
        PLOGF<<e.what();
        abort(); //wstring_convert failed
    }
}

std::wstring s2ws(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}

std::wstring getXPath(xmlNodePtr node) {
    auto internal_str = xmlGetNodePath(node);
    std::wstring rval = xmlCharToWideString(internal_str);
    xmlFree(internal_str);
    return rval;
}
std::wstring getText(xmlDocPtr doc, xmlNodePtr node) {
    auto internal_str = xmlNodeGetContent(node);
    std::wstring rval=xmlCharToWideString(internal_str);
    xmlFree(internal_str);
    return rval;
}
std::unique_ptr<xmlXPathObject, void (*)(xmlXPathObjectPtr)> getByXPath(xmlDocPtr doc,
                                                                        const std::wstring &xpath) {
    xmlXPathContextPtr context;
    context = xmlXPathNewContext(doc);

    return {xmlXPathEvalExpression(BAD_CAST (ws2s(xpath)).c_str(), context), &xmlXPathFreeObject};
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

std::wstring getliParentTagName(const std::wstring &xpath_containing_li) {
    std::wregex li_pattern(L"/(\\w*)/li(\\[|/)");
    std::wsmatch li_match;
    if (std::regex_search(xpath_containing_li, li_match, li_pattern)) {
        return li_match[1];
    }
    throw std::runtime_error("Invalid xpath: no li found");
}

bool getliNumber(const std::wstring &xpath_containing_li, long *result) {
    std::wregex li_number_pattern(LR"(/\w*/li\[(\d*)\]/)");
    std::wsmatch li_match;
    if (std::regex_search(xpath_containing_li, li_match, li_number_pattern)) {
        *result = _wtol(li_match[1].str().c_str()) - 1;
        return true;
    }
    return false;
}

std::wstring getOutputDirectory(const std::wstring &xpath) {
    std::wstring prefix(L"/Defs/");
    const auto prefix_length = prefix.size();
    // Ҫ������ļ����ڵ�Ŀ¼�����Ǹ���xpath���"/Defs/"���汩¶������"MyNameSpace.MyCustomDef"��ȷ����
    // ���Ȱ�"/Defs/"ȥ��
    auto directory = xpath.substr(xpath.find_first_of(prefix) + prefix_length);
    if (directory.empty()) {
        auto info = L"Invalid xpath: " + xpath;
        PLOG_FATAL << info;
        throw std::runtime_error("Invalid xpath");
    }
    // Ȼ��׼���õ�����Ĳ��֣����ǿ��ܻ�������[]��Def
    // ����/Defs/ResearchProjectDef[1]/label
    const auto index_of_square = directory.find(L'[');
    const auto index_of_slash = directory.find(L'/');
    // ��Ҫȡ���߽�С������Ϊxpath������ܻ�li��[0]��Def����û�У����¶�λ��
    // ����/Defs/AlienRace.ThingDef_AlienRace/tools/li[1]/label
    directory = index_of_square == std::wstring::npos
        ? directory.erase(index_of_slash)
        : directory.erase(index_of_slash<index_of_square?index_of_slash:index_of_square);
    PLOGD << "directory: " << directory;
    return directory;
}