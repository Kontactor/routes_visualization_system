// json_builder.cpp

#include "json_builder.h"

namespace json {

Builder::KeyContext Builder::Key(std::string key)
{
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Key can only be added to a dictionary");
    }
    if (current_key_.has_value()) {
        throw std::logic_error("Key can`t be added after another key");
    }
    current_key_ = key;
    return BaseContext{*this};
}

Builder::BaseContext Builder::Value(Node::Value value)
{
    if (!nodes_stack_.empty()) {
        if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->AsArray().emplace_back(std::move(value));
        } else if (nodes_stack_.back()->IsDict()) {
            if (!current_key_.has_value()) {
                throw std::logic_error(
                    "Dictionary must have a key before a value");
            }
            nodes_stack_.back()->AsDict().emplace(current_key_.value(),
                                                  std::move(value));
            current_key_.reset();
        } else {
            throw std::logic_error(
                "Value can only be added to an array or dictionary");
        }
    } else {
        if (root_ != nullptr) {
            throw std::logic_error("JSON structure built already");
        }
        root_ = Node(std::move(value));
    }

    return *this;
}

Builder::DictItemContext Builder::StartDict()
{
    if (!nodes_stack_.empty()) {
        if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->AsArray().emplace_back(Node(Dict{}));
            nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
        } else if (nodes_stack_.back()->IsDict()) {
            if (!current_key_.has_value()) {
                throw std::logic_error(
                    "Dictionary must have a key before starting a nested "
                    "dictionary");
            }
            nodes_stack_.back()->AsDict().emplace(current_key_.value(),
                                                  Node(Dict{}));
            nodes_stack_.push_back(
                &nodes_stack_.back()->AsDict().at(current_key_.value()));
            current_key_.reset();
        } else {
            throw std::logic_error(
                "Dictionary can only be started after constructor, in an "
                "array or another dictionary");
        }
    } else {
        root_ = Node(Dict{});
        nodes_stack_.push_back(&root_);
    }
    return BaseContext{*this};
}

Builder::ArrayItemContext Builder::StartArray()
{
    if (!nodes_stack_.empty()) {
        if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->AsArray().emplace_back(Node(Array{}));
            nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
        } else if (nodes_stack_.back()->IsDict()) {
            if (!current_key_.has_value()) {
                throw std::logic_error(
                    "Dictionary must have a key before starting a nested "
                    "dictionary");
            }
            nodes_stack_.back()->AsDict().emplace(current_key_.value(),
                                                  Node(Array{}));
            nodes_stack_.push_back(
                &nodes_stack_.back()->AsDict().at(current_key_.value()));
            current_key_.reset();
        } else {
            throw std::logic_error(
                "Array can only be started after constructor, in an "
                "array or another dictionary");
        }
    } else {
        root_ = Node(Array{});
        nodes_stack_.push_back(&root_);
    }
    return BaseContext{*this};
}

Builder::BaseContext Builder::EndDict()
{
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
        throw std::logic_error("No dictionary to end");
    }
    nodes_stack_.pop_back();
    return *this;
}

Builder::BaseContext Builder::EndArray()
{
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("No array to end");
    }
    nodes_stack_.pop_back();
    return *this;
}

json::Node Builder::Build()
{
    if (!nodes_stack_.empty() || root_ == nullptr) {
        throw std::logic_error("Incomplete JSON structure");
    }
    return std::move(root_);
}

} // namespace json