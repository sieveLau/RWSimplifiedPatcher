//
// Created by sieve on 2021/9/25.
//
#pragma once
#include <filesystem>
#include <format>
#include <libxml/parser.h>
#include <string>
#include <vector>

using std::vector;
using std::string;
namespace simplexml {
vector<xmlNodePtr> get_child_elements_by_name(const xmlNode *start_node, const string &name);

xmlNodePtr get_child_element_by_name(const xmlNode *start_node, const string &name);

class simple_xml {
  xmlDocPtr doc_;

  void private_print_element_names(xmlNode *a_node) {
    xmlNode *cur_node = NULL;
    static int depth = 0;
    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
      if (cur_node->type == XML_ELEMENT_NODE) {
        printf("node type: Element, depth: %d, name: %s\n", depth, cur_node->name);
      }
      depth++;
      private_print_element_names(cur_node->children);
      depth--;
    }
  }

 public:
  explicit simple_xml(const string &xml_file_path);
  explicit simple_xml(const std::filesystem::path &xml_file_path);
  ~simple_xml();
  string get_string_by_node(xmlNodePtr) const;

  xmlNodePtr get_root_node() const {
    return xmlDocGetRootElement(doc_);
  }

  void print_element_names() {
    private_print_element_names(get_root_node());
  }
};
struct operation
{
  string whatDef;
  string defName;
  // label或description
  string tag;
  // <xpath>/Defs/ThingDef[defName="WallscreenTelevision"]/label</xpath>
  // string xpath;
  // value
  string value;
};
vector<operation> bulk_create_operation(const simple_xml &simple_xml, string whatDef);
}// namespace simplexml

class xml_construct {
  xmlDoc *doc_;
  xmlNode *root_;

 public:
  xml_construct();

  xml_construct(const xml_construct &another) : doc_(xmlCopyDoc(another.doc_, 1)), root_(xmlDocGetRootElement(doc_)) {
  }

  friend void swap(xml_construct &left, xml_construct &right) {
    using std::swap;
    swap(left.doc_, right.doc_);
    swap(left.root_, right.root_);
  }

  xml_construct(xml_construct &&another) noexcept : xml_construct() {
    swap(*this, another);
  }

  xml_construct &operator=(xml_construct another) {
    swap(*this, another);
    return *this;
  }

  ~xml_construct();

  void add_replace_operation(const simplexml::operation &a_operation) {
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
    string full_xpath(std::format(R"(/Defs/{}[defName="{}"]/{})", a_operation.whatDef, a_operation.defName,
                                       a_operation.tag));
    xmlNewChild(node, NULL, BAD_CAST "xpath", BAD_CAST full_xpath.c_str());
    auto *value_node = xmlNewChild(node, NULL, BAD_CAST "value", NULL);
    xmlNewChild(value_node, NULL, BAD_CAST a_operation.tag.c_str(), BAD_CAST a_operation.value.c_str());
  }

  void dump(const string &filename = "") {
    xmlSaveFormatFileEnc(filename.empty() ? "-" : filename.c_str(), doc_, "UTF-8", 1);
  }
};
