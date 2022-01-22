#include "message_generator.h"

namespace protobuf::cebus {
MessageGenerator::MessageGenerator(const google::protobuf::Descriptor* descriptor)
    : descriptor(descriptor)
    , fieldGenerators(descriptor)
{
}

bool MessageGenerator::hasRouting() const
{
    return !fieldGenerators.empty();
}

void MessageGenerator::generateStructDefinition(google::protobuf::io::Printer* printer)
{
    std::map<std::string, std::string> vars;
    std::string baseName = descriptor->full_name();
    baseName.append("Binding");

    vars["classname"] = google::protobuf::compiler::c::FullNameToC(baseName, descriptor->file());
    vars["ucclassname"] = google::protobuf::compiler::c::FullNameToUpper(baseName, descriptor->file());

    // Generate the special Binding struct definition
    printer->Print(vars,
        "struct $classname$\n"
        "{\n");

    // Generate fields for the Binding struct based on the `routing_position` field attribute
    printer->Indent();

    for (int i = 0; i < fieldGenerators.count(); i++) {
        auto [field, generator] = fieldGenerators.get(i);

        if (!generator) {
            GenerationException::throwF("Failed to retrieve field information at index %d (%s)",
                i, descriptor->name().c_str());
        }

        google::protobuf::SourceLocation loc;
        if (field->GetSourceLocation(&loc)) {
            google::protobuf::compiler::c::PrintComment(printer, loc.leading_comments);
            google::protobuf::compiler::c::PrintComment(printer, loc.trailing_comments);
        }

        generator->generateStructMember(printer);
    }

    printer->Outdent();
    printer->Print(vars, "};\n\n");

    // Generate special `__INIT` helper macro for the binding struct
    printer->Print(vars,
        "#define $ucclassname$__INIT \\\n"
        " { ");

    for (int i = 0, j = 0; i < fieldGenerators.count(); i++) {
        auto [field, generator] = fieldGenerators.get(i);

        if (!generator) {
            GenerationException::throwF("Failed to retrieve field information at index %d (%s)",
                i, descriptor->name().c_str());
        }

        if (j > 0)
            printer->Print(", ");

        generator->generateStaticInit(printer);
        ++j;
    }

    printer->Print(vars, " }\n\n");

    // Generate typedef for binding struct
    printer->Print(vars, "typedef struct $classname$ $classname$;\n\n");
}

void MessageGenerator::generateFunctionDeclarations(google::protobuf::io::Printer* printer)
{
    std::map<std::string, std::string> vars;
    std::string baseName = descriptor->full_name();

    std::string className(baseName);
    className.append("Binding");

    vars["basename"] = google::protobuf::compiler::c::FullNameToC(baseName, descriptor->file());
    vars["lcbasename"] = google::protobuf::compiler::c::FullNameToLower(baseName, descriptor->file());

    vars["classname"] = google::protobuf::compiler::c::FullNameToC(className, descriptor->file());
    vars["lcclassname"] = google::protobuf::compiler::c::FullNameToLower(className, descriptor->file());

    // Generate `__init` function
    printer->Print(vars, "void $lcclassname$__init($classname$ *binding);\n");

    // Generate function to retrieve the binding key from the binding
    printer->Print(vars, "cb_binding_key $lcclassname$__key(const $classname$ *binding);\n");

    // Generate function to retrieve the routing key from the message
    printer->Print(vars, "cb_binding_key $lcbasename$__key(const $basename$ *message);\n");
}

void MessageGenerator::generateFunctionDefinitions(google::protobuf::io::Printer* printer)
{
    std::map<std::string, std::string> vars;
    std::string baseName = descriptor->full_name();

    std::string className(baseName);
    className.append("Binding");

    vars["basename"] = google::protobuf::compiler::c::FullNameToC(baseName, descriptor->file());
    vars["lcbasename"] = google::protobuf::compiler::c::FullNameToLower(baseName, descriptor->file());

    vars["classname"] = google::protobuf::compiler::c::FullNameToC(className, descriptor->file());
    vars["lcclassname"] = google::protobuf::compiler::c::FullNameToLower(className, descriptor->file());
    vars["ucclassname"] = google::protobuf::compiler::c::FullNameToUpper(className, descriptor->file());

    // Generate `__init` function
    printer->Print(vars,
        "void $lcclassname$__init($classname$ *binding)\n"
        "{\n"
        "  static const $classname$ init_value = $ucclassname$__INIT;\n"
        "  *binding = init_value;\n"
        "}\n\n");

    // Generate function to retrieve the binding key from the binding
    printer->Print(vars,
        "cb_binding_key $lcclassname$__key(const $classname$ *binding)\n"
        "{\n");

    printer->Indent();
    printer->Print("const char* __fragments[] = {\n");
    printer->Indent();
    size_t count = 0;
    for (int i = 0; i < fieldGenerators.count(); i++) {
        auto [_, generator] = fieldGenerators.get(i);
        if (!generator) {
            GenerationException::throwF("Failed to retrieve field information at index %d (%s)",
                i, descriptor->name().c_str());
        }

        if (count > 0)
            printer->Print(", ");

        generator->generateReadValue(printer, "binding");
        ++count;
    }
    printer->Outdent();
    printer->Print("\n};\n");

    vars["fragment_count"] = std::to_string(count);

    printer->Print(vars, "return cb_binding_key_from_fragments(__fragments, $fragment_count$);\n");
    printer->Outdent();
    printer->Print(vars, "}\n\n");

    // Generate function to retrieve the routing key from the message
    printer->Print(vars,
        "cb_binding_key $lcbasename$__key(const $basename$ *message)\n"
        "{\n");
    printer->Indent();
    printer->Print(vars,
        "cb_binding_key_builder __builder = cb_binding_key_builder_with_capacity($fragment_count$);\n");
    for (int i = 0; i < descriptor->field_count(); i++) {
        auto [_, generator] = fieldGenerators.get(i);
        if (!generator) {
            GenerationException::throwF("Failed to retrieve field information at index %d (%s)",
                i, descriptor->name().c_str());
        }
        generator->generateBindingKeyBuild(printer, "message");
    }
    printer->Print(vars, "return cb_binding_key_build(&__builder);\n");
    printer->Outdent();
    printer->Print(vars, "}\n\n");
}

}
