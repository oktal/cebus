#include "file_generator.h"


namespace protobuf::cebus
{
    FileGenerator::FileGenerator(const google::protobuf::FileDescriptor* file)
        : file(file)
    {
        for (int i = 0; i < file->message_type_count(); i++)
        {
            auto message = file->message_type(i);
            messageGenerators.push_back(std::make_unique<MessageGenerator>(message));
        }
    }

    bool FileGenerator::hasRouting() const
    {
        return std::any_of(std::begin(messageGenerators), std::end(messageGenerators), [](const auto& generator) {
            return generator->hasRouting();
        });
    }

    void FileGenerator::generateHeader(google::protobuf::compiler::OutputDirectory* outputDir)
    {
        std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> output(
                outputDir->Open(outputFileName(".h")));

        google::protobuf::io::Printer printer(output.get(), '$');

        printer.Print(
                "#include \"$basename$.pb-c.h\"\n"
                "#include <cebus/binding_key.h>\n\n",
                "basename", google::protobuf::compiler::c::StripProto(file->name()));

        for (const auto& generator: messageGenerators)
        {
            generator->generateStructDefinition(&printer);
            generator->generateFunctionDeclarations(&printer);
        }
    }

    void FileGenerator::generateSource(google::protobuf::compiler::OutputDirectory* outputDir)
    {
        std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> output(
                outputDir->Open(outputFileName(".c")));
        google::protobuf::io::Printer printer(output.get(), '$');

        std::map<std::string, std::string> vars;
        vars["include_file"] = outputFileName(".h");

        printer.Print(vars, "#include \"$include_file$\"\n\n");

        for (const auto& generator: messageGenerators)
        {
            generator->generateFunctionDefinitions(&printer);
        }
    }

    std::string FileGenerator::outputFileName(std::string suffix) const
    {
        auto baseName = google::protobuf::compiler::c::StripProto(file->name());
        baseName.append(".pb-cb");
        baseName.append(suffix);
        return baseName;
    }

}
