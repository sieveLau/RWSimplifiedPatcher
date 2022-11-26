//
// Created by Sieve Lau on 2022/11/18.
//
#include "helper.hpp"
#include "shared.hpp"
#include <cstddef>
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
#include <codecvt>
#include <windows.h>
#include <stringapiset.h>

std::vector<std::filesystem::path> file_walker(const std::wstring &dir,
                                               const std::wstring &extension = L".xml") {
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

std::wstring init_search_list() {
    std::wstring result;
    const wchar_t *keywords[]{L"label",L"labelPlural",L"labelMale",L"labelMalePlural",L"labelFemale",L"labelFemalePlural",L"description",L"title",L"titleShort",L"baseDescription",L"verb",L"gerund",L"reportString"};
    for (auto *keyword : keywords) {
        result += fmt::format(L"//{} |", keyword);
    }
    result.pop_back();
    return result;
}

int main(int argc, char **argv) {
    const std::locale utf8( std::locale(), new std::codecvt_utf8<wchar_t> );
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
    switch (argc) {
    case 1:
    case 2:
        std::wcout << "outputDir: ";
        std::getline(std::wcin, input);
        language_dir_prefix = input + language_dir_prefix;
        std::wcout << "DefsDir: ";
        input.clear();
        std::getline(std::wcin, input);
        break;
    case 3:
        input = s2ws(std::string(argv[2]));
        language_dir_prefix = s2ws(std::string(argv[1])) + language_dir_prefix;
        break;
    default: std::cerr << "Too many arguments!"; exit(-1);
    }

    std::set<std::wstring> keys;

    for (auto &&file : file_walker(input)) {
        std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> doc(
            xmlReadFile(file.string().c_str(), nullptr, XML_PARSE_RECOVER), &xmlFreeDoc);
        std::wstring target_output_file, target_output_dir;

        auto xpath_result = getByXPath(doc.get(), init_search_list());
        auto match_nodeset = getNodeSet(xpath_result.get());

        if (!match_nodeset.empty()) {
            for (auto *current_node : match_nodeset) {
                PLOGD<< reinterpret_cast<const char*>( xmlGetNodePath(current_node));
                auto nodeText = getText(doc.get(), current_node->xmlChildrenNode);
                PLOGD << "nodeText: " << nodeText;
                auto xpath = getXPath(current_node);
                PLOGD << "xmlGetNodePath: " << xpath;
                // Ҫ������ļ����ڵ�Ŀ¼�����Ǹ���xpath���"/Defs/"���汩¶������"MyNameSpace.MyCustomDef"��ȷ����
                auto directory = getOutputDirectory(xpath);
                target_output_dir = language_dir_prefix + directory + L'/';
                target_output_file = target_output_dir + s2ws(file.filename().string());

                auto defName = getDefNameFromXPath(doc.get(), xpath);
                // ����xpath��ĩβ��һ������ȷ����ʲôtag
                auto what_type = xpath.substr(xpath.rfind('/') + 1);

                if (!defName.empty()) {
                    if ((what_type != L"description") && (what_type != L"baseDescription")
                        && (what_type != L"reportString"))
                        keys.insert(nodeText);
                    if (!(str_contains(xpath, L"li[") || str_contains(xpath, L"li/"))) {
                        // ���ֻ��һ����ͨ��label
                        output_map[target_output_file].emplace_back(
                            fmt::format(L"<{0}.{1}>{2}</{0}.{1}>\n", defName, what_type, nodeText));
                    } else {
                        // ���ﴦ��ľ�����Ϊĳ��liԪ�������label��
                        // ������/Defs/AlienRace.ThingDef_AlienRace/tools/li[1]/label
                        auto name = getliParentTagName(xpath);
                        long int li_number;
                        // �е�li����ţ��е�û��
                        if (getliNumber(xpath, &li_number)) {
                            auto final_tag_name =
                                // clang-format off
                                fmt::format(L"<{defName}.{liName}.{liNumber}.{type}>{text}</{defName}.{liName}.{liNumber}.{type}>\n",
                                            fmt::arg(L"defName",defName),
                                            fmt::arg(L"liName", name),
                                            fmt::arg(L"liNumber", li_number),
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