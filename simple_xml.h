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
