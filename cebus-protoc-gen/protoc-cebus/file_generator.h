#pragma once

#include "message_generator.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/compiler/code_generator.h>

namespace protobuf::cebus
{

class FileGenerator
{
public:
    explicit FileGenerator(const google::protobuf::FileDescriptor* file);

    bool hasRouting() const;

    void generateHeader(google::protobuf::compiler::OutputDirectory* outputDir);
    void generateSource(google::protobuf::compiler::OutputDirectory* outputDir);

private:
    const google::protobuf::FileDescriptor* file;
    std::vector<std::unique_ptr<MessageGenerator>> messageGenerators;

    std::string outputFileName(std::string suffix) const;
};
}
