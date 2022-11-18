//
// Created by Sieve Lau on 2022/11/18.
//

#include "tools.h"
std::vector<std::filesystem::path> file_walker(const std::string &dir) {
  std::vector<std::filesystem::path> result;
  using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
  for (const auto &dirEntry : recursive_directory_iterator(dir))
    if (std::filesystem::is_regular_file(dirEntry.path())) {
      if (dirEntry.path().filename().extension() == ".xml") {
        result.push_back(dirEntry.path());
        PLOGD << "source file found: " << dirEntry.path().filename();
      }
    }
  return result;
}

std::vector<std::string> init_defs(const std::filesystem::path &exe_dir) {
  PLOG_DEBUG << "init_defs() start";

  std::vector<std::string> result;
  std::filesystem::path config_path = exe_dir / def_classes;
  // std::cout<<exe_dir<<std::endl;
  PLOG_DEBUG << "def classes file: " << config_path;

  if (exists(config_path)) {
    PLOG_DEBUG << "if(exists(config_path))";
    std::string line;
    std::ifstream in;
    in.open(config_path);
    while (getline(in, line)) {
      result.push_back(line);
      PLOG_DEBUG << "Read: " << line;
    }
    in.close();
  } else {
    PLOG_DEBUG << "if(exists(config_path))-else";
    result.emplace_back("ThingDef");
  }
  PLOG_DEBUG << "init_defs() end";
  return result;
}

#ifdef _WIN32
std::string path_to_string(std::filesystem::path a_path) {
  std::string temp_buff(a_path.c_str());
  return std::string(temp_buff.begin(), temp_buff.end());
}
#endif