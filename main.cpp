//
// Created by Sieve Lau on 2022/11/18.
//

#ifdef WIN32
#endif

#include "Node.h"
#include "helper/helper.hpp"
#include "shared.hpp"
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
    using path = std::filesystem::path;
    std::wcout.imbue(get_locale());
    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
#ifndef NDEBUG
    plog::init(plog::debug, &consoleAppender);
#else
    plog::init(plog::warning, &consoleAppender);
#endif
    std::map<std::wstring, simplexml::translation_unit *> output_map;

    auto language_dir_prefix = getDirectoryPrefix();
    std::wstring input;
    std::wstring output_dir_from_user;
    switch (argc) {
    case 1:
    case 2:
        std::wcout << L"outputDir: ";
        std::getline(std::wcin, output_dir_from_user);
        std::wcout << L"DefsDir: ";
        std::getline(std::wcin, input);
        break;
    case 3:
        input = s2ws(std::string(argv[2]));
        output_dir_from_user = s2ws(std::string(argv[1]));
        break;
    default: std::cerr << "Too many arguments!"; exit(-1);
    }

    path output_definjected_destination = path(output_dir_from_user) / language_dir_prefix;

    //    language_dir_prefix = output_dir_from_user + language_dir_prefix;

    std::set<std::wstring> keys;

    for (auto &&file : file_walker(input)) {
        std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> doc(
            xmlReadFile(file.string().c_str(), nullptr, XML_PARSE_RECOVER), &xmlFreeDoc);
        path target_output_file;

        auto xpath_result = getByXPath(doc.get(), init_search_list());
        auto match_nodeset = getNodeSet(xpath_result.get());

        if (!match_nodeset.empty()) {
            for (auto *current_node : match_nodeset) {

                auto xpath = getXPath(current_node);
                PLOGD << "xmlGetNodePath: " << xpath;
                // 要输出的文件所在的目录名，是根据xpath里的"/Defs/"后面暴露出来的"MyNameSpace.MyCustomDef"来确定的
                auto directory = getOutputDirectory(xpath);
                target_output_file = output_definjected_destination / directory / file.filename();
                if (std::filesystem::exists(target_output_file))
                    target_output_file += L".new";

                auto defName = getDefNameFromXPath(doc.get(), xpath);
                if (!defName.empty()) {
                    // 根据xpath最末尾的一部分来确定是什么tag
                    auto what_type = xpath.substr(xpath.rfind('/') + 1);

                    std::wstring nodeText;
                    try {
                        nodeText = getText(doc.get(), current_node->xmlChildrenNode);
                    } catch (empty_node_text &e) { nodeText = L"origin_empty//" + defName; }
                    PLOGD << "nodeText: " << nodeText;

                    if (output_map[target_output_file] == nullptr)
                        output_map[target_output_file] =
                            new simplexml::translation_unit(target_output_file);

                    if ((what_type != L"description") && (what_type != L"baseDescription")
                        && (what_type != L"reportString"))
                        keys.insert(nodeText);

                    if (!(str_contains(xpath, L"li[") || str_contains(xpath, L"li/")))
                        // 如果只是一个普通的label
                        output_map[target_output_file]->add(
                            new simplexml::Node(defName, what_type, nodeText));
                    else {
                        // 这里处理的就是作为某个li元素里面的label了
                        // 例子是/Defs/AlienRace.ThingDef_AlienRace/tools/li[1]/label
                        auto li_name = get_li_parent_tag_name(xpath);
                        long li_number = 1;
                        // 有的li有序号，有的没有
                        get_li_number(xpath, &li_number);
                        output_map[target_output_file]->add(new simplexml::ListNode(
                            defName, li_name, li_number - 1, what_type, nodeText));
                    }
                }
            }
        }
    }

    for (const auto &pair : output_map) {
        pair.second->save();
        delete pair.second;
    }

    std::wofstream output;
    output.open(output_definjected_destination / L"pairs.txt", std::ios_base::trunc);
    output.imbue(get_locale());
    if (output.is_open()) {
        for (auto &&key : keys) {
            output << key << L":\n";
        }
        output.close();
    }
    return 0;
}