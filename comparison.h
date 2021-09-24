#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/descriptor.h>

#include <iostream>
#include <sstream>
#include <memory>
#include <list>
#include <unordered_map>

using std::string;
using std::list;
using std::shared_ptr;
using std::unordered_map;

using google::protobuf::Descriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::EnumDescriptor;

class ErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector
{
public:
    ErrorCollector() {}

    void AddError(const std::string & filename, int line, int column, const std::string & message) override
    {
        using namespace std;
        cerr << "Error: " << filename << "@" << line << "," << column << ": " << message << endl;
    }

    void AddWarning(const std::string & filename, int line, int column, const std::string & message) override
    {
        using namespace std;
        cerr << "Warning: " << filename << "@" << line << "," << column << ": " << message << endl;
    }
};

class Source
{
    using DiskSourceTree = google::protobuf::compiler::DiskSourceTree;
    using Importer = google::protobuf::compiler::Importer;
    using DescriptorPool = google::protobuf::DescriptorPool;
    using FileDescriptor = google::protobuf::FileDescriptor;

public:
    Source() {}
    Source(const string & file_path, const string & root_dir)
    {
        source_tree.MapPath("", root_dir);

        ErrorCollector error_collector;

        importer = std::make_shared<Importer>(&source_tree, &error_collector);

        d_file_descriptor = importer->Import(file_path);
        if (!d_file_descriptor)
        {
            throw std::runtime_error("Failed to load source.");
        }
    }

    const FileDescriptor * file_descriptor() const { return d_file_descriptor; }
    const DescriptorPool * pool() const { return importer->pool(); }

    list<string> * requestMessages;
    list<string> * responseMessages;

private:
    DiskSourceTree source_tree;
    ErrorCollector error_collector;
    shared_ptr<Importer> importer;
    const FileDescriptor * d_file_descriptor = nullptr;
};

class Comparison
{
public:
    enum ItemType
    {
        Enum_Value_Name_Changed,
        Enum_Value_Id_Changed,
        Enum_Value_Added,
        Enum_Value_Removed,
        Message_Field_Name_Changed,
        Message_Field_Id_Changed,
        Message_Field_Label_Changed,
        Message_Field_Type_Changed,
        Message_Field_Default_Value_Changed,
        Message_Field_Added,
        Message_Field_Removed,
        File_Message_Added,
        File_Message_Removed,
        File_Enum_Added,
        File_Enum_Removed,
        Name_Missing,
        Optional_Message_Field_Added,
        Optional_Message_Field_Removed,
        File_Service_Added,
        File_Service_Removed,
        Optional_OutputMessage_Field_Added,
        Optional_OutputMessage_Field_Removed,
        Optional_InputMessage_Field_Added,
        Optional_InputMessage_Field_Removed
    };

    struct Item
    {
        Item(ItemType t, const string & a, const string & b): type(t), a(a), b(b) {}
        ItemType type;
        string a;
        string b;

        string message() const;
    };

    enum SectionType
    {
        Root_Section,
        Message_Comparison,
        Message_Field_Comparison,
        Enum_Comparison,
        Enum_Value_Comparison
    };

    struct Section
    {
        Section(SectionType t, const string & a, const string & b):
            type(t), a(a), b(b) {}

        SectionType type;
        string a;
        string b;

        list<string> notes;
        list<Section> subsections;
        list<Item> items;

        Section & add_subsection(SectionType t, const string & a, const string & b)
        {
            subsections.emplace_back(t, a, b);
            return subsections.back();
        }

        void add_item(ItemType t, const string & a, const string & b)
        {
            items.emplace_back(t, a, b);
        }

        bool is_empty() const { return subsections.empty() && items.empty(); }

        void trim()
        {
            auto s = subsections.begin();
            while (s != subsections.end())
            {
                s->trim();
                if (s->is_empty())
                    s = subsections.erase(s);
                else
                    ++s;
            }
        }

        string message() const;

        void print(int level = 0);
    };

    struct Options
    {
        Options() {}
        bool binary = false;
    };

    enum MessageType
    {
	    InputMessage,
        OutputMessage,
        Undefined
    };

    Comparison(const Options & options = Options{});

    void compare(Source & source1, Source & source2);
    void compare(Source & source1, const string & name1, Source & source2, const string &name2);
    Section * compare(const EnumDescriptor * enum1, const EnumDescriptor * enum2);
    Section * compare(const Descriptor * desc1, MessageType desc1type, const Descriptor * desc2, MessageType desc2type);
    Section compare(const FieldDescriptor * field1, const FieldDescriptor * field2);
    bool compare_default_value(const FieldDescriptor * field1, const FieldDescriptor * field2);

    void print_lists();

    Section root { Root_Section, "", "" };

    unordered_map<string, Section*> compared;

private:
    MessageType getMessageType(const string messageName) const;
    Options options;
    list<string> inputMessages;
    list<string> outputMessages;

};
