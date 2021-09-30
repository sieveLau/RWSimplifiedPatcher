#include "helper.h"
#include "simple_xml.h"
#include <boost/log/trivial.hpp>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libxml/parser.h>
#include <regex>
#include <vector>
constexpr auto def_classes = "defclasses.txt";


using path = std::filesystem::path;
using std::vector;
using std::string;

void generate_operation(simplexml::simple_xml* simple_xml, xml_construct& xml_construct,
                        const vector<string>& defs_to_search_for)
{
	for (auto&& i : defs_to_search_for)
	{

		for (auto operations = xml::helper::search_xml_and_create_operation_list(*simple_xml, i); auto&& operation : operations)
		{
			xml_construct.add_replace_operation(operation);
		}
	}
}


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

vector<string> init_defs(path exe_dir){
	
	BOOST_LOG_TRIVIAL(trace) << "init_defs() start";
	
	vector<string> result;
	// std::cout<<exe_dir<<std::endl;
	path config_path=exe_dir.concat(separator()).concat(def_classes);
	BOOST_LOG_TRIVIAL(info) << "def classes file: " << config_path;
	
	if(exists(config_path)){
		BOOST_LOG_TRIVIAL(trace) << "if(exists(config_path))";
		string line;
		std::ifstream in;
		in.open(config_path);
		while(getline(in,line)){
			result.push_back(line);
			BOOST_LOG_TRIVIAL(info) << "Read: " << line;
		}
		in.close();
	}else{
		BOOST_LOG_TRIVIAL(trace) << "if(exists(config_path))-else";
		result.push_back("ThingDef");
	}
	BOOST_LOG_TRIVIAL(trace) << "init_defs() end";
	return result;
	
}

std::string path_to_string(const path& a_path)
{
	std::wstring temp_buff(a_path.c_str());
	return string(temp_buff.begin(),temp_buff.end());
}

int main(int argc, char** argv)
{
	vector<simplexml::simple_xml*> all_xmls;
	if (argc < 2)
	{
		all_xmls.push_back(new simplexml::simple_xml(
			string(R"(C:\Steam\steamapps\common\RimWorld\Mods\WallStuff\1.3\Defs\Buildings_Misc.xml)")));
	}
	else
	{
		auto src_path = std::filesystem::path(argv[1]);
		if (is_regular_file(src_path)) all_xmls.push_back(new simplexml::simple_xml(src_path));
		if (is_directory(src_path))
		{
			for (auto&& i : file_walker(argv[1]))
			{
				all_xmls.push_back(new simplexml::simple_xml(i));
			}
			// getchar();
		}
		else
		{
			BOOST_LOG_TRIVIAL(fatal) << "open source file(s)/directory failed";
			static_cast<void>(getchar());
			return -1;
		}
	}

	xml_construct xmlc;
	for (simplexml::simple_xml* simplexml : all_xmls)
	{
		generate_operation(simplexml, xmlc, init_defs(path(argv[0]).parent_path()));
	}
	// std::cout<<path(argv[0]).parent_path()<<std::endl;
	xmlc.dump(path_to_string(path(argv[0]).parent_path())+separator()+"all_patch.xml");

	while (!all_xmls.empty())
	{
		auto* xml = all_xmls.back();
		all_xmls.pop_back();
		delete xml;
	}
	xmlCleanupParser();
	// getchar();
	return 0;
}
