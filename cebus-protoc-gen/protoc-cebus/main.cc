#include "file_generator.h"
#include "generation_exception.h"

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/compiler/plugin.h>

namespace protobuf::cebus
{
    class Generator : public google::protobuf::compiler::CodeGenerator
    {
    public:
        bool Generate(const google::protobuf::FileDescriptor* file,
                      const std::string& parameter,
                      google::protobuf::compiler::OutputDirectory* outputDir,
                      std::string* error) const
        {
            try
            {
                FileGenerator generator(file);
                if (generator.hasRouting())
                {
                    generator.generateHeader(outputDir);
                    generator.generateSource(outputDir);
                }

                return true;
            }
            catch (const GenerationException& e)
            {
                *error = e.what();
                return false;
            }
        }
    };
}

int main(int argc, char* argv[]) {
    protobuf::cebus::Generator generator;
    return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
