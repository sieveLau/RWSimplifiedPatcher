//
// Created by Sieve Lau on 2022/12/4.
//

#include "Node.h"
std::string simplexml::Node::str() const {
    return fmt::format("<{0}.{1}>{2}</{0}.{1}>", def_name, tag_name, text);
}
std::string simplexml::ListNode::str() const {
    return fmt::format("<{0}.{1}.{2}.{3}>{4}</{0}.{1}.{2}.{3}>", def_name, list_name, list_index,
                       tag_name, text);
}
void simplexml::translation_unit::add(const Node *node) noexcept { nodes.push_back(node); }
bool simplexml::translation_unit::save() noexcept {
    if (!std::filesystem::exists(file_name.parent_path()))
        std::filesystem::create_directories(file_name.parent_path());
    std::ofstream output_fd(file_name, std::ios_base::trunc);
    if (!output_fd.is_open()) {
        return false;
    }
    output_fd << xml_header() << "\n";
    output_fd << "<LanguageData>\n";
    for (auto *node : nodes) {
        output_fd << node->str() << "\n";
    }
    output_fd << "</LanguageData>\n";
    output_fd.close();
    return true;
}
simplexml::translation_unit::~translation_unit() {
    while (!nodes.empty()) {
        delete nodes.back();
        nodes.pop_back();
    }
}
