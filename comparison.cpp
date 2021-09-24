#include "comparison.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

using namespace std;

string Comparison::Item::message() const
{
    string msg;

    switch (type)
    {
    case Enum_Value_Name_Changed:
        msg = "Value name changed";
        break;
    case Enum_Value_Id_Changed:
        msg = "Value ID changed";
        break;
    case Enum_Value_Added:
        msg = "Value added";
        break;
    case Enum_Value_Removed:
        msg = "Value removed";
        break;
    case Message_Field_Name_Changed:
        msg = "Name changed";
        break;
    case Message_Field_Id_Changed:
        msg = "ID changed";
        break;
    case Message_Field_Label_Changed:
        msg = "Label changed";
        break;
    case Message_Field_Type_Changed:
        msg = "Type changed";
        break;
    case Message_Field_Default_Value_Changed:
        msg = "Default value changed";
        break;
    case Message_Field_Added:
        msg = "Field added";
        break;
    case Message_Field_Removed:
        msg = "Field removed";
        break;
    case File_Message_Added:
        msg = "Message added";
        break;
    case File_Message_Removed:
        msg = "Message removed";
        break;
    case File_Enum_Added:
        msg = "Enum added";
        break;
    case File_Enum_Removed:
        msg = "Enum removed";
        break;
    case Name_Missing:
        msg = "Name missing";
        break;
    case Optional_Message_Field_Added:
        msg = "Optional_Field_added";
        break;
    case Optional_Message_Field_Removed:
        msg = "Optional_Field_removed";
        break;
    case File_Service_Added:
        msg = "File Service Added";
        break;
    case File_Service_Removed:
        msg = "File Service Removed";
        break;
    case Optional_InputMessage_Field_Added:
        msg = "Optional_InputField_added";
        break;
    case Optional_InputMessage_Field_Removed:
        msg = "Optional_InputField_removed";
        break;
    case Optional_OutputMessage_Field_Added:
        msg = "Optional_OutputField_added";
        break;
    case Optional_OutputMessage_Field_Removed:
        msg = "Optional_OutputField_removed";
        break;
    default:
        msg = "?";
        return msg;
    }

    msg += ": " + a + " -> " + b;

    return msg;
}

string Comparison::Section::message() const
{
    ostringstream msg;

    switch(type)
    {
    case Root_Section:
        msg << "/";
        break;
    case Message_Comparison:
        msg << "Comparing messages: " << a << " -> " << b;
        break;
    case Message_Field_Comparison:
        msg << "Comparing fields: " << a << " -> " << b;
        break;
    case Enum_Comparison:
        msg << "Comparing enums: " << a << " -> " << b;
        break;
    case Enum_Value_Comparison:
        msg << "Comparing enum values: " << a << " -> " << b;
        break;
    default:
        msg << "?";
    }

    return msg.str();
}

static
void print_field(const FieldDescriptor * field)
{
    cout << field->number() << " = " << field->full_name()
         << " " << field->type_name()
         << " " << field->label();

    if (field->has_default_value())
    {
        cout << " (";
        switch(field->cpp_type())
        {
        case FieldDescriptor::CPPTYPE_INT32:
            cout << field->default_value_int32(); break;
        case FieldDescriptor::CPPTYPE_INT64:
            cout << field->default_value_int64(); break;
        case FieldDescriptor::CPPTYPE_UINT32:
            cout << field->default_value_uint32(); break;
        case FieldDescriptor::CPPTYPE_UINT64:
            cout << field->default_value_uint64(); break;
        case FieldDescriptor::CPPTYPE_FLOAT:
            cout << field->default_value_float(); break;
        case FieldDescriptor::CPPTYPE_DOUBLE:
            cout << field->default_value_double(); break;
        case FieldDescriptor::CPPTYPE_BOOL:
            cout << field->default_value_bool(); break;
        case FieldDescriptor::CPPTYPE_STRING:
            cout << field->default_value_string(); break;
        case FieldDescriptor::CPPTYPE_ENUM:
            cout << field->default_value_enum()->name(); break;
        }
        cout << " )";
    }
}

void Comparison::Section::print(int level)
{
    cout << string(level*2, ' ') << message() << endl;

    ++level;

    string subprefix(level*2, ' ');

    for (auto & note : notes)
    {
        cout << subprefix << note << endl;
    }

    for (auto & item : items)
    {
        cout << subprefix << "* " << item.message() << endl;
    }

    for (auto & subsection : subsections)
    {
        subsection.print(level);
    }
}

Comparison::Comparison(const Options & options):
    options(options)
{}

bool Comparison::compare_default_value(const FieldDescriptor * field1, const FieldDescriptor * field2)
{
    if (field1->has_default_value() != field2->has_default_value())
        return false;

    if (!field1->has_default_value())
        return true;

    if (field1->cpp_type() != field2->cpp_type())
        return false;

    switch(field1->cpp_type())
    {
    case FieldDescriptor::CPPTYPE_INT32:
        return field1->default_value_int32() == field2->default_value_int32(); break;
    case FieldDescriptor::CPPTYPE_INT64:
        return field1->default_value_int64() == field2->default_value_int64(); break;
    case FieldDescriptor::CPPTYPE_UINT32:
        return field1->default_value_uint32() == field1->default_value_uint32(); break;
    case FieldDescriptor::CPPTYPE_UINT64:
        return field1->default_value_uint64() == field2->default_value_uint64(); break;
    case FieldDescriptor::CPPTYPE_FLOAT:
        return field1->default_value_float() == field2->default_value_float(); break;
    case FieldDescriptor::CPPTYPE_DOUBLE:
        return field1->default_value_double() == field1->default_value_double(); break;
    case FieldDescriptor::CPPTYPE_BOOL:
        return field1->default_value_bool() == field1->default_value_bool(); break;
    case FieldDescriptor::CPPTYPE_STRING:
        return field1->default_value_string() == field1->default_value_string(); break;
    case FieldDescriptor::CPPTYPE_ENUM:
        return field1->default_value_enum()->number() == field1->default_value_enum()->number(); break;
    default:
        return false;
    }
}

Comparison::Section * Comparison::compare(const EnumDescriptor * enum1, const EnumDescriptor * enum2)
{
    string key = enum1->full_name() + ":" + enum2->full_name();
    if (compared.count(key))
        return compared.at(key);

    auto & section = root.add_subsection(Enum_Comparison, enum1->full_name(), enum2->full_name());
    compared.emplace(key, &section);

    for (int i = 0; i < enum1->value_count(); ++i)
    {
        auto * value1 = enum1->value(i);
        auto * value2 = options.binary ?
                    enum2->FindValueByNumber(value1->number()) :
                    enum2->FindValueByName(value1->name());

        if (value2)
        {
            auto & subsection = section.add_subsection(Enum_Value_Comparison, value1->name(), value2->name());

            if (value1->number() != value2->number())
            {
                subsection.add_item(Enum_Value_Id_Changed,
                                    to_string(value1->number()), to_string(value2->number()));
            }
            if (value1->name() != value2->name())
            {
                subsection.add_item(Enum_Value_Name_Changed,
                                    value1->name(), value2->name());
            }
        }
        else
        {
            string value1_id = options.binary ? to_string(value1->number()) : value1->name();
            section.add_item(Enum_Value_Removed, value1_id, "");
        }
    }

    for (int i = 0; i < enum2->value_count(); ++i)
    {
        auto * value2 = enum2->value(i);
        auto * value1 = options.binary ?
                    enum1->FindValueByNumber(value2->number()) :
                    enum1->FindValueByName(value2->name());

        if (!value1)
        {
            string value2_id = options.binary ? to_string(value2->number()) : value2->name();
            section.add_item(Enum_Value_Added, "", value2_id);
        }
    }

    return &section;
}

Comparison::Section Comparison::compare(const FieldDescriptor * field1, const FieldDescriptor * field2)
{
    Section section(Message_Field_Comparison, field1->name(), field2->name());

    if (field1->name() != field2->name())
    {
        section.add_item(Message_Field_Name_Changed, field1->name(), field2->name());
    }

    if (field1->number() != field2->number())
    {
        section.add_item(Message_Field_Id_Changed, to_string(field1->number()), to_string(field2->number()));
    }

    if (field1->label() != field2->label())
    {
        section.add_item(Message_Field_Label_Changed, "", "");
    }

    if (field1->type() != field2->type())
    {
        section.add_item(Message_Field_Type_Changed, field1->type_name(), field2->type_name());
    }
    else if (field1->type() == FieldDescriptor::TYPE_ENUM)
    {
        auto * enum1 = field1->enum_type();
        auto * enum2 = field2->enum_type();

        auto * type_comparison = compare(enum1, enum2);
        type_comparison->notes.push_back("Required by " + field1->full_name() + " -> " + field2->full_name());

        type_comparison->trim();
        if (!type_comparison->is_empty())
        {
            section.add_item(Message_Field_Type_Changed, enum1->full_name(), enum2->full_name());
        }
    }
    else if (field1->type() == FieldDescriptor::TYPE_MESSAGE)
    {
        auto * msg1 = field1->message_type();
        auto * msg2 = field2->message_type();

        const MessageType msg1type = getMessageType(msg1->full_name());
        const MessageType msg2type = getMessageType(msg2->full_name());
        auto * type_comparison = compare(msg1, msg1type, msg2, msg2type);
        type_comparison->notes.push_back("Required by " + field1->full_name() + " -> " + field2->full_name());

        type_comparison->trim();
        if (!type_comparison->is_empty())
        {
            section.add_item(Message_Field_Type_Changed, msg1->full_name(), msg2->full_name());
        }
    }

    if (field1->cpp_type() == field2->cpp_type())
    {
        if (!compare_default_value(field1, field2))
        {
            section.add_item(Message_Field_Default_Value_Changed, "", "");
        }
    }

    return section;
}

Comparison::Section * Comparison::compare(const Descriptor * desc1, Comparison::MessageType desc1type, const Descriptor * desc2, Comparison::MessageType desc2type)
{
    string key = desc1->full_name() + ":" + desc2->full_name();
    if (compared.count(key))
        return compared.at(key);

    auto & section = root.add_subsection(Message_Comparison, desc1->full_name(), desc2->full_name());
    compared.emplace(key, &section);


    for (int i = 0; i < desc1->field_count(); ++i)
    {
        auto * field1 = desc1->field(i);
        auto * field2 = options.binary ?
                    desc2->FindFieldByNumber(field1->number()) :
                    desc2->FindFieldByName(field1->name());

        if (field2)
        {
            section.subsections.push_back(compare(field1, field2));
        }
        else
        {
            string field1_id = options.binary ? to_string(field1->number()) : field1->name();
            if(field1->has_optional_keyword())
            {
	            switch (desc1type)
	            {
                case Comparison::InputMessage:
                    section.add_item(Optional_InputMessage_Field_Removed, field1_id, "");
                    break;
                case Comparison::OutputMessage:
                    section.add_item(Optional_OutputMessage_Field_Removed, field1_id, "");
                    break;
                case Comparison::Undefined:
                    section.add_item(Optional_Message_Field_Removed, field1_id, "");
                    break;
	            }
            }else
            {
                section.add_item(Message_Field_Removed, field1_id, "");
            }
            
        }
    }

    for (int i = 0; i < desc2->field_count(); ++i)
    {
        auto * field2 = desc2->field(i);
        auto * field1 =  options.binary ?
                    desc1->FindFieldByNumber(field2->number()) :
                    desc1->FindFieldByName(field2->name());

        if (!field1)
        {
            string field2_id = options.binary ? to_string(field2->number()) : field2->name();
            if(field2->has_optional_keyword())
            {
                switch (desc1type)
                {
                case Comparison::InputMessage:
                    section.add_item(Optional_InputMessage_Field_Added, field2_id, "");
                    break;
                case Comparison::OutputMessage:
                    section.add_item(Optional_OutputMessage_Field_Added, field2_id, "");
                    break;
                case Comparison::Undefined:
                    section.add_item(Optional_Message_Field_Added, field2_id, "");
                    break;
                }
            }
            else
            {
                section.add_item(Message_Field_Added, "", field2_id);
            }
        }
    }

    return &section;
}

void Comparison::compare(Source & source1, Source & source2)
{
    auto * file1 = source1.file_descriptor();
    auto * file2 = source2.file_descriptor();

    
    for (int i = 0; i < file1->service_count(); ++i)
    {
        auto* service1 = file1->service(i);
        auto* service2 = file2->FindServiceByName(service1->name());
        if (! service2)
        {
            root.add_item(File_Service_Removed, service1->full_name(), "");
        }

        //Die Input und Output messages der Methoden werden gesucht
        for (int x = 0; x < service1->method_count(); ++x)
        {
            auto* method1 = service1->method(x);
            this->inputMessages.push_back(method1->input_type()->full_name());
            this->outputMessages.push_back(method1->output_type()->full_name());
        }
    }

    for (int i = 0; i < file2->service_count(); ++i)
    {
        auto* service2 = file2->service(i);
        auto* service1 = file1->FindServiceByName(service2->name());
        if (!service1)
        {
            root.add_item(File_Service_Added, service2->full_name(), "");
        }

        //Die Input und Output messages der Methoden werden gesucht
        for (int x = 0; x < service2->method_count(); ++x)
        {
            auto* method2 = service2->method(x);
            this->inputMessages.push_back(method2->input_type()->full_name());
            this->outputMessages.push_back(method2->output_type()->full_name());
        }

        this->inputMessages.unique();
        this->outputMessages.unique();
        
    }

    for (int i = 0; i < file2->message_type_count(); ++i)
    {
        auto* msg2 = file2->message_type(i);
        auto* msg1 = file1->FindMessageTypeByName(msg2->name());
        if (!msg1)
        {
            root.add_item(File_Message_Added, "", msg2->full_name());
        }
    }

    for (int i = 0; i < file1->message_type_count(); ++i)
    {
        auto * msg1 = file1->message_type(i);
        auto * msg2 = file2->FindMessageTypeByName(msg1->name());
        if (msg2)
        {
            //Get Message type of msg1 and msg2
            const MessageType msg1type = getMessageType(msg1->full_name());
            const MessageType msg2type = getMessageType(msg2->full_name());

            compare(msg1, msg1type, msg2, msg2type);
        }
        else
        {
            root.add_item(File_Message_Removed, msg1->full_name(), "");
        }
    }

    for (int i = 0; i < file2->message_type_count(); ++i)
    {
        auto * msg2 = file2->message_type(i);
        auto * msg1 = file1->FindMessageTypeByName(msg2->name());
        if (!msg1)
        {
            root.add_item(File_Message_Added, "", msg2->full_name());
        }
    }

    for (int i = 0; i < file1->enum_type_count(); ++i)
    {
        auto * enum1 = file1->enum_type(i);
        auto * enum2 = file2->FindEnumTypeByName(enum1->name());
        if (enum2)
        {
            compare(enum1, enum2);
        }
        else
        {
            root.add_item(File_Enum_Removed, enum1->full_name(), "");
        }
    }

    for (int i = 0; i < file2->enum_type_count(); ++i)
    {
        auto * enum2 = file2->enum_type(i);
        auto * enum1 = file1->FindEnumTypeByName(enum2->name());
        if (!enum1)
        {
            root.add_item(File_Enum_Added, "", enum2->full_name());
        }
    }
}

void Comparison::compare(Source & source1, const string & name1, Source & source2, const string &name2)
{
    auto desc1 = source1.pool()->FindMessageTypeByName(name1);
    auto desc2 = source2.pool()->FindMessageTypeByName(name2);

    auto enum1 = source1.pool()->FindEnumTypeByName(name1);
    auto enum2 = source2.pool()->FindEnumTypeByName(name2);

    if (desc1 && desc2)
    {
        MessageType desc1type = getMessageType(desc1->full_name());
        MessageType desc2type = getMessageType(desc2->full_name());
        compare(desc1, desc1type, desc2, desc2type);
    }
    else if (enum1 && enum2)
    {
        compare(enum1, enum2);
    }
    else
    {
        root.add_item(Name_Missing, name1, name2);
    }
}

void Comparison::print_lists()
{
    //For Debug
    cout << "Input  Messages: " << endl;

    for(auto message : this->inputMessages)
    {
        cout << message << endl;
    }

    cout << "Output  Messages: " << endl;

    for (auto message : this->outputMessages)
    {
        cout << message << endl;
    }
}

Comparison::MessageType Comparison::getMessageType(const string messageName) const
{
	if(find(inputMessages.begin(), inputMessages.end(), messageName) != inputMessages.end())
	{
        if (find(outputMessages.begin(), outputMessages.end(), messageName) != outputMessages.end())
        {
            return Comparison::Undefined;
        }else
        {
            return Comparison::InputMessage;
        }
	}else
	{
        return Comparison::OutputMessage;
	}
}
