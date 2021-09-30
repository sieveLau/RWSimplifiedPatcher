//
// Created by sieve on 2021/9/30.
//

#ifndef RWSIMPLIFIEDPATCHER__HELPER_H_
#define RWSIMPLIFIEDPATCHER__HELPER_H_

#include "simple_xml.h"
namespace xml::helper{

std::vector<xmlNodePtr> get_child_elements_by_name(const xmlNode* start_node, const string& name);
xmlNodePtr get_child_element_by_name(const xmlNode* start_node, const string& name);
std::vector<simplexml::operation> search_xml_and_create_operation_list(const simplexml::simple_xml& simple_xml, const string& whatDef);
}

#endif // RWSIMPLIFIEDPATCHER__HELPER_H_
