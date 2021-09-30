//
// Created by sieve on 2021/9/25.
//
#pragma once
#include <filesystem>
#include <format>
#include <libxml/parser.h>
#include <string>
#include <vector>

using std::string;
using std::vector;
namespace simplexml
{

class simple_xml
{
    xmlDocPtr doc_;

    void private_print_element_names(xmlNode *a_node);

  public:
    explicit simple_xml(const string &xml_file_path);
    explicit simple_xml(const std::filesystem::path &xml_file_path);
    ~simple_xml();
    string get_string_by_node(xmlNodePtr) const;

    xmlNodePtr get_root_node() const
    {
        return xmlDocGetRootElement(doc_);
    }

    void print_element_names()
    {
        private_print_element_names(get_root_node());
    }
};
struct operation
{
    string whatDef;
    string defName;
    // label»òdescription
    string tag;
    // <xpath>/Defs/ThingDef[defName="WallscreenTelevision"]/label</xpath>
    // string xpath;
    // value
    string value;
};
} // namespace simplexml

class xml_construct
{
    xmlDoc *doc_;
    xmlNode *root_;

  public:
    xml_construct();

    xml_construct(const xml_construct &another) : doc_(xmlCopyDoc(another.doc_, 1)), root_(xmlDocGetRootElement(doc_))
    {
    }

    friend void swap(xml_construct &left, xml_construct &right)
    {
        using std::swap;
        swap(left.doc_, right.doc_);
        swap(left.root_, right.root_);
    }

    xml_construct(xml_construct &&another) noexcept : xml_construct()
    {
        swap(*this, another);
    }

    xml_construct &operator=(xml_construct another)
    {
        swap(*this, another);
        return *this;
    }

    ~xml_construct();

    void add_replace_operation(const simplexml::operation &a_operation);

    void dump(const string &filename = "")
    {
        xmlSaveFormatFileEnc(filename.empty() ? "-" : filename.c_str(), doc_, "UTF-8", 1);
    }
};
