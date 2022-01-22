#include "field_generator.h"

#include <cebus.pb.h>

namespace protobuf::cebus {
static constexpr size_t MAX_ROUTING_MEMBERS = 10;

FieldGenerator::FieldGenerator(
    const google::protobuf::FieldDescriptor* descriptor)
    : descriptor(descriptor)
{
    variables["field_name"] = google::protobuf::compiler::c::FieldName(descriptor);
    variables["default"] = "CB_BINDING_KEY_ALL";
}

void FieldGenerator::generateStructMember(
    google::protobuf::io::Printer* printer) const
{
    printer->Print(variables, "const char *$field_name$;\n");
}

void FieldGenerator::generateStaticInit(google::protobuf::io::Printer* printer) const
{
    printer->Print(variables, "$default$");
}

void FieldGenerator::generateReadValue(google::protobuf::io::Printer* printer,
    const std::string& parameterName) const
{
    std::map<std::string, std::string> vars(variables);
    vars["parameter_name"] = parameterName;
    printer->Print(vars, "$parameter_name$->$field_name$");
}

void FieldGenerator::generateBindingKeyBuild(google::protobuf::io::Printer* printer,
    const std::string& parameterName) const
{
    std::map<std::string, std::string> vars(variables);
    switch (descriptor->type()) {
    case google::protobuf::FieldDescriptor::TYPE_BOOL:
        vars["type"] = "bool";
        break;
    case google::protobuf::FieldDescriptor::TYPE_FLOAT:
        vars["type"] = "float";
        break;
    case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
        vars["type"] = "double";
        break;
    case google::protobuf::FieldDescriptor::TYPE_FIXED32:
    case google::protobuf::FieldDescriptor::TYPE_UINT32:
        vars["type"] = "u32";
        break;
    case google::protobuf::FieldDescriptor::TYPE_FIXED64:
    case google::protobuf::FieldDescriptor::TYPE_INT64:
        vars["type"] = "u64";
        break;
    case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
    case google::protobuf::FieldDescriptor::TYPE_SINT32:
    case google::protobuf::FieldDescriptor::TYPE_INT32:
        vars["type"] = "i32";
        break;
    case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
    case google::protobuf::FieldDescriptor::TYPE_SINT64:
        vars["type"] = "i64";
        break;
    case google::protobuf::FieldDescriptor::TYPE_STRING:
        vars["type"] = "str";
        break;
    default:
        GenerationException::throwF("Unhandled field %s",
            descriptor->name().c_str());
    }

    printer->Print(vars, "cb_binding_key_builder_add_$type$(&__builder, ");
    generateReadValue(printer, parameterName);
    printer->Print(vars, ");\n");
}

FieldGeneratorMap::FieldGeneratorMap(
    const google::protobuf::Descriptor* descriptor)
    : descriptor(descriptor)
    , fields(createGenerators(descriptor))
{
}

std::vector<FieldGeneratorMap::RoutingField>
FieldGeneratorMap::createGenerators(
    const google::protobuf::Descriptor* descriptor)
{
    std::vector<RoutingField> fields;

    for (int i = 0; i < descriptor->field_count(); ++i) {
        const auto* field = descriptor->field(i);
        const auto& options = field->options();

        if (!options.HasExtension(routing_position))
            continue;

        auto routingPosition = field->options().GetExtension(routing_position);
        if (routingPosition <= 0 || routingPosition > MAX_ROUTING_MEMBERS) {
            GenerationException::throwF(
                "Invalid routing_position %d for field `%s` (%s) (must be [1; %zu])",
                routingPosition,
                field->name().c_str(),
                descriptor->full_name().c_str(),
                MAX_ROUTING_MEMBERS);
        }

        if (field->label() == google::protobuf::FieldDescriptor::LABEL_REPEATED) {
            GenerationException::throwF(
                "Routing field `%s` (%s) can not be a repeated-field",
                field->name().c_str(),
                descriptor->full_name().c_str());
        }

        auto type = field->type();
        if (!isRoutableField(field)) {
            GenerationException::throwF(
                "Routing field `%s` (%s) of type %s is not routable. Only primitive "
                "fields are currently routable",
                field->name().c_str(),
                descriptor->full_name().c_str(),
                field->type_name());
        }

        auto generator = std::make_unique<FieldGenerator>(field);

        fields.push_back(RoutingField {
            static_cast<size_t>(routingPosition), field, std::move(generator) });
    }

    std::sort(
        std::begin(fields), std::end(fields), [](const auto& lhs, const auto& rhs) {
            return lhs.position < rhs.position;
        });

    return fields;
}

bool FieldGeneratorMap::isRoutableField(
    const google::protobuf::FieldDescriptor* field)
{
    const auto type = field->type();

    if (type == google::protobuf::FieldDescriptor::TYPE_MESSAGE || type == google::protobuf::FieldDescriptor::TYPE_ENUM || type == google::protobuf::FieldDescriptor::TYPE_GROUP) {
        return false;
    }

    return true;
}
} // namespace protobuf::cebus
