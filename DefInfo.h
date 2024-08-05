#pragma once
#include <fmt/format.h>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <filesystem>

namespace rimtrans {
class DefInfo {
    std::string def_class_;
    std::string def_name_;
    std::filesystem::path src_file_;
    std::map<std::string, std::string> fields_;

  public:
    explicit DefInfo(std::string defclass) : def_class_(std::move(defclass)){};

    DefInfo(const DefInfo &other) = default;

    friend void swap(DefInfo &dest, DefInfo &src) noexcept {
        using std::swap;
        swap(dest.def_class_, src.def_class_);
        swap(dest.def_name_, src.def_name_);
        swap(dest.src_file_, src.src_file_);
        swap(dest.fields_, src.fields_);
    }

    DefInfo(DefInfo &&other) noexcept : DefInfo("") { swap(*this, other); }

    DefInfo &operator=(const DefInfo &other) {
        DefInfo temp(other);
        swap(*this, temp);

        return *this;
    }
    const std::string &getDefClass() const;
    const std::string &getDefName() const;
    const std::filesystem::path &getSrcFile() const;
    void setDefName(const std::string &defName);
    void setSrcFile(const std::filesystem::path &srcFile);
    const auto &get_fields() const { return fields_; }
    bool empty() const { return fields_.empty(); }
    void add_field(const std::string &key, std::string value);
    void add_fields(const std::map<std::string, std::string> &fields);
    friend std::ostream &operator<<(std::ostream &os, const DefInfo &info);
};
}// namespace rimtrans