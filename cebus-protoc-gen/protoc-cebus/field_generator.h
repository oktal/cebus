#pragma once

#include "generation_exception.h"

#include <google/protobuf/descriptor.h>
#include <protoc-c/c_helpers.h>

namespace protobuf::cebus {
class FieldGenerator {
public:
    explicit FieldGenerator(const google::protobuf::FieldDescriptor* descriptor);

    void generateStructMember(google::protobuf::io::Printer* printer) const;
    void generateStaticInit(google::protobuf::io::Printer* printer) const;
    void generateReadValue(google::protobuf::io::Printer* printer, const std::string& parameterName) const;
    void generateBindingKeyBuild(google::protobuf::io::Printer* printer, const std::string& parameterName) const;

private:
    const google::protobuf::FieldDescriptor* descriptor;
    std::map<std::string, std::string> variables;
};

class FieldGeneratorMap {
public:
    explicit FieldGeneratorMap(const google::protobuf::Descriptor* descriptor);

    const std::pair<const google::protobuf::FieldDescriptor*, FieldGenerator*> get(size_t i) const
    {
        if (i >= fields.size())
            return std::make_pair(nullptr, nullptr);

        const auto& field = fields[i];
        return std::make_pair(field.descriptor, field.generator.get());
    }

    bool empty() const
    {
        return fields.empty();
    }

    size_t count() const
    {
        return fields.size();
    }

private:
    struct RoutingField {
        size_t position;
        const google::protobuf::FieldDescriptor* descriptor;
        std::unique_ptr<FieldGenerator> generator;
    };

    const google::protobuf::Descriptor* descriptor;
    std::vector<RoutingField> fields;

    static std::vector<RoutingField> createGenerators(const google::protobuf::Descriptor* descriptor);
    static bool isRoutableField(const google::protobuf::FieldDescriptor* field);
};
}
