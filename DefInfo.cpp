//
// Created by Sieve Lau on 2022/12/4.
//

#include "DefInfo.h"

const std::string &rimtrans::DefInfo::getDefName() const { return def_name_; }
const std::filesystem::path &rimtrans::DefInfo::getSrcFile() const { return src_file_; }
void rimtrans::DefInfo::setDefName(const std::string &defName) { def_name_ = defName; }
void rimtrans::DefInfo::setSrcFile(const std::filesystem::path &srcFile) { src_file_ = srcFile; }
void rimtrans::DefInfo::add_field(const std::string &key, std::string value) {
    this->fields_[key] = std::move(value);
}
std::ostream &rimtrans::operator<<(std::ostream &os, const rimtrans::DefInfo &info) {
    os << "defclass: " << info.def_class_ << "\n"
       << "defName: " << info.def_name_ << "\n"
       << "from file: " << info.src_file_ << "\n"
       << "fields: \n";
    for (auto &&pair : info.fields_) {
        os << fmt::format("  {}={}", pair.first, pair.second) << "\n";
    }
    return os;
}
void rimtrans::DefInfo::add_fields(const std::map<std::string, std::string> &fields) {
    for (auto &&entry : fields) {
        this->add_field(entry.first, entry.second);
    }
}
const std::string &rimtrans::DefInfo::getDefClass() const { return def_class_; }
