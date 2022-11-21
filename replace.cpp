#include <fstream>
#include <iostream>
#include <string>
#include <regex>
#include <fmt/format.h>

int main(int argc, char **argv) {
    std::regex pair_pattern("(.{1,}):(.{1,})");
    std::string format_str =
        R"(egrep -rl '>{origin}<' {directory} | xargs -I@ sed -i '' 's/>{origin}</>{replacement}</g' @)";

    std::cout << "pairs.txt: ";
    std::string line;
    std::getline(std::cin, line);

    std::string search_dir;
    std::cout << "Path to DefInjected: ";
    std::getline(std::cin, search_dir);

    std::ifstream pairs_file_handler;
    pairs_file_handler.open(line);
    std::ofstream output_file;
    output_file.open(search_dir+"/commands.txt");

    if (pairs_file_handler.is_open()&&output_file.is_open()) {
        while(std::getline(pairs_file_handler,line)) {
            std::smatch match;
            if (std::regex_match(line, match, pair_pattern)) {
                output_file << fmt::format(format_str, fmt::arg("origin", match[1].str()),
                                           fmt::arg("replacement", match[2].str()),
                                           fmt::arg("directory", search_dir))
                            << "\n";
            }
        }
        pairs_file_handler.close();
        output_file.close();
    }

    return 0;
}