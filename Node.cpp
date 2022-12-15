//
// Created by Sieve Lau on 2022/12/4.
//

#include "Node.h"
std::wstring simplexml::Node::str() const {
    return fmt::format(L"<{0}.{1}>{2}</{0}.{1}>", def_name, tag_name, text);
}
std::wstring simplexml::ListNode::str() const {
    return fmt::format(L"<{0}.{1}.{2}.{3}>{4}</{0}.{1}.{2}.{3}>", def_name, list_name, list_index,
                       tag_name, text);
}
void simplexml::translation_unit::add(const Node *node) noexcept { nodes.push_back(node); }
bool simplexml::translation_unit::save() noexcept {
    if (!std::filesystem::exists(file_name.parent_path()))
        std::filesystem::create_directories(file_name.parent_path());
    std::wofstream output_fd(file_name, std::ios_base::trunc);
    output_fd.imbue(get_locale());
    if (!output_fd.is_open()) {
        return false;
    }
    output_fd << xml_header() << L"\n";
    output_fd << L"<LanguageData>\n";
    for (auto *node : nodes) {
        output_fd << node->str() << L"\n";
    }
    output_fd << L"</LanguageData>\n";
    output_fd.close();
    return true;
}
simplexml::translation_unit::~translation_unit() {
    while (!nodes.empty()) {
        delete nodes.back();
        nodes.pop_back();
    }
}
