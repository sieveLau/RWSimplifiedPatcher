//
// Created by Sieve Lau on 2022/11/18.
//

#ifndef RWSIMPLIFIEDPATCHER_CMAKE_BUILD_DEBUG_TOOLS_H
#define RWSIMPLIFIEDPATCHER_CMAKE_BUILD_DEBUG_TOOLS_H
#include "consts.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <plog/Log.h>

std::vector<std::filesystem::path> file_walker(const std::string &dir);

std::vector<std::string> init_defs(const std::filesystem::path &exe_dir);

#ifdef _WIN32
std::string path_to_string(std::filesystem::path a_path);
#endif

#endif//RWSIMPLIFIEDPATCHER_CMAKE_BUILD_DEBUG_TOOLS_H
