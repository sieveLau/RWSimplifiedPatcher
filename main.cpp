#include "simple_xml.h"
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <libxml/parser.h>
#include <regex>
#include <vector>
#include <boost/log/trivial.hpp>
#include <memory>
#include <map>
constexpr auto def_classes = "defclasses.txt";


using path = std::filesystem::path;
using std::vector;
using std::string;


vector<path> file_walker(const string& dir)
{
	vector<path> result;
	using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
	for (const auto& dirEntry : recursive_directory_iterator(dir))
		if (std::filesystem::is_regular_file(dirEntry.path()))
		{
			if (dirEntry.path().filename().extension() == ".xml")
			{
				result.push_back(dirEntry.path());
				BOOST_LOG_TRIVIAL(debug) << "source file found: " << dirEntry.path().filename();
			}
		}
	return result;
}

inline const char* separator()
{
#ifdef _WIN32
	return "\\";
#else
    return "/";
#endif
}

vector<string> init_defs(path exe_dir)
{
	BOOST_LOG_TRIVIAL(trace) << "init_defs() start";

	vector<string> result;
	path config_path = exe_dir / def_classes;;
	// std::cout<<exe_dir<<std::endl;
	BOOST_LOG_TRIVIAL(info) << "def classes file: " << config_path;

	if (exists(config_path))
	{
		BOOST_LOG_TRIVIAL(trace) << "if(exists(config_path))";
		string line;
		std::ifstream in;
		in.open(config_path);
		while (getline(in, line))
		{
			result.push_back(line);
			BOOST_LOG_TRIVIAL(info) << "Read: " << line;
		}
		in.close();
	}
	else
	{
		BOOST_LOG_TRIVIAL(trace) << "if(exists(config_path))-else";
		result.push_back("ThingDef");
	}
	BOOST_LOG_TRIVIAL(trace) << "init_defs() end";
	return result;
}

std::string path_to_string(path a_path)
{
	std::wstring temp_buff(a_path.c_str());
	return string(temp_buff.begin(), temp_buff.end());
}

auto xml_parser(std::string path) -> std::map<std::string, std::vector<simplexml::operation>>
{
	using std::unique_ptr;
	unique_ptr<xmlDoc, void(*)(xmlDocPtr)> doc(xmlReadFile(path.c_str(), NULL, XML_PARSE_RECOVER), &xmlFreeDoc);
	auto* doc_root = xmlDocGetRootElement(doc.get());
	std::map<std::string, std::vector<simplexml::operation>> xml_cache;

	auto* first_level_def = doc_root->xmlChildrenNode;
	while (first_level_def)
	{
		std::string def_type(reinterpret_cast<const char*>(first_level_def->name));
		if (def_type.ends_with("Def"))
		{
			BOOST_LOG_TRIVIAL(info) << "Found: " << def_type;
			static const xmlChar* defName_str = BAD_CAST"defName";
			static const xmlChar* label_str = BAD_CAST"label";
			static const xmlChar* description_str = BAD_CAST"description";
			simplexml::operation a_operation({def_type});
			bool ready = false;
			for (auto* second_level_element = first_level_def->children; second_level_element; second_level_element =
			     second_level_element->next)
			{
				if (!xmlStrcmp(second_level_element->name, defName_str))
				{
					a_operation.defName = reinterpret_cast<const char*>(xmlNodeListGetString(
						doc.get(), second_level_element->children, 1));
					ready = true;
					continue;
				}
				if (!xmlStrcmp(second_level_element->name, label_str))
				{
					a_operation.tag = "label";
					a_operation.value = reinterpret_cast<const char*>(xmlNodeListGetString(
						doc.get(), second_level_element->children, 1));
					if (ready)xml_cache[def_type].push_back(a_operation);
					continue;
				}
				if (!xmlStrcmp(second_level_element->name, description_str))
				{
					a_operation.tag = "description";
					a_operation.value = reinterpret_cast<const char*>(xmlNodeListGetString(
						doc.get(), second_level_element->children, 1));
					if (ready)xml_cache[def_type].push_back(a_operation);
				}
			}
		}
		first_level_def = first_level_def->next;
	}
	return xml_cache;
}

inline void add_operation(xml_construct* construct, std::map<std::string, std::vector<simplexml::operation>> cache_map)
{
	for (auto const& [key, val] : cache_map)
		for (auto&& item : val)
			construct->add_replace_operation(item);
}

int main(int argc, char** argv)
{
	const std::string exe_dir = path_to_string(path(argv[0]).parent_path());
	// const auto defs = init_defs(exe_dir);
	if (argc < 2)
	{
		BOOST_LOG_TRIVIAL(fatal) << "no source file";
		std::cout << "usage: RWSimplifiedPatcher.exe <Defs folder>\n";
		std::cout << "       RWSimplifiedPatcher.exe <xml file>"<<std::endl;
		exit(EXIT_FAILURE);
	}
	xml_construct xmlc;
	if (std::filesystem::is_regular_file(argv[1])) add_operation(&xmlc, xml_parser(argv[1]));
	else if (std::filesystem::is_directory(argv[1])) 
		for (auto&& i : file_walker(argv[1])) add_operation(&xmlc, xml_parser(path_to_string(i)));
	else
	{
		BOOST_LOG_TRIVIAL(fatal) << "open source file(s)/directory failed";
		static_cast<void>(getchar());
		return -1;
	}
	xmlc.dump(exe_dir + separator() + "all_patch.xml");
	xmlCleanupParser();
	return 0;
}
