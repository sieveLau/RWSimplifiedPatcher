#include "xmlprocess.hpp"

// 传递一个 child 都是 li 的 node，生成对应的 tag 和 value
/** 例如：
<stage>
   <li>
       bla bal bla
   </li>
   <li>
       fo fo fo
   </li>
</stage>
那么应该把 <stage> 这个 node 传进来，而不是把 li 传进来
**/
// TODO: use ordered_map like https://github.com/Tessil/ordered-map
std::map<std::string, std::string> li_processor(const tinyxml2::XMLElement *node) {
    std::map<std::string, std::string> ret;
    const std::string parent_tag(node->Name());
    const char *label_tag = "label";
    uint8_t count = 0;
    for (auto *current_node = node->FirstChildElement(); current_node != nullptr;
         current_node = current_node->NextSiblingElement()) {
        auto *label_element = current_node->FirstChildElement(label_tag);
        if (label_element == nullptr)
            continue;
        std::string label_text = label_element->GetText();
        auto full_tag = fmt::format("{}.{}.{}", parent_tag, count++, label_tag);
        ret[full_tag] = label_text;
    }
    return ret;
}

// remember to free the returned object
rimtrans::DefInfo a_def(const tinyxml2::XMLElement *node, const std::filesystem::path &file_path,
                        const std::set<std::string> &interested_tags,
                        const std::set<std::string> &list_tags) {
    // somehow, the QueryAttribute will modify the pointer, so we need a dummy "handle" in order to
    // free the memory
    char *def_name_buffer = new char[256]{'\0'};
    const auto *handle = def_name_buffer;
    // first, check if custom Def, which write the class name in attribute Class
    // Example: <Def Class="Rimatomics.RimatomicsFailureDef">
    auto error = node->QueryAttribute("Class", &handle);
    // if not found, then the class name should be the tag name
    // Example: <DamageDef>
    if (error == tinyxml2::XMLError::XML_NO_ATTRIBUTE) {
        strncpy(const_cast<char *>(handle), node->Name(), 255);
    }

    rimtrans::DefInfo definfo(handle);

    for (auto *current_node = node->FirstChildElement(); current_node != nullptr;
         current_node = current_node->NextSiblingElement()) {
        std::string tag_name(current_node->Name());
        if (tag_name == "defName") {
            definfo.setDefName(current_node->GetText());
            continue;
        }
        if (interested_tags.contains(tag_name)) {
            const char * text = current_node->GetText();
            definfo.add_field(tag_name, text == nullptr? "" : text);
            continue;
        }
        if (list_tags.contains(tag_name)) {
            definfo.add_fields(li_processor(current_node));
        }
    }

    delete[] def_name_buffer;
    definfo.setSrcFile(file_path);
    return definfo;
}

auto scan_def(const std::string &src_path, const std::set<std::string> &interested_tags,
              const std::set<std::string> &list_tags,
              std::map<std::string, std::list<rimtrans::DefInfo>> *def_map_by_class)
    -> std::map<std::string, std::list<rimtrans::DefInfo>> * {
    using namespace std::filesystem;
    if (def_map_by_class == nullptr) {
        def_map_by_class = new std::map<std::string, std::list<rimtrans::DefInfo>>();
    }
    for (auto &&entry : recursive_directory_iterator(src_path)) {
        if (entry.is_regular_file()) {
            if (entry.path().extension() != ".xml")
                continue;
            auto abs_path = absolute(entry);
            // load file 加载
            tinyxml2::XMLDocument doc;
            doc.LoadFile(abs_path.c_str());
            auto *root = doc.RootElement();
            // if no child, next file
            if (root->ChildElementCount() == 0)
                continue;
            // get first child, this should arrive the def_class depth
            for (auto *current_node = root->FirstChildElement(); current_node != nullptr;
                 current_node = current_node->NextSiblingElement()) {
                if (current_node->ChildElementCount() == 0) {
                    continue;
                }
                // get one step deeper, try to find the tag: defName
                auto *first_tag = current_node->FirstChildElement("defName");
                if (first_tag == nullptr)
                    continue;
                auto r_def = a_def(current_node, abs_path, interested_tags, list_tags);
                if (!r_def.empty()) {
                    // found some meaningful content to be translated
                    if (!def_map_by_class->contains(r_def.getDefClass())) {
                        // if not in map, create one with empty list
                        (*def_map_by_class)[r_def.getDefClass()] = std::list<rimtrans::DefInfo>();
                    }
                    // add to the list
                    (*def_map_by_class)[r_def.getDefClass()].emplace_back(r_def);
                }
            }
        }
    }
    return def_map_by_class;
}

auto format_def_to_xml_element(const rimtrans::DefInfo& definfo, tinyxml2::XMLElement& root) ->tinyxml2::XMLElement& {
    for (const auto& field : definfo.get_fields()) {
        auto * element = root.InsertNewChildElement(fmt::format("{}.{}", definfo.getDefName(), field.first).c_str());
        element->SetText(field.second.c_str());
    }
    return root;
}

void format_defs_to_file(const std::filesystem::path& translation_mod_root_path, const std::map<std::string, std::list<rimtrans::DefInfo>>& def_map_by_class, const std::string& lang){
    using std::filesystem::path;
    using tinyxml2::XMLDocument;
    using tinyxml2::XMLElement;
    const path output_lang_dir = translation_mod_root_path/"Languages"/lang/"DefInjected";
    std::map<path, XMLDocument*> output_dict;
    for (const auto& pair : def_map_by_class){
        const path output_class_path = output_lang_dir/pair.first;
        for (const auto & def : pair.second){
            const path output_file_path = output_class_path/def.getSrcFile().filename();
            XMLElement* root = nullptr;
            if (!output_dict.contains(output_file_path)) {
                auto* output_doc = new XMLDocument();
                root = output_doc->NewElement("LanguageData");
                output_doc->InsertFirstChild(root);
                output_dict[output_file_path] = output_doc;
            } else {
                root = output_dict[output_file_path]->RootElement();
            }
            format_def_to_xml_element(def, *root);
        }
    }
    using std::filesystem::exists;
    using std::filesystem::create_directories;
    using tinyxml2::XMLPrinter;
    using std::unique_ptr;
    for (auto && pair : output_dict) {
        auto output_file = pair.first;
        if (exists(output_file)) {
            output_file+=".new";
        }
        auto output_file_parent = output_file.parent_path();
        if (!exists(output_file_parent)){
            create_directories(output_file_parent);
        }
        auto* doc = pair.second;
        auto fp = unique_ptr<FILE,decltype(&fclose)>(fopen(output_file.c_str(), "wb"), &fclose);
        custom_printer printer(fp.get());
        // Rimworld requires UTF-8 With BOM
        printer.PushHeader(true, true);
        doc->Print(&printer);
    }
}

void copy_included_trans(const std::filesystem::path& translation_mod_root_path, const std::filesystem::path& original_mod_root_path, const std::string& lang){
    using std::filesystem::path;
    using std::filesystem::exists;
    using std::filesystem::recursive_directory_iterator;
    using std::filesystem::absolute;
    using std::filesystem::create_directories;
    using std::filesystem::copy;
    using tinyxml2::XMLDocument;
    auto lang_dir = path("Languages")/lang;
    for (auto& dir_entry : recursive_directory_iterator(original_mod_root_path)){
        if (dir_entry.is_regular_file() && dir_entry.path().extension() == ".xml"){
            auto full_path_str = absolute(dir_entry.path()).string();
            auto index_of_Lang = full_path_str.find(lang_dir);
            if (index_of_Lang != std::string::npos) {
                auto output_file = translation_mod_root_path/full_path_str.substr(index_of_Lang);
//                if (exists(output_file))
                output_file+=".fromorigin";
//                else
                create_directories(output_file.parent_path());
                copy(full_path_str, output_file, std::filesystem::copy_options::overwrite_existing);
            }
        }
    }
    // if the original mod has translated Keyed, the previous step has copied
    // we just copy the english one to serve as reference
    auto english_keyed = original_mod_root_path/"Languages"/"English"/"Keyed";
    auto lang_keyed = translation_mod_root_path/"Languages"/lang/"Keyed";
    if (exists(english_keyed)) {
        if (!exists(lang_keyed)) {
            create_directories(lang_keyed);
        }
        for (auto& dir_entry : recursive_directory_iterator(english_keyed)) {
            if (dir_entry.is_regular_file() && dir_entry.path().extension() == ".xml"){
                auto output_file = lang_keyed/dir_entry.path().filename();
                output_file+=".english";
                copy(absolute(dir_entry.path()), output_file,std::filesystem::copy_options::overwrite_existing);
            }
        }
    }
}

void create_and_insert(tinyxml2::XMLDocument& doc, const char* tag_name, const char* tag_text){
    auto* doc_root = doc.RootElement();
    auto* new_node = doc.NewElement(tag_name);
    new_node->SetText(tag_text);
    doc_root->InsertEndChild(new_node);
}

void auto_about(const std::filesystem::path& translation_mod_root_path, const std::filesystem::path& original_mod_root_path, const std::string& version){
    using std::filesystem::path;
    using std::filesystem::exists;
    using std::filesystem::absolute;
    using std::filesystem::create_directories;
    using tinyxml2::XMLDocument;
    using tinyxml2::XMLElement;

    auto origin_about = original_mod_root_path/"About"/"About.xml";
    XMLDocument in_doc;
    in_doc.LoadFile(origin_about.c_str());
    auto * in_root = in_doc.RootElement();

    auto output_about = translation_mod_root_path/"About"/"About.xml";
    if (!exists(output_about.parent_path())) {
        create_directories(output_about.parent_path());
    }
    XMLDocument out_doc;
    auto* out_root = out_doc.NewElement("ModMetaData");
    out_doc.InsertFirstChild(out_root);

    // name
    auto in_text = in_root->FirstChildElement("name")->GetText();
    create_and_insert(out_doc, "name", fmt::format("{} 简体汉化", in_text).c_str());

    // author
    create_and_insert(out_doc, "author", "anonymous");

    // supportedVersions
    auto *out_node = out_doc.NewElement("supportedVersions");
    out_root->InsertEndChild(out_node);
    out_node = out_doc.NewElement("li");
    out_node->SetText(version.c_str());
    out_root->FirstChildElement("supportedVersions")->InsertEndChild(out_node);

    // packageId
    in_text = in_root->FirstChildElement("packageId")->GetText();
    create_and_insert(out_doc, "packageId", fmt::format("zh.{}", in_text).c_str());

    // loadAfter
    create_and_insert(out_doc, "loadAfter","");
    out_node = out_root->FirstChildElement("loadAfter")->InsertNewChildElement("li");
    out_node->SetText(in_text);

    std::unique_ptr<FILE, decltype(&fclose)> fd(fopen(output_about.c_str(),"wb"),&fclose);
    tinyxml2::XMLPrinter printer(fd.get());
    printer.PushHeader(true, true);
    out_doc.Print(&printer);
}