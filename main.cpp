//
// Created by Sieve Lau on 2022/11/18.
//

#ifdef WIN32
#endif

#include "helper/helper.hpp"
#include "shared.hpp"
#include <codecvt>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/xchar.h>
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

std::wstring init_search_list() {
    std::wstring result;
    const wchar_t *keywords[]{L"label",
                              L"labelShort",
                              L"labelPlural",
                              L"labelMale",
                              L"labelMalePlural",
                              L"labelFemale",
                              L"labelFemalePlural",
                              L"description",
                              L"deathMessage",
                              L"title",
                              L"helpText",
                              L"titleShort",
                              L"baseDescription",
                              L"verb",
                              L"gerund",
                              L"reportString",
                              L"jobString",
                              L"beginLetter",
                              L"beginLetterLabel",
                              L"recoveryMessage",
                              L"baseInspectLine",
                              L"formatString"};
    for (auto *keyword : keywords) {
        result += fmt::format(L"//{} |", keyword);
    }
    result.pop_back();
    return result;
}

int main(int argc, char **argv) {
    const std::locale utf8(std::locale(), new std::codecvt_utf8<wchar_t>);
    std::wcout.imbue(utf8);
    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
#ifndef NDEBUG
    plog::init(plog::debug, &consoleAppender);
#else
    plog::init(plog::warning, &consoleAppender);
#endif
    std::map<std::wstring, std::vector<std::wstring>> output_map;
    std::vector<std::wstring> dir_to_create;

    auto language_dir_prefix = getDirectoryPrefix();
    std::wstring input;
    std::wstring outputDir_from_user;
    switch (argc) {
    case 1:
    case 2:
        std::wcout << "outputDir: ";
        std::getline(std::wcin, outputDir_from_user);
        std::wcout << "DefsDir: ";
        std::getline(std::wcin, input);
        break;
    case 3:
        input = s2ws(std::string(argv[2]));
        outputDir_from_user = s2ws(std::string(argv[1]));
        break;
    default: std::cerr << "Too many arguments!"; exit(-1);
    }

    if (!outputDir_from_user.ends_with(L'\\'))
        outputDir_from_user += L'\\';
    language_dir_prefix = outputDir_from_user + language_dir_prefix;

    std::set<std::wstring> keys;

    for (auto &&file : file_walker(input)) {
        std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> doc(
            xmlReadFile(file.string().c_str(), nullptr, XML_PARSE_RECOVER), &xmlFreeDoc);
        std::wstring target_output_file, target_output_dir;

        auto xpath_result = getByXPath(doc.get(), init_search_list());
        auto match_nodeset = getNodeSet(xpath_result.get());

        if (!match_nodeset.empty()) {
            for (auto *current_node : match_nodeset) {
                PLOGD << reinterpret_cast<const char *>(xmlGetNodePath(current_node));

                auto xpath = getXPath(current_node);
                PLOGD << "xmlGetNodePath: " << xpath;
                // 要输出的文件所在的目录名，是根据xpath里的"/Defs/"后面暴露出来的"MyNameSpace.MyCustomDef"来确定的
                auto directory = getOutputDirectory(xpath);
                target_output_dir = language_dir_prefix + directory + L'/';
                target_output_file = target_output_dir + s2ws(file.filename().string());

                auto defName = getDefNameFromXPath(doc.get(), xpath);
                // 根据xpath最末尾的一部分来确定是什么tag
                auto what_type = xpath.substr(xpath.rfind('/') + 1);

                std::wstring nodeText;
                try {
                    nodeText = getText(doc.get(), current_node->xmlChildrenNode);
                } catch (empty_node_text &e) { nodeText = L"origin_empty//" + defName; }
                PLOGD << "nodeText: " << nodeText;

                if (!defName.empty()) {
                    if ((what_type != L"description") && (what_type != L"baseDescription")
                        && (what_type != L"reportString"))
                        keys.insert(nodeText);
                    if (!(str_contains(xpath, L"li[") || str_contains(xpath, L"li/"))) {
                        // 如果只是一个普通的label
                        output_map[target_output_file].emplace_back(
                            fmt::format(L"<{0}.{1}>{2}</{0}.{1}>\n", defName, what_type, nodeText));
                    } else {
                        // 这里处理的就是作为某个li元素里面的label了
                        // 例子是/Defs/AlienRace.ThingDef_AlienRace/tools/li[1]/label
                        auto name = get_li_parent_tag_name(xpath);
                        long int li_number;
                        // 有的li有序号，有的没有
                        if (get_li_number(xpath, &li_number)) {
                            auto final_tag_name =
                                // clang-format off
                                fmt::format(L"<{defName}.{liName}.{liNumber}.{type}>{text}</{defName}.{liName}.{liNumber}.{type}>\n",
                                            fmt::arg(L"defName",defName),
                                            fmt::arg(L"liName", name),
                                            fmt::arg(L"liNumber", li_number - 1),
                                            fmt::arg(L"type", what_type),
                                            fmt::arg(L"text", nodeText)
                                            );
                            // clang-format on
                            output_map[target_output_file].emplace_back(final_tag_name);
                        } else
                            // clang-format off
                            output_map[target_output_file].emplace_back(fmt::format(
                                L"<{defName}.{liName}.0.{type}>{text}</{defName}.{liName}.0.{type}>\n",
                                fmt::arg(L"defName", defName),
                                fmt::arg(L"liName", name),
                                fmt::arg(L"type", what_type),
                                fmt::arg(L"text", nodeText)));
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
            PLOG_WARNING << L"Create directory " + dir + L" failed, skipping.";
        }
    }

    for (const auto &pair : output_map) {
        auto &&target_output_file = pair.first;
        std::wofstream output;
        output.open(target_output_file, std::ios_base::trunc);
        output.imbue(utf8);
        if (output.is_open()) {
            output << L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                      L"<LanguageData>\n";
            for (auto &&line : pair.second) {
                output << line;
            }
            output << L"</LanguageData>\n";
            output.close();
        } else
            PLOG_WARNING << L"Can't write to " << target_output_file << L", skipping.";
    }

    std::wofstream output;
    output.open(language_dir_prefix + L"pairs.txt", std::ios_base::trunc);
    output.imbue(utf8);
    if (output.is_open()) {
        for (auto &&key : keys) {

            output << key << L":\n";
        }
        output.close();
    }
    return 0;
}