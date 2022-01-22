#include <string>

#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/compiler/command_line_interface.h>

#include <protoc-c/c_generator.h>

int main(int argc, char* argv[]) {
  google::protobuf::compiler::c::CGenerator c_generator;
  return google::protobuf::compiler::PluginMain(argc, argv, &c_generator);
}
