//
// Created by sieve on 2021/9/25.
//

#include "simple_xml.h"

namespace simplexml {
	string simple_xml::get_string_by_node(xmlNodePtr node) const {
		if (!node) return "";
		auto* str = xmlNodeListGetString(doc_, node->xmlChildrenNode, 1);
		string return_str(reinterpret_cast<char*>(str));
		xmlFree(str);
		return return_str;
	}
	simple_xml::simple_xml(const string& xml_file_path) : doc_(xmlReadFile(xml_file_path.c_str(), NULL,
		XML_PARSE_RECOVER)) {
	}
	simple_xml::~simple_xml() {
		xmlFreeDoc(doc_);
	}
	simple_xml::simple_xml(const std::filesystem::path& xml_file_path) :doc_(nullptr) {
		std::wstring tempbuf(xml_file_path.c_str());
		string usable_str(tempbuf.begin(), tempbuf.end());
		doc_ = xmlReadFile(usable_str.c_str(), NULL, XML_PARSE_RECOVER);
	}
	vector<xmlNodePtr> get_child_elements_by_name(const xmlNode* start_node, const string& name) {

		vector<xmlNodePtr> result;
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
	xmlNodePtr get_child_element_by_name(const xmlNode* start_node, const string& name) {
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

	vector<operation> bulk_create_operation(const simple_xml& simple_xml, string whatDef) {
		vector<operation> result;
		for (auto someDefList = get_child_elements_by_name(simple_xml.get_root_node(), whatDef); auto && i :
		someDefList) {
			auto* defName = get_child_element_by_name(i, "defName");
			auto* temp_ptr = get_child_element_by_name(i, "label");
			if (!defName) continue;
			if (temp_ptr)

				result.push_back({ whatDef, simple_xml.get_string_by_node(defName), "label", simple_xml.get_string_by_node(temp_ptr) });

			temp_ptr = get_child_element_by_name(i, "description");
			if (temp_ptr)

				result.push_back({ whatDef, simple_xml.get_string_by_node(defName), "description", simple_xml.get_string_by_node(temp_ptr) });
		}
		return result;
	}

};// namespace simplexml

xml_construct::xml_construct() : doc_(xmlNewDoc(BAD_CAST "1.0")), root_(xmlNewNode(NULL, BAD_CAST "Patch")) {
	xmlDocSetRootElement(doc_, root_);
}

xml_construct::~xml_construct() {
	xmlFreeDoc(doc_);
}
