#!/usr/bin/env python3

INFILE_NAME = "misc/config_options.txt"
OUTFILE_NAME = "src/inc/cfg/cfg_fields.hpp"

KEYFILE_NAME = "cfg_file"

output_list = []

supported_types = [
    "string",
    "boolean",
    "integer",
    "double"]
SItype_dic = {
    "string":  "",
    "boolean": "Bool",
    "integer": "Long",
    "double":  "Double",
}

ctype_dic = {
    "string":  "const char *",
    "boolean": "bool",
    "integer": "s32",
    "double":  "double",
}

with open(INFILE_NAME) as in_f:
    with open(OUTFILE_NAME,"w") as out_f:
        out_f.write(
            f"//generated with cfg_class_gen.py\n"
            f"#include <cfg/lib/SimpleIni.h>\n\n"
            f"class Configuration_Fields {{\n"
            f"    CSimpleIniA *{KEYFILE_NAME};\n"
            f"public:\n"
            f"    Configuration_Fields(CSimpleIniA *file): {KEYFILE_NAME}(file) {{}}\n\n")

        current_group = None
        for line in in_f.readlines():
            if len(line.strip()) == 0:
                continue

            sec = line.strip().split(" ")
            if sec[0] == "group":
                if current_group != None:
                    out_f.write(
                        f"    private:\n"
                        f"        Configuration_Fields *parent;\n"
                        f"    }} {current_group} = __{current_group}(this);\n\n")
                current_group = sec[1]
                output_list.append(current_group)
                out_f.write(
                f"    /**\n"
                f"      * {current_group} Access Class\n"
                f"      */\n"
                f"    struct __{current_group} {{\n"
                f"        __{current_group}(Configuration_Fields *parent): parent(parent) {{}}\n\n")
            elif sec[0] in supported_types:
                c_type =    ctype_dic[sec[0]]
                lib_type = SItype_dic[sec[0]]
                name =      sec[1]
                out_f.write(
                    f"        /**\n"
                    f"          * {name}\n"
                    f"          */\n"
                    f"        {c_type} get_{name}() {{\n"
                    f"            return ({c_type})parent->{KEYFILE_NAME}->Get{lib_type}Value(\"{current_group}\", \"{name}\");\n"
                    f"        }}\n\n"
                    f"        void set_{name}({c_type} data) {{\n"
                    f"            parent->{KEYFILE_NAME}->Set{lib_type}Value(\"{current_group}\", \"{name}\", data);\n"
                    f"        }}\n\n")
            else:
                print("Unrecognized type:", sec[0])
        out_f.write(
            f"    private:\n"
            f"        Configuration_Fields *parent;\n"
            f"    }} {current_group} = __{current_group}(this);\n\n"
            f"}};")

