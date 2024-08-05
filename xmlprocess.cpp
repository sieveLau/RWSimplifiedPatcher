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
        auto full_tag = fmt::format("{}.{}.{}", parent_tag, count++, label_text);
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
            definfo.add_field(tag_name, current_node->GetText());
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
