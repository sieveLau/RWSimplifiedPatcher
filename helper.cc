//
// Created by sieve on 2021/9/30.
//

#include "helper.h"
std::vector<xmlNodePtr> xml::helper::get_child_elements_by_name(const xmlNode *start_node, const string &name)
{

    std::vector<xmlNodePtr> result;
    if ((start_node && start_node->xmlChildrenNode)) {
        xmlNodePtr cur = const_cast<xmlNodePtr>(start_node->xmlChildrenNode);
        while (cur != NULL) {
            if (!xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>(name.c_str()))) {
                result.push_back(cur);
            }
            cur = cur->next;
        }
    }
    return result;
}
xmlNodePtr xml::helper::get_child_element_by_name(const xmlNode *start_node, const string &name)
{
    if ((start_node && start_node->xmlChildrenNode)) {
        xmlNodePtr cur = const_cast<xmlNodePtr>(start_node)->children;
        while (cur != NULL) {
            if (!xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>(name.c_str()))) {
                return cur;
            }
            cur = cur->next;
        }
    }
    return NULL;
}
std::vector<simplexml::operation> xml::helper::search_xml_and_create_operation_list(const simplexml::simple_xml &simple_xml,
                                                                     const string& whatDef)
{
    std::vector<simplexml::operation> result;
    for (auto someDefList = get_child_elements_by_name(simple_xml.get_root_node(), whatDef); auto && i :
        someDefList) {
        auto* defName = get_child_element_by_name(i, "defName");
        auto* temp_ptr = get_child_element_by_name(i, "label");
        [[unlikely]] if (!defName) continue;
        if (temp_ptr)

            result.push_back({ whatDef, simple_xml.get_string_by_node(defName), "label", simple_xml.get_string_by_node(temp_ptr) });

        temp_ptr = get_child_element_by_name(i, "description");
        if (temp_ptr)

            result.push_back({ whatDef, simple_xml.get_string_by_node(defName), "description", simple_xml.get_string_by_node(temp_ptr) });
    }
    return result;
}
