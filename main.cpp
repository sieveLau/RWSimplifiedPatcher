#include "consts.h"
#include "simple_xml.h"
#include "tools.h"
#include <filesystem>
#include <iostream>
#include <libxml/parser.h>
#include <map>
#include <memory>
#include <vector>
#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>

using std::string;
using std::vector;

auto xml_parser(const std::string &path) -> std::map<std::string, std::vector<simplexml::operation>> {
  std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> doc(
      xmlReadFile(path.c_str(), NULL, XML_PARSE_RECOVER),
      &xmlFreeDoc);
  auto *doc_root = xmlDocGetRootElement(doc.get());
  std::map<std::string, std::vector<simplexml::operation>> xml_cache;

  for (auto *first_level_def = doc_root->xmlChildrenNode;
       first_level_def;
       first_level_def = first_level_def->next) {
    std::string def_type(reinterpret_cast<const char *>(first_level_def->name));
    // 在整个第一层里找以Def结尾的node
    if (def_type.ends_with("Def")) {
      PLOG_DEBUG<< "Found: " << def_type;
      static const xmlChar *defName_str = BAD_CAST "defName";
      static const xmlChar *label_str = BAD_CAST "label";
      static const xmlChar *description_str = BAD_CAST "description";
      simplexml::operation a_operation({def_type});
      bool ready = false;
      // 对于每个Def，分别找里面的defName、label和可能存在的description
      for (auto *second_level_element = first_level_def->children;
           second_level_element;
           second_level_element = second_level_element->next) {
        // 找defName，只有这个找到了，label和description才有意义
        if (!xmlStrcmp(second_level_element->name, defName_str)) {
          a_operation.defName = reinterpret_cast<const char *>(xmlNodeListGetString(
              doc.get(), second_level_element->children, 1));
          ready = true;
          continue;
        }
        // 找label，如果已经找到了它所属的defName，就生成一个节点
        if (ready && (!xmlStrcmp(second_level_element->name, label_str))) {
          a_operation.tag = "label";
          a_operation.value = reinterpret_cast<const char *>(xmlNodeListGetString(
              doc.get(), second_level_element->children, 1));
          xml_cache[def_type].push_back(a_operation);
          continue;
        }
        // 找description，如果已经找到了它所属的defName，就生成一个节点
        if (ready && (!xmlStrcmp(second_level_element->name, description_str))) {
          a_operation.tag = "description";
          a_operation.value = reinterpret_cast<const char *>(xmlNodeListGetString(
              doc.get(), second_level_element->children, 1));
          xml_cache[def_type].push_back(a_operation);
        }
      }
    }
  }
  return xml_cache;
}

inline void add_operation(xml_construct *construct, const std::map<std::string, std::vector<simplexml::operation>> &cache_map) {
  for (auto const &[key, val] : cache_map)
    for (auto &&item : val)
      construct->add_replace_operation(item);
}

int main(int argc, char **argv) {
  using path = std::filesystem::path;
  static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
  plog::init(plog::debug,&consoleAppender);
#ifdef _WIN32
  const std::string exe_dir = path_to_string(path(argv[0]).parent_path());
#else
  const std::string exe_dir = path(argv[0]).parent_path();
#endif
  std::string source;
  if (argc < 2) {
#ifdef _WIN32
    PLOGF <<  "no source file";
    std::cout << "usage: RWSimplifiedPatcher <Defs folder>\n";
    std::cout << "       RWSimplifiedPatcher <xml file>\n";
    exit(EXIT_FAILURE);
#else
    printf("source file or source dir: ");
    std::getline(std::cin,source);
#endif
  }else{
    source=argv[1];
  }
  xml_construct xmlc;
  if (std::filesystem::is_regular_file(source))
    add_operation(&xmlc, xml_parser(source));
  else if (std::filesystem::is_directory(source))
#ifdef _WIN32
    for (auto &&i : file_walker(argv[1]))
      add_operation(&xmlc, xml_parser(path_to_string(i)));
#else
    for (auto &&i : file_walker(source))
      add_operation(&xmlc, xml_parser(i.string()));
#endif
  else {
    PLOG_FATAL << "open source file(s)/directory failed";
    static_cast<void>(getchar());
    return -1;
  }
#ifndef _WIN32
  xmlc.dump();
#else
  xmlc.dump(exe_dir + separator() + "all_patch.xml");
#endif
  xmlCleanupParser();
  return 0;
}
