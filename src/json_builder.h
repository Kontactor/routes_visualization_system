// json_builder.h

#pragma once

#include "json.h"

#include <optional>
#include <string>
#include <vector>

namespace json {

class Builder {
    class BaseContext;
    class KeyContext;
    class DictItemContext;
    class ArrayItemContext;

public:
    Builder() = default;

    KeyContext Key(std::string key);
    BaseContext Value(Node::Value value);

    DictItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();

    json::Node Build();

private:
    json::Node root_ = nullptr;
    std::vector<json::Node*> nodes_stack_;
    std::optional<std::string> current_key_;

    class BaseContext {
    public:
        BaseContext(Builder& builder)
            : builder_(builder)
        {
        }

        KeyContext Key(std::string key)
        {
            return builder_.Key(std::move(key));
        }

        BaseContext Value(Node::Value value)
        {
            return builder_.Value(std::move(value));
        }

        DictItemContext StartDict()
        {
            return builder_.StartDict();
        }

        ArrayItemContext StartArray()
        {
            return builder_.StartArray();
        }

        BaseContext EndDict()
        {
            return builder_.EndDict();
        }

        BaseContext EndArray()
        {
            return builder_.EndArray();
        }

        json::Node Build()
        {
            return builder_.Build();
        }

    private:
        Builder& builder_;
    };

    class DictItemContext : public BaseContext {
    public:
        DictItemContext(BaseContext base)
            : BaseContext(base)
        {
        }

        BaseContext Value(Node::Value value) = delete;
        DictItemContext StartDict() = delete;
        ArrayItemContext StartArray() = delete;
        BaseContext EndArray() = delete;
        json::Node Build() = delete;
    };

    class ArrayItemContext : public BaseContext {
    public:
        ArrayItemContext(BaseContext base)
            : BaseContext(base)
        {
        }

        KeyContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        json::Node Build() = delete;

        ArrayItemContext Value(Node::Value value)
        {
            return BaseContext::Value(std::move(value));
        }
    };

    class KeyContext : public BaseContext {
    public:
        KeyContext(BaseContext base)
            : BaseContext(base)
        {
        }

        KeyContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        BaseContext EndArray() = delete;
        json::Node Build() = delete;

        DictItemContext Value(Node::Value value)
        {
            return BaseContext::Value(std::move(value));
        }
    };
};

} // namespace json