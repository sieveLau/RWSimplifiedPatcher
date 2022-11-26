//
// Created by Sieve Lau on 2022/11/18.
//
#include "helper.hpp"
#include "shared.hpp"
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <ios>
#include <iostream>
#include <libxml/parser.h>
#include <map>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>
#include <set>
#include <string>
#include <vector>

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

std::string init_search_list() {
    std::string result;
    const char *keywords[]{"label",       "labelPlural",       "labelMale",   "labelMalePlural",
                           "labelFemale", "labelFemalePlural", "description", "title",
                           "titleShort",  "baseDescription",   "verb",        "gerund",
                           "reportString"};
    for (auto *keyword : keywords) {
        result += fmt::format("//{} |", keyword);
    }
    result.pop_back();
    return result;
}

int main(int argc, char **argv) {
    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
#ifndef NDEBUG
    plog::init(plog::debug, &consoleAppender);
#else
    plog::init(plog::warning, &consoleAppender);
#endif
    std::map<std::string, std::vector<std::string>> output_map;
    std::vector<std::string> dir_to_create;

    auto language_dir_prefix = getDirectoryPrefix();
    std::string input;
    switch (argc) {
    case 1:
    case 2:
        std::cout << "outputDir: ";
        std::getline(std::cin, input);
        language_dir_prefix = input + language_dir_prefix;
        std::cout << "DefsDir: ";
        input.clear();
        std::getline(std::cin, input);
        break;
    case 3:
        input = argv[2];
        language_dir_prefix = argv[1] + language_dir_prefix;
        break;
    default: std::cerr << "Too many arguments!"; exit(-1);
    }

    std::set<std::string> keys;

    for (auto &&file : file_walker(input)) {
        std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> doc(
            xmlReadFile(file.string().c_str(), nullptr, XML_PARSE_RECOVER), &xmlFreeDoc);
        std::string target_output_file, target_output_dir;

        auto xpath_result = getByXPath(doc.get(), init_search_list());
        auto match_nodeset = getNodeSet(xpath_result.get());

        if (!match_nodeset.empty()) {
            for (auto *current_node : match_nodeset) {
                auto nodeText = getText(doc.get(), current_node);
                PLOGD << "nodeText: " << nodeText;
                auto xpath = getXPath(current_node);
                PLOGD << "xmlGetNodePath: " << xpath;
                // 要输出的文件所在的目录名，是根据xpath里的"/Defs/"后面暴露出来的"MyNameSpace.MyCustomDef"来确定的
                auto directory = getOutputDirectory(xpath);
                target_output_dir = language_dir_prefix + directory + '/';
                target_output_file = target_output_dir + file.filename().string();

                auto defName = getDefNameFromXPath(doc.get(), xpath);
                // 根据xpath最末尾的一部分来确定是什么tag
                auto what_type = xpath.substr(xpath.rfind('/') + 1);

                if (!defName.empty()) {
                    if ((what_type != "description") && (what_type != "baseDescription")
                        && (what_type != "reportString"))
                        keys.insert(nodeText);
                    if (!(str_contains(xpath, "li[") || str_contains(xpath, "li/"))) {
                        // 如果只是一个普通的label
                        output_map[target_output_file].emplace_back(
                            fmt::format("<{0}.{1}>{2}</{0}.{1}>\n", defName, what_type, nodeText));
                    } else {
                        // 这里处理的就是作为某个li元素里面的label了
                        // 例子是/Defs/AlienRace.ThingDef_AlienRace/tools/li[1]/label
                        auto name = getliParentTagName(xpath);
                        long int li_number;
                        // 有的li有序号，有的没有
                        if (getliNumber(xpath, &li_number)) {
                            auto final_tag_name =
                                // clang-format off
                                fmt::format("<{defName}.{liName}.{liNumber}.{type}>{text}</{defName}.{liName}.{liNumber}.{type}>\n",
                                            fmt::arg("defName",defName),
                                            fmt::arg("liName", name),
                                            fmt::arg("liNumber", li_number),
                                            fmt::arg("type", what_type),
                                            fmt::arg("text", nodeText)
                                            );
                            // clang-format on
                            output_map[target_output_file].emplace_back(final_tag_name);
                        } else
                            // clang-format off
                            output_map[target_output_file].emplace_back(fmt::format(
                                "<{defName}.{liName}.0.{type}>{text}</{defName}.{liName}.0.{type}>\n",
                                fmt::arg("defName", defName),
                                fmt::arg("liName", name),
                                fmt::arg("type", what_type),
                                fmt::arg("text", nodeText)));
                        // clang-format on
                    }
                    dir_to_create.emplace_back(target_output_dir);
                }
            }
        }
    }

    for (auto &&dir : dir_to_create) {
        try {
            std::filesystem::create_directories(dir);
        } catch (std::exception &e) {
            PLOG_WARNING << "Create directory " + dir + " failed, skipping.";
        }
    }

    for (const auto &pair : output_map) {
        auto &&target_output_file = pair.first;
        std::ofstream output;
        output.open(target_output_file, std::ios_base::trunc);
        if (output.is_open()) {
            output << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                      "<LanguageData>\n";
            for (auto &&line : pair.second) {
                output << line;
            }
            output << "</LanguageData>\n";
            output.close();
        } else
            PLOG_WARNING << "Can't write to " << target_output_file << ", skipping.";
    }

    std::ofstream output;
    output.open(language_dir_prefix + "pairs.txt", std::ios_base::trunc);
    if (output.is_open()) {
        for (auto &&key : keys) {

            output << key << ":\n";
        }
        output.close();
    }
    return 0;
}