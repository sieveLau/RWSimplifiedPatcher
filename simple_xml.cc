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
    void simple_xml::private_print_element_names(xmlNode *a_node)
    {
        xmlNode *cur_node = NULL;
        static int depth = 0;
        for (cur_node = a_node; cur_node; cur_node = cur_node->next)
        {
            if (cur_node->type == XML_ELEMENT_NODE)
            {
                printf("node type: Element, depth: %d, name: %s\n", depth, cur_node->name);
            }
            depth++;
            private_print_element_names(cur_node->children);
            depth--;
        }
    }

    };// namespace simplexml

xml_construct::xml_construct() : doc_(xmlNewDoc(BAD_CAST "1.0")), root_(xmlNewNode(NULL, BAD_CAST "Patch")) {
	xmlDocSetRootElement(doc_, root_);
}

xml_construct::~xml_construct() {
	xmlFreeDoc(doc_);
}
void xml_construct::add_replace_operation(const simplexml::operation &a_operation)
{
    /*
<Operation Class="PatchOperationReplace">
    <xpath>/Defs/ThingDef[defName="WallscreenTelevision"]/label</xpath>
    <value>
        <label>壁挂式电视</label>
    </value>
</Operation>
     */
    auto *node = xmlNewChild(root_, NULL, BAD_CAST "Operation", NULL);
    xmlNewProp(node, BAD_CAST "Class", BAD_CAST "PatchOperationReplace");

    std::string format_str(R"(/Defs/{}[defName="{}"]/{})");
    char *buff = new char[format_str.size() + a_operation.whatDef.size() + a_operation.defName.size() +
        a_operation.tag.size() + 8];
    sprintf(buff, format_str.c_str(), a_operation.whatDef.c_str(), a_operation.defName.c_str(),
            a_operation.tag.c_str());

    xmlNewChild(node, NULL, BAD_CAST "xpath", BAD_CAST buff);
    auto *value_node = xmlNewChild(node, NULL, BAD_CAST "value", NULL);
    xmlNewChild(value_node, NULL, BAD_CAST a_operation.tag.c_str(), BAD_CAST a_operation.value.c_str());
}
