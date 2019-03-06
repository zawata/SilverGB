#!/usr/bin/env python3

INFILE_NAME = "misc/config_options.txt"
OUTFILE_NAME = "src/cfg_fields.hpp"
GKEYFILE_NAME = "cfg_file"

output_list = []

type_dic = {
    "string":  "const char *",
    "boolean": "bool",
    "integer": "s32",
    "int64":   "s64",
    "uint64":  "u64",
    "double":  "double",
}

with open(INFILE_NAME) as in_f:
    with open(OUTFILE_NAME,"w") as out_f:
        out_f.write(
            f"//generated with cfg_class_gen.py\n"
            "#include <glib-2.0/glib.h>\n\n"
            "class Configuration_Fields {\n"
            "    GKeyFile *cfg_file;\n"
            "public:\n"
            "    Configuration_Fields(GKeyFile *file): cfg_file(file) {}\n\n")

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
            elif sec[0] in ["string", "boolean","integer","int64","uint64","double"]:
                c_type =    type_dic[sec[0]]
                glib_type = sec[0]
                name =      sec[1]
                out_f.write(
                    f"        /**\n"
                    f"          * {name}\n"
                    f"          */\n"
                    f"        {c_type} get_{name}() {{\n"
                    f"            return ({c_type})g_key_file_get_{glib_type}(parent->{GKEYFILE_NAME}, \"{current_group}\", \"{name}\", nullptr);\n"
                    f"        }}\n\n"
                    f"        void set_{name}({c_type} data) {{\n"
                    f"            g_key_file_set_{glib_type}(parent->{GKEYFILE_NAME}, \"{current_group}\", \"{name}\", data);\n"
                    f"        }}\n\n")
            else:
                print("Unrecognized type:", sec[0])
        out_f.write(
            f"    private:\n"
            f"        Configuration_Fields *parent;\n"
            f"    }} {current_group} = __{current_group}(this);\n\n"
            f"}};")
        

# {c_type} Configuration::get_{name}() {
#     return ({c_type})g_key_file_get_{glib_type}({GKEYFILE_NAME}, {group}, \"{name}\", nullptr);
# }

# void Configuration::get_{name}({c_type} data) {
#     return ({c_type})g_key_file_get_{glib_type}({GKEYFILE_NAME}, {group}, \"{name}\", data);
# }

