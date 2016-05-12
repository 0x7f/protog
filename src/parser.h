#pragma once

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/compiler/parser.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

using google::protobuf::Descriptor;
using google::protobuf::DescriptorPool;
using google::protobuf::FieldDescriptor;
using google::protobuf::FileDescriptor;
using google::protobuf::FileDescriptorProto;
using google::protobuf::compiler::Parser;
using google::protobuf::io::FileInputStream;
using google::protobuf::io::Tokenizer;
using google::protobuf::io::ZeroCopyInputStream;

namespace protog {

enum class NodeType : int {
    BOOL = 1,
    LONG = 2,
    DOUBLE = 3,
    STRING = 4,
    OUTSIDE_OBJECT = 5,
    INSIDE_OBJECT = 6,
    ARRAY = 7,
};

static NodeType getNodeTypeForProtoType(FieldDescriptor::Type type) {
    switch (type) {
        case FieldDescriptor::TYPE_BOOL:
            return NodeType::BOOL;
        case FieldDescriptor::TYPE_INT32:
        case FieldDescriptor::TYPE_INT64:
        case FieldDescriptor::TYPE_UINT32:
        case FieldDescriptor::TYPE_UINT64: // TODO: test for data loss!
        case FieldDescriptor::TYPE_FIXED32:
        case FieldDescriptor::TYPE_FIXED64:
        case FieldDescriptor::TYPE_SFIXED32:
        case FieldDescriptor::TYPE_SFIXED64:
        case FieldDescriptor::TYPE_SINT32:
        case FieldDescriptor::TYPE_SINT64:
            return NodeType::LONG;
        case FieldDescriptor::TYPE_FLOAT:
        case FieldDescriptor::TYPE_DOUBLE:
            return NodeType::DOUBLE;
        case FieldDescriptor::TYPE_STRING:
            return NodeType::STRING;
        case FieldDescriptor::TYPE_MESSAGE:
            return NodeType::OUTSIDE_OBJECT;
        case FieldDescriptor::TYPE_ENUM:
            return NodeType::LONG;
        case FieldDescriptor::TYPE_BYTES: // TODO
        default:
            throw std::runtime_error("Unsupported protobuf type " + std::to_string(static_cast<int>(type)));
    }
}

static std::string getTypeNameForFieldDesc(const FieldDescriptor &fieldDesc) {
    return fieldDesc.type() == FieldDescriptor::TYPE_MESSAGE ? fieldDesc.message_type()->name()
                                                             : fieldDesc.type_name();
}

static std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

static std::vector<std::string> split(const std::string& str, const char delim, bool include_empty = false) {
    std::vector<std::string> result;
    std::stringstream ss{str};
    std::string item;
    while(std::getline(ss, item, delim)) {
        if (!item.empty() || include_empty) {
            result.push_back(item);
        }
    }
    return result;
}

struct Node {
    // structure
    Node *parent;
    std::vector<Node*> children;

    // node info
    NodeType type;
    int state;
    std::string name;
    std::string full_name; // including path
    std::string type_name;
    const Descriptor *desc;
    const FieldDescriptor *field;

    ~Node() {
        for(auto& child : children) {
            delete child;
        }
    }
};

struct Graph {
    std::string fname;
    std::string msgName;
    DescriptorPool pool;
    const FileDescriptor *fileDesc;
    const Descriptor *msgDesc;
    int stateCounter = 0;
    Node root;
    std::vector<Node *> all_nodes;
    std::vector<Node *> null_nodes;
    std::vector<Node *> bool_nodes;
    std::vector<Node *> long_nodes;
    std::vector<Node *> double_nodes;
    std::vector<Node *> string_nodes;
    std::vector<Node *> object_nodes;
    std::vector<Node *> key_nodes;
    std::vector<Node *> array_nodes;

    Graph(const std::string &fname, const std::string &msgName) : fname(fname), msgName(msgName) {
        root.state = ++stateCounter;

        int fd = open(fname.c_str(), O_RDONLY);
        if (fd < 0) {
            throw std::runtime_error("Unable to open proto file " + fname);
        }

        ZeroCopyInputStream *input = new FileInputStream(fd);
        Tokenizer tokenizer{input, nullptr};

        Parser parser;
        FileDescriptorProto protoDesc;
        if (!parser.Parse(&tokenizer, &protoDesc)) {
            throw std::runtime_error("Unable to parse proto file " + fname);
        }

        protoDesc.set_name("XXX"); // TODO: what is this name for?
        protoDesc.CheckInitialized();

#if LOG_TRACE == 1
        printf("Loaded file with following messages:\n");
        for (int i = 0; i < protoDesc.message_type_size(); ++i) {
            printf(">> %s.%s\n", protoDesc.package().c_str(), protoDesc.message_type(i).name().c_str());
        }
        printf("\n");
#endif

        fileDesc = pool.BuildFile(protoDesc);
        if (!fileDesc) {
            throw std::runtime_error("Unable to load proto file " + fname);
        }

        msgDesc = pool.FindMessageTypeByName(msgName);
        if (!msgDesc) {
            throw std::runtime_error("Unable to find message type " + msgName);
        }
    }

    void parseMessageDesc() {
        const auto &desc = *msgDesc;

        root.parent = nullptr;
        root.name = ".";
        root.full_name = ".";
        root.type_name = desc.name();
        root.type = NodeType::INSIDE_OBJECT;
        root.desc = &desc;
        root.field = nullptr;
        addNodeToTypeLists(root);

        parseMessageDescRec(desc, root);
    }

    void parseMessageDescRec(const Descriptor &desc, Node &node) {
        for (int f = 0; f < desc.field_count(); ++f) {
            const FieldDescriptor &fieldDesc = *desc.field(f);
            const auto type = getNodeTypeForProtoType(fieldDesc.type());
            const auto isRepeated = fieldDesc.is_repeated();

            Node &child = addChild(node);
            child.name = fieldDesc.name();
            child.full_name = node.full_name + child.name;
            child.field = &fieldDesc;
            child.desc = &desc;

            if (!isRepeated) {
                child.type = type;
                child.type_name = getTypeNameForFieldDesc(fieldDesc);
                addNodeToTypeLists(child);
                if (type == NodeType::OUTSIDE_OBJECT) {
                    Node& objChild = injectObjectNode(desc, fieldDesc, child);
                    parseMessageDescRec(*fieldDesc.message_type(), objChild);
                }
            } else {
                child.type = NodeType::ARRAY;
                child.type_name = "[" + getTypeNameForFieldDesc(fieldDesc) + "]";
                addNodeToTypeLists(child);
                Node& arrChild = injectArrayNode(desc, fieldDesc, type, child);
                if (type == NodeType::OUTSIDE_OBJECT) {
                    Node& objChild = injectObjectNode(desc, fieldDesc, arrChild);
                    parseMessageDescRec(*fieldDesc.message_type(), objChild);
                }
            }
        }
    }

    Node& injectArrayNode(const Descriptor &desc, const FieldDescriptor& fieldDesc, NodeType type, Node& node) {
        Node &arrChild = addChild(node);
        arrChild.name = fieldDesc.name();
        arrChild.full_name = node.full_name + "[]";
        arrChild.type = type; // specifies what type to expect in array
        arrChild.type_name = getTypeNameForFieldDesc(fieldDesc);
        arrChild.field = &fieldDesc;
        arrChild.desc = &desc;
        addNodeToTypeLists(arrChild);
        return arrChild;
    }

    Node& injectObjectNode(const Descriptor &desc, const FieldDescriptor& fieldDesc, Node& keyNode) {
        Node &objNode = addChild(keyNode);
        objNode.name = keyNode.name;
        objNode.full_name = keyNode.full_name + ".";
        objNode.type = NodeType::INSIDE_OBJECT;
        objNode.type_name = keyNode.type_name;
        objNode.field = &fieldDesc;
        objNode.desc = &desc;
        addNodeToTypeLists(objNode);
        return objNode;
    }

    void addNodeToTypeLists(Node &node) {
        assert(node.state);
        all_nodes.push_back(&node);
        if (node.field && (node.field->is_optional() || node.field->is_repeated())) {
            null_nodes.push_back(&node);
        }
        switch (node.type) {
            case NodeType::BOOL:
                bool_nodes.push_back(&node);
                // TODO: hack to allow 1/0 as true/false
                long_nodes.push_back(&node);
                break;
            case NodeType::LONG:
                long_nodes.push_back(&node);
                break;
            case NodeType::DOUBLE:
                double_nodes.push_back(&node);
                // TODO: hack to allow ints as doubles. shall we switch to number anyways?
                long_nodes.push_back(&node);
                break;
            case NodeType::STRING:
                string_nodes.push_back(&node);
                break;
            case NodeType::INSIDE_OBJECT:
                object_nodes.push_back(&node);
                break;
            case NodeType::OUTSIDE_OBJECT:
                key_nodes.push_back(&node);
                break;
            case NodeType::ARRAY:
                array_nodes.push_back(&node);
                break;
        }
    }

    Node &addChild(Node &node) {
        node.children.push_back(new Node());
        auto& child = *node.children.back();
        child.parent = &node;
        child.state = ++stateCounter;
        return child;
    }

    void printDebug(FILE* file) const {
        printDebugRec(file, root, 0);
    }

    void printDebugRec(FILE* file, const Node& node, int depth) const {
        printf(">> %s (type=%s, type_id=%d, state=%d\n", node.full_name.c_str(), node.type_name.c_str(), node.type, node.state);
        for (const auto &child : node.children) {
            printDebugRec(file, *child, depth + 1);
        }
    }
};

} // namespace protog
