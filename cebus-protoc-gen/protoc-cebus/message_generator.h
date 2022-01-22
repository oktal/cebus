#pragma once

#include "field_generator.h"

#include <google/protobuf/descriptor.h>

namespace protobuf::cebus {
class MessageGenerator {
public:
    explicit MessageGenerator(const google::protobuf::Descriptor* descriptor);

    bool hasRouting() const;

    void generateStructDefinition(google::protobuf::io::Printer* printer);
    void generateFunctionDeclarations(google::protobuf::io::Printer* printer);
    void generateFunctionDefinitions(google::protobuf::io::Printer* printer);

private:
    const google::protobuf::Descriptor* descriptor;
    FieldGeneratorMap fieldGenerators;
};
}
