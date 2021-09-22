This tool allows you to compare versions of a Protocol Buffer specification
(entire .proto files or specific message and enum type).
The purpose is to see clearly whether the newer version maintains backwards compatibility and what new features it adds.

## Building

### Linux
Install the protobuf and protoc library:
 - sudo apt install libprotobuf-dev
 - sudo apt install libprotoc-dev

IMPORTANT:
You also need to link the pthread library. This can be done in two different ways:
1) Using cMake: Update the CMakeList.txt with this line
    target_link_libraries(protobuf-spec-compare protoc protobuf pthread)
2) Using g++: Use this expression for compiling:
    g++ -o proto comparison.cpp main.cpp -l protobuf -l protoc -l pthread
    
### Windows

Prerequisites:

- CMake
- make
- C++ compiler
- Protocol Buffer libraries

- If you don't have the required librarys and want to build everything on your own:
    1) You need to build the protoc and protbuf library.
        For that, download them and follow the instructions in their readme.
    2) Use cmake to generate the solution of the project.
    3) Add the builded libraries to the projekt dependencies.
    4) Press "Build" in Visual Studio.

- If you just want to use the compiled .exe, use the release on github. You don't have to install anything else 

- If you already have the builded library at the right place, the original instruction of the developer might work:
    Steps:
        mkdir build
        cd build
        cmake ..
        make

## Usage

    protobuf-spec-comparator dir1 file1.proto dir2 file2.proto type-name [--binary]

The program takes 5 arguments:

- dir1: A directory containing .proto files
- file1.proto: The relative path of a .proto file in dir1
- dir2 and file2.proto: The same as above for another version to compare.
- type-name: Use '.' to compare all messages and enums. Giving the name of a specific type doesn't work

You can add the following options:

- `--binary`: Report compatibility of the binary serialization as opposed to the JSON serialization or similar. See below for details.

### Behavior

The definition of a message or enum `type-name` in file1.proto and file2.proto is compared as detailed in the following sections.

If type-name is just ".", then all the messages and enums in file1.proto and file2.proto are compared.
Added and removed messages and enums are reported.

### Message comparison

Message fields are matched by name (default) or by number (when using `--binary`).

Fields present in file1 and missing in file2 are reported as removed, and vice-versa for added fields.
Any changes in matching fields are reported.

If two matching fields both have an enum or message type, then those enums and message types are also compared.

### Enum comparison

Enum values are matched by name (a binary comparison mode will be added where values are matched by ID number).

Values present in file1 and missing in file2 are reported as removed, and vice-versa for added fields.
Any changes in maching enum values are reported.
