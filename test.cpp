//
// Created by Sieve Lau on 2022/11/18.
//
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>
#include <set>
#include <string>
#include <vector>

std::vector<std::filesystem::path> file_walker(const std::string &dir, const std::string &extension = ".xml") {
  std::vector<std::filesystem::path> result;
  using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
  for (const auto &dirEntry : recursive_directory_iterator(dir))
    if (std::filesystem::is_regular_file(dirEntry.path())) {
      if (dirEntry.path().filename().extension() == extension) {
        result.push_back(dirEntry.path());
        PLOGD << "source file found: " << dirEntry.path().filename();
      }
    }
  return result;
}

inline std::string getText(xmlDocPtr doc, xmlNodePtr node) {
  auto internal_str = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
  std::string rval(reinterpret_cast<const char *>(internal_str));
  xmlFree(internal_str);
  return rval;
}

inline std::string getXPath(xmlNodePtr node) {
  auto internal_str = xmlGetNodePath(node);
  std::string rval(reinterpret_cast<const char *>(internal_str));
  xmlFree(internal_str);
  return rval;
}

void touch(const std::string &path) {
  std::ofstream output;
  output.open(path, std::ios_base::app);
  output.close();
}

std::unique_ptr<xmlXPathObject, void (*)(xmlXPathObjectPtr)> getByXPath(xmlDocPtr doc, const std::string &xpath) {
  xmlXPathContextPtr context;
  context = xmlXPathNewContext(doc);
  return {xmlXPathEvalExpression(BAD_CAST xpath.c_str(), context),
          &xmlXPathFreeObject};
}

bool getNodeSets(std::unique_ptr<xmlXPathObject, void (*)(xmlXPathObjectPtr)> xpath_result, xmlNodeSetPtr nodeset_storage) {
  if (xmlXPathNodeSetIsEmpty(xpath_result->nodesetval)) {
    return false;
  }
  return xpath_result->nodesetval;
}

int main(int argc, char **argv) {
  static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
  plog::init(plog::debug, &consoleAppender);
  std::string language_dir_prefix = "Languages/ChineseSimplified/DefInjected/";
  std::cout<< "outputDir: ";
  std::string input;
  std::getline(std::cin,input);
  language_dir_prefix=input+language_dir_prefix;
  std::cout<< "DefsDir: ";
  input.clear();
  std::getline(std::cin,input);

  for (auto &&file : file_walker(input)) {
    std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> doc(
        xmlReadFile(file.c_str(), NULL, XML_PARSE_RECOVER),
        &xmlFreeDoc);
    std::string xml_constructor;
    xml_constructor += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                       "<LanguageData>\n";
    std::string target_output_file;
    bool hasContent = false;

    auto label_no_li_result = getByXPath(doc.get(), "//label");
    if (!xmlXPathNodeSetIsEmpty(label_no_li_result->nodesetval)) {
      auto nodeset = label_no_li_result->nodesetval;
      for (auto i = 0; i < nodeset->nodeNr; i++) {
        auto label_node = nodeset->nodeTab[i];
        auto label_text = getText(doc.get(), label_node);
        auto xpath = getXPath(label_node);
        auto directory = xpath.substr(xpath.find_first_of("/Defs/") + strlen("/Defs/"));
        auto index_of_square = directory.find('[');
        auto index_of_slash = directory.find('/');
        // 需要取二者较小的是因为xpath里面可能会li有[0]而Def部分没有，导致定位错
        // 例如/Defs/AlienRace.ThingDef_AlienRace/tools/li[1]/label
        directory = index_of_square == std::string::npos ? directory.substr(0, directory.find('/')) : directory.substr(0, std::min(index_of_square, index_of_slash));
        if (!hasContent) {
          std::filesystem::create_directories(language_dir_prefix + directory);
          target_output_file = language_dir_prefix + directory + "/" + file.filename().string();
        }
        auto xpath_li_index = xpath.find("li[");
        if (xpath_li_index == std::string::npos) {
          auto defName_result = getByXPath(doc.get(), xpath.substr(0, xpath.rfind('/') + 1) + "defName");
          if (!xmlXPathNodeSetIsEmpty(defName_result->nodesetval)) {
            auto nodeset = defName_result->nodesetval;
            auto defName = getText(doc.get(), nodeset->nodeTab[0]);
            xml_constructor += fmt::format("<{0}.{1}>\n{2}\n</{0}.{1}>\n", defName, "label", label_text);
            hasContent = true;
          }
        } else {
          auto temp_str = xpath.substr(0, xpath_li_index - 1);
          temp_str = temp_str.substr(0, temp_str.rfind('/') + 1);
//          auto square_index = temp_str.find('[');
//          if (square_index != std::string::npos)
//            temp_str.erase(temp_str.find('['), temp_str.rfind(']'));
          //
          auto mark = temp_str.size();
          temp_str.append("defName");
          PLOGD<<temp_str;
          auto temp_result = getByXPath(doc.get(), temp_str);
          if (!xmlXPathNodeSetIsEmpty(temp_result->nodesetval)) {

            auto li_defName = getText(doc.get(), temp_result->nodesetval->nodeTab[0]);
            auto name = xpath.substr(mark,xpath.find('/',mark)-mark);
            auto li_number =  atoi(xpath.substr(xpath_li_index+ strlen("li["),1).c_str())-1;
            auto final_tag_name = fmt::format("<{0}.{1}.{2}.label>\n{3}\n</{0}.{1}.{2}.label>\n",li_defName,name,li_number,label_text);
            xml_constructor += final_tag_name;
            hasContent=true;
          }
        }
        PLOGD << directory;
        PLOGD << "xmlGetNodePath: " << getXPath(label_node);
        PLOGD << "label_text: " << label_text;
      }
    }

    auto desciption_result = getByXPath(doc.get(), "//description");
    if (!xmlXPathNodeSetIsEmpty(desciption_result->nodesetval)) {
      auto nodeset = desciption_result->nodesetval;
      for (auto i = 0; i < nodeset->nodeNr; i++) {
        auto description_node = nodeset->nodeTab[i];
        auto description_text = getText(doc.get(), description_node);
        auto xpath = getXPath(description_node);
        if (!hasContent) {
          auto directory = xpath.substr(xpath.find_first_of("/Defs/") + strlen("/Defs/"));
          auto index_of_square = directory.find('[');
          auto index_of_slash = directory.find('/');
          // 需要取二者较小的是因为xpath里面可能会li有[0]而Def部分没有，导致定位错
          // 例如/Defs/AlienRace.ThingDef_AlienRace/tools/li[1]/label
          directory = index_of_square == std::string::npos ? directory.substr(0, directory.find('/')) : directory.substr(0, std::min(index_of_square, index_of_slash));
          std::filesystem::create_directories(language_dir_prefix + directory);
          target_output_file = language_dir_prefix + directory + "/" + file.filename().string();
        }
        auto xpath_has_li = xpath.find("li[") != std::string::npos;
        if (!xpath_has_li) {
          auto defName_result = getByXPath(doc.get(), xpath.substr(0, xpath.rfind('/') + 1) + "defName");
          if (!xmlXPathNodeSetIsEmpty(defName_result->nodesetval)) {
            auto nodeset = defName_result->nodesetval;
            auto defName = getText(doc.get(), nodeset->nodeTab[0]);
            xml_constructor += fmt::format("<{0}.{1}>\n{2}\n</{0}.{1}>\n", defName, "description", description_text);
            hasContent = true;
          }
        }
        PLOG_DEBUG << file.filename().string() + "searched description";
      }
    }

    if (hasContent) {

      xml_constructor += "</LanguageData>\n";
      std::ofstream output;
      output.open(target_output_file, std::ios_base::trunc);
      output << xml_constructor;
      output.close();
      xml_constructor.clear();
    }
  }
  return 0;
}