#pragma once
#include "DefInfo.h"
#include <filesystem>
#include <fmt/format.h>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <tinyxml2.h>

rimtrans::DefInfo a_def(const tinyxml2::XMLElement *node, const std::filesystem::path &file_path,
                        const std::set<std::string> &interested_tags,
                        const std::set<std::string> &list_tags);

auto scan_def(const std::string &src_path, const std::set<std::string> &interested_tags,
              const std::set<std::string> &list_tags,
              std::map<std::string, std::list<rimtrans::DefInfo>> *def_map_by_class = nullptr)
    -> std::map<std::string, std::list<rimtrans::DefInfo>> *;