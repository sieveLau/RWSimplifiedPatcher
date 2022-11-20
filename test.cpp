//
// Created by Sieve Lau on 2022/11/18.
//
#include "helper.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>
#include <set>
#include <string>
#include <vector>

// getdefname
// 从xpath右侧开始切"/"然后拼接"defName"，直到找到为止

std::vector<std::filesystem::path> file_walker(const std::string &dir,
                                               const std::string &extension = ".xml") {
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

void touch(const std::string &path) {
    std::ofstream output;
    output.open(path, std::ios_base::app);
    output.close();
}

bool getNodeSets(std::unique_ptr<xmlXPathObject, void (*)(xmlXPathObjectPtr)> xpath_result,
                 xmlNodeSetPtr nodeset_storage) {
    if (xmlXPathNodeSetIsEmpty(xpath_result->nodesetval)) {
        return false;
    }
    nodeset_storage = xpath_result->nodesetval;
    return true;
}

int main(int argc, char **argv) {
    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::debug, &consoleAppender);
    std::string language_dir_prefix = "Languages/ChineseSimplified/DefInjected/";
    std::cout << "outputDir: ";
    std::string input;
    std::getline(std::cin, input);
    language_dir_prefix = input + language_dir_prefix;
    std::cout << "DefsDir: ";
    input.clear();
    std::getline(std::cin, input);

    for (auto &&file : file_walker(input)) {
        std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> doc(
            xmlReadFile(file.c_str(), NULL, XML_PARSE_RECOVER), &xmlFreeDoc);
        std::string xml_constructor;
        xml_constructor += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                           "<LanguageData>\n";
        std::string target_output_file, target_output_dir;
        bool hasContent = false;

        auto label_no_li_result = getByXPath(doc.get(), "//label");
        auto label_nodeset = getNodeSet(label_no_li_result.get());
        if (!label_nodeset.empty()) {
            for (auto *label_node : label_nodeset) {
                //                auto label_node = nodeset->nodeTab[i];
                auto label_text = getText(doc.get(), label_node);
                PLOGD << "label_text: " << label_text;
                auto xpath = getXPath(label_node);
                PLOGD << "xmlGetNodePath: " << xpath;
                // 要输出的文件所在的目录名，是根据xpath里的"/Defs/"后面暴露出来的"MyNameSpace.MyCustomDef"来确定的
                // 首先把"/Defs/"去掉
                auto directory = xpath.substr(xpath.find_first_of("/Defs/") + strlen("/Defs/"));
                // 然后准备裁掉后面的部分，但是可能会遇到带[]的Def
                // 例如/Defs/ResearchProjectDef[1]/label
                auto index_of_square = directory.find('[');
                auto index_of_slash = directory.find('/');
                // 需要取二者较小的是因为xpath里面可能会li有[0]而Def部分没有，导致定位错
                // 例如/Defs/AlienRace.ThingDef_AlienRace/tools/li[1]/label
                directory = index_of_square == std::string::npos
                    ? directory.substr(0, directory.find('/'))
                    : directory.substr(0, std::min(index_of_square, index_of_slash));
                PLOGD << "directory: " << directory;
                // 在第一次运行的时候，到这一步为止都不会输出内容，所以可以放心建立文件夹，同时对输出文件的名字进行设置
                // 对于同一个文件，没有必要生成两次文件夹，而且输出文件肯定是同一个，所以这里只需要执行一次就可以了
                if (!hasContent) {
                    target_output_dir = language_dir_prefix + directory + '/';
                    target_output_file = target_output_dir + file.filename().string();
                }
                // 首先确定当前的label是不是某个li里的元素
                auto xpath_li_index = xpath.find("li[");
                if (xpath_li_index == std::string::npos) {
                    // 如果只是一个普通的label，就用xpath来获取它所属的defName
                    // 以便生成Dot Notation，类似<defName.label>的结构
                    auto defName_result =
                        getByXPath(doc.get(), xpath.substr(0, xpath.rfind('/') + 1) + "defName");
                    auto defName_nodeset = getNodeSet(defName_result.get());
                    if (!defName_nodeset.empty()) {
                        auto defName = getText(doc.get(), defName_nodeset[0]);
                        xml_constructor +=
                            fmt::format("<{0}.label>{1}</{0}.label>\n", defName, label_text);
                        // 找到了需要输出的内容，所以要把这个flag设置成true
                        hasContent = true;
                        assert(defName == getDefName(doc.get(), xpath));
                    }
                } else {
                    // 这里处理的就是作为某个li元素里面的label了
                    // 例子是/Defs/AlienRace.ThingDef_AlienRace/tools/li[1]/label
                    // 首先也是要找到defName，首先把"tools/li[1]"以及后面的切掉
                    auto temp_str = xpath.substr(0, xpath_li_index - 1);
                    temp_str = temp_str.substr(0, temp_str.rfind('/') + 1);
                    // 获取一个mark，标记li的上级tag（即"tools"）的位置，方便后面生成Dot Notation
                    auto mark = temp_str.size();
                    temp_str.append("defName");
                    PLOGD << "[label in li]defName XPath:" << temp_str;
                    auto temp_result = getByXPath(doc.get(), temp_str);
                    auto label_in_li_defName_nodeset = temp_result->nodesetval;
                    if (!xmlXPathNodeSetIsEmpty(label_in_li_defName_nodeset)) {
                        auto li_defName =
                            getText(doc.get(), label_in_li_defName_nodeset->nodeTab[0]);
                        auto name = xpath.substr(mark, xpath.find('/', mark) - mark);
                        auto li_number =
                            atoi(xpath.substr(xpath_li_index + strlen("li["), 1).c_str()) - 1;
                        auto final_tag_name =
                            fmt::format("<{0}.{1}.{2}.label>{3}</{0}.{1}.{2}.label>\n", li_defName,
                                        name, li_number, label_text);
                        xml_constructor += final_tag_name;
                        hasContent = true;
                    }
                }
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
                    directory = index_of_square == std::string::npos
                        ? directory.substr(0, directory.find('/'))
                        : directory.substr(0, std::min(index_of_square, index_of_slash));
                    target_output_dir = language_dir_prefix + directory + '/';
                    target_output_file = target_output_dir + file.filename().string();
                }
                auto xpath_has_li = xpath.find("li[") != std::string::npos;
                if (!xpath_has_li) {
                    auto defName_result =
                        getByXPath(doc.get(), xpath.substr(0, xpath.rfind('/') + 1) + "defName");
                    if (!xmlXPathNodeSetIsEmpty(defName_result->nodesetval)) {
                        auto nodeset = defName_result->nodesetval;
                        auto defName = getText(doc.get(), nodeset->nodeTab[0]);
                        xml_constructor += fmt::format("<{0}.{1}>{2}</{0}.{1}>\n", defName,
                                                       "description", description_text);
                        hasContent = true;
                    }
                }
                // PLOG_DEBUG << file.filename().string() + "searched description";
            }
        }

        if (hasContent) {
            std::filesystem::create_directories(target_output_dir);
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
