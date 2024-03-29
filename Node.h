//
// Created by Sieve Lau on 2022/12/4.
//

#ifndef RWSIMPLIFIEDPATCHER__NODE_H
#define RWSIMPLIFIEDPATCHER__NODE_H
#include "helper/helper.hpp"
#include "shared.hpp"
#include <fmt/format.h>
#include <fmt/xchar.h>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace simplexml {

class Node {
  protected:
    std::wstring def_name, tag_name, text;
    Node() = default;

  public:
    explicit Node(std::wstring def_name, std::wstring tag_name, std::wstring text)
        : def_name(std::move(def_name)), tag_name(std::move(tag_name)), text(std::move(text)) {}

    Node(const Node &another) = default;

    friend void swap(Node &left, Node &right) {
        using std::swap;
        swap(left.def_name, right.def_name);
        swap(left.tag_name, right.tag_name);
        swap(left.text, right.text);
    }

    Node(Node &&another) noexcept : Node() { swap(*this, another); }

    Node &operator=(Node another) {
        swap(*this, another);
        return *this;
    }

    virtual std::wstring str() const;
    virtual ~Node() = default;
};

class ListNode final : public Node {
    std::wstring list_name;
    long list_index = 0;
    ListNode() = default;

  public:
    explicit ListNode(std::wstring def_name, std::wstring list_name, long list_index,
                      std::wstring tag_name, std::wstring text)
        : Node(std::move(def_name), std::move(tag_name), std::move(text)),
          list_name(std::move(list_name)), list_index(list_index) {}

    ListNode(const ListNode &another) = default;

    friend void swap(ListNode &left, ListNode &right) {
        using std::swap;
        swap(left.def_name, right.def_name);
        swap(left.tag_name, right.tag_name);
        swap(left.list_name, right.list_name);
        swap(left.list_index, right.list_index);
        swap(left.text, right.text);
    }

    ListNode(ListNode &&another) noexcept : ListNode() { swap(*this, another); }

    ListNode &operator=(ListNode another) {
        swap(*this, another);
        return *this;
    }

    std::wstring str() const override;
    ~ListNode() final = default;
};

class translation_unit {
    using wstring = std::wstring;
    std::filesystem::path file_name;
    std::vector<const Node *> nodes;

  public:
    translation_unit(const translation_unit &) = delete;
    translation_unit(translation_unit &&) = delete;
    ~translation_unit();

    explicit translation_unit(wstring file_name) : file_name(std::move(file_name)){};

    void add(const Node *node) noexcept;

    void set_file_name(const wstring &file_name) { this->file_name = file_name; }
    wstring get_file_name() noexcept { return file_name; }

    bool empty() noexcept { return nodes.empty(); }

    bool save() noexcept;
};

}// namespace simplexml

#endif//RWSIMPLIFIEDPATCHER__NODE_H
