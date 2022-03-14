#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/io/printer.h>
#include <protoc-c/c_helpers.h>

#include "generation_exception.h"
#include <cebus.pb.h>

namespace protobuf::cebus {
class MessageGenerator {
public:
    static std::unique_ptr<MessageGenerator> create(const google::protobuf::Descriptor* descriptor);

    void generateDescriptorDeclarations(
        google::protobuf::io::Printer* printer) const;

    void generateDescriptorDefinitions(google::protobuf::io::Printer* printer) const;

    void generateCommandOrEventHelperDeclarations(google::protobuf::io::Printer* printer) const;
    void generateCommandOrEventHelperDefinitions(google::protobuf::io::Printer* printer) const;

private:
    struct RoutingField {
        const google::protobuf::FieldDescriptor* descriptor;

        size_t fieldIndex;

        size_t routingPosition;
    };

    const google::protobuf::Descriptor* descriptor;
    std::vector<RoutingField> routingFields;

    explicit MessageGenerator(const google::protobuf::Descriptor* descriptor, std::vector<RoutingField> routingFields);

    static bool isRoutableField(const google::protobuf::FieldDescriptor* field);
    static std::vector<RoutingField> getRoutingFields(const google::protobuf::Descriptor* descriptor);
};

MessageGenerator::MessageGenerator(const google::protobuf::Descriptor* descriptor, std::vector<MessageGenerator::RoutingField> routingFields)
    : descriptor(descriptor)
    , routingFields(std::move(routingFields))
{
}

std::unique_ptr<MessageGenerator> MessageGenerator::create(const google::protobuf::Descriptor* descriptor)
{
    auto routingFields = getRoutingFields(descriptor);
    const auto hasRouting = routingFields.size() > 0;

    const auto& options = descriptor->options();

    // If we are routable, let's check if we have a message_type
    if (hasRouting) {
        if (!options.HasExtension(message_type)) {
            GenerationException::throwF("Message `%s` with routing fields should provide a `message_type`",
                descriptor->full_name().c_str());
        }
    } else {
        // This is not a bus message, skip it
        if (!options.HasExtension(message_type))
            return nullptr;
    }

    return std::unique_ptr<MessageGenerator>(new MessageGenerator(descriptor, std::move(routingFields)));
}

void MessageGenerator::generateDescriptorDeclarations(google::protobuf::io::Printer* printer) const
{
    printer->Print("extern const ProtobufCebusMessageDescriptor $name$__message_descriptor;\n",
        "name",
        google::protobuf::compiler::c::FullNameToLower(
            descriptor->full_name(), descriptor->file()));
}

void MessageGenerator::generateDescriptorDefinitions(google::protobuf::io::Printer* printer) const
{
    std::map<std::string, std::string> vars;

    const auto& options = descriptor->options();

    vars["fullname"] = descriptor->full_name();
    vars["classname"] = google::protobuf::compiler::c::FullNameToC(descriptor->full_name(), descriptor->file());
    vars["lcclassname"] = google::protobuf::compiler::c::FullNameToLower(descriptor->full_name(), descriptor->file());
    vars["shortname"] = google::protobuf::compiler::c::ToCamel(descriptor->name());
    vars["n_routing_fields"] = google::protobuf::compiler::c::SimpleItoa(routingFields.size());
    vars["namespace_name"] = options.GetExtension(namespace_);

    switch (options.GetExtension(message_type)) {
    case MessageType::Command:
        vars["message_type"] = "PROTOBUF_CEBUS_MESSAGE_TYPE_COMMAND";
        break;
    case MessageType::Event:
        vars["message_type"] = "PROTOBUF_CEBUS_MESSAGE_TYPE_EVENT";
        break;
    }

    // Generate routing field descriptors
    //
    if (!routingFields.empty()) {
        printer->Print(vars,
            "static const ProtobufCebusRoutingFieldDescriptor $lcclassname$__routing_field_descriptors[$n_routing_fields$] =\n"
            "{\n");
        printer->Indent();

        for (const auto& routingField : routingFields) {
            vars["routing_position"] = google::protobuf::compiler::c::SimpleItoa(routingField.routingPosition);
            vars["field_index"] = google::protobuf::compiler::c::SimpleItoa(routingField.fieldIndex);
            printer->Print("{\n");
            printer->Indent();
            printer->Print(vars,
                "&$lcclassname$__field_descriptors[$field_index$],\n"
                "$routing_position$\n");
            printer->Outdent();
            printer->Print("},\n");
        }
        printer->Outdent();
        printer->Print("};\n");
    }

    // Generate message descriptor

    printer->Print(vars,
        "const ProtobufCebusMessageDescriptor $lcclassname$__message_descriptor =\n"
        "{\n");
    printer->Indent();
    printer->Print(vars,
        "&$lcclassname$__descriptor,\n"
        "\"$namespace_name$\",\n"
        "$message_type$,\n"
        "$n_routing_fields$,\n");
    if (routingFields.empty())
        printer->Print("NULL,\n");
    else
        printer->Print(vars, "$lcclassname$__routing_field_descriptors,\n");
    printer->Print("0\n");
    printer->Outdent();

    printer->Print("};\n");
}

void MessageGenerator::generateCommandOrEventHelperDeclarations(google::protobuf::io::Printer* printer) const
{
    std::map<std::string, std::string> vars;

    vars["classname"] = google::protobuf::compiler::c::FullNameToC(descriptor->full_name(), descriptor->file());
    vars["lcclassname"] = google::protobuf::compiler::c::FullNameToLower(descriptor->full_name(), descriptor->file());
    const auto& options = descriptor->options();
    switch (options.GetExtension(message_type)) {
    case MessageType::Command:
        vars["ret_type"] = "ProtobufCebusCommand";
        vars["suffix"] = "command";
        break;
    case MessageType::Event:
        vars["ret_type"] = "ProtobufCebusEvent";
        vars["suffix"] = "event";
        break;
    }

    printer->Print(vars, "$ret_type$ $lcclassname$__$suffix$(const $classname$* message);");
}

void MessageGenerator::generateCommandOrEventHelperDefinitions(google::protobuf::io::Printer* printer) const
{
    std::map<std::string, std::string> vars;

    vars["classname"] = google::protobuf::compiler::c::FullNameToC(descriptor->full_name(), descriptor->file());
    vars["lcclassname"] = google::protobuf::compiler::c::FullNameToLower(descriptor->full_name(), descriptor->file());
    const auto& options = descriptor->options();
    switch (options.GetExtension(message_type)) {
    case MessageType::Command:
        vars["ret_type"] = "ProtobufCebusCommand";
        vars["suffix"] = "command";
        break;
    case MessageType::Event:
        vars["ret_type"] = "ProtobufCebusEvent";
        vars["suffix"] = "event";
        break;
    }

    printer->Print(
            vars,
            "$ret_type$ $lcclassname$__$suffix$(const $classname$* message)\n"
            "{\n"
            );
    printer->Indent();
    printer->Print(vars,
        "const $ret_type$ $suffix$ = { (const ProtobufCMessage *) message, &$lcclassname$__message_descriptor };\n"
        "return $suffix$;\n"
    );
    printer->Outdent();
    printer->Print("}\n");
}

bool MessageGenerator::isRoutableField(const google::protobuf::FieldDescriptor* field)
{
    const auto type = field->type();
    switch (type) {
    case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
    case google::protobuf::FieldDescriptor::TYPE_ENUM:
    case google::protobuf::FieldDescriptor::TYPE_GROUP:
        return false;
    default:
        return true;
    }
}

std::vector<MessageGenerator::RoutingField> MessageGenerator::getRoutingFields(const google::protobuf::Descriptor* descriptor)
{
    std::vector<RoutingField> fields;

    // Get fields with a `routing_position` attribute
    for (int i = 0; i < descriptor->field_count(); ++i) {
        const auto* field = descriptor->field(i);
        const auto& options = field->options();

        if (!options.HasExtension(routing_position))
            continue;

        auto routingPosition = field->options().GetExtension(routing_position);
        if (routingPosition <= 0) {
            GenerationException::throwF(
                "Invalid routing_position %d for field `%s` (%s) (must be >= 0)",
                routingPosition,
                field->name().c_str(),
                descriptor->full_name().c_str());
        }

        if (field->label() == google::protobuf::FieldDescriptor::LABEL_REPEATED) {
            GenerationException::throwF(
                "Routing field `%s` (%s) can not be a repeated-field",
                field->name().c_str(),
                descriptor->full_name().c_str());
        }

        if (!isRoutableField(field)) {
            GenerationException::throwF(
                "Routing field `%s` (%s) of type %s is not routable. Only primitive "
                "fields are currently routable",
                field->name().c_str(),
                descriptor->full_name().c_str(),
                field->type_name());
        }

        fields.push_back({ field, static_cast<size_t>(i), static_cast<size_t>(routingPosition) });
    }

    // Sort fields by their routing position
    std::sort(
        std::begin(fields), std::end(fields), [](const auto& lhs, const auto& rhs) {
            return lhs.routingPosition < rhs.routingPosition;
        });

    // Check that routing positions are contiguous
    if (!fields.empty()) {
        for (size_t i = 1; i < fields.size(); ++i) {
            if (fields[i].routingPosition != fields[i - 1].routingPosition + 1) {
                GenerationException::throwF(
                    "Routing field `%s` (`%s`) has a gap in routing position (%zu -> %zu)",
                    fields[i].descriptor->full_name().c_str(),
                    descriptor->full_name().c_str(),
                    fields[i].routingPosition,
                    fields[i - 1].routingPosition);
            }
        }
    }

    return fields;
}

class FileGenerator {
public:
    explicit FileGenerator(const google::protobuf::FileDescriptor* file);

    void generateHeader(
        google::protobuf::compiler::OutputDirectory* outputDir);

    void generateSource(google::protobuf::compiler::OutputDirectory* outputDir);

private:
    const google::protobuf::FileDescriptor* file;

    std::vector<std::unique_ptr<MessageGenerator>> messageGenerators;
};

FileGenerator::FileGenerator(const google::protobuf::FileDescriptor* file)
    : file(file)
{
    for (int i = 0; i < file->message_type_count(); ++i) {
        auto generator = MessageGenerator::create(file->message_type(i));
        if (generator != nullptr)
            messageGenerators.push_back(std::move(generator));
    }
}

void FileGenerator::generateHeader(
    google::protobuf::compiler::OutputDirectory* outputDir)
{
    if (messageGenerators.empty())
        return;

    std::string baseName = google::protobuf::compiler::c::StripProto(file->name());
    baseName.append(".pb-c");

    // Generate cebus-specific protobuf headers
    {
        std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> output(
            outputDir->OpenForInsert(baseName + ".h", "includes"));
        google::protobuf::io::Printer printer(output.get(), '$');

        printer.Print("#include <protobuf-cebus/protobuf-cebus.h>");
    }

    // Generate descriptors for messages
    {
        std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> output(
            outputDir->OpenForInsert(baseName + ".h", "global_scope"));
        google::protobuf::io::Printer printer(output.get(), '$');

        for (const auto& messageGenerator : messageGenerators) {
            messageGenerator->generateDescriptorDeclarations(&printer);
            messageGenerator->generateCommandOrEventHelperDeclarations(&printer);
        }
    }
}

void FileGenerator::generateSource(google::protobuf::compiler::OutputDirectory* outputDir)
{
    std::string baseName = google::protobuf::compiler::c::StripProto(file->name());
    baseName.append(".pb-c");
    // Generate descriptors for messages
    {
        std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> output(
            outputDir->OpenForInsert(baseName + ".c", "global_scope"));
        google::protobuf::io::Printer printer(output.get(), '$');

        for (const auto& messageGenerator : messageGenerators) {
            messageGenerator->generateDescriptorDefinitions(&printer);
            messageGenerator->generateCommandOrEventHelperDefinitions(&printer);
        }
    }
}

class Generator : public google::protobuf::compiler::CodeGenerator {
public:
    bool Generate(const google::protobuf::FileDescriptor* file,
        const std::string& parameter,
        google::protobuf::compiler::OutputDirectory* outputDir,
        std::string* error) const override
    {
        FileGenerator generator(file);

        // Generate header
        generator.generateHeader(outputDir);

        // Generate source
        generator.generateSource(outputDir);

        return true;
    }
};
}

int main(int argc, char* argv[])
{
    protobuf::cebus::Generator generator;
    return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
