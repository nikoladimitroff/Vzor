import functools
import os
from os import path
import re
import sys

PRIMITIVE_TYPES_HEADER_NAME = "VzorPrimitiveTypes.h"
DATABASE_FILE_NAME = "VzorDatabase.cpp"
# Check groups <attr>, <typename>, <bases>
ATTR_LIST = r"(?P<attr>\((?:[\w_]+(?:(?:[\w_],)+)*)?\))?"
CLASS_REGEX = fr"((?P<templates>template)[\s\S]*?)?(class|struct)\s*\[\[reflect::type{ATTR_LIST}\]\]\s*(?P<typename>\w+)(?:\s*:\s*(?P<bases>(?:public|private|protected)\s+[\w:]+)(?:,\s*(?:public|private|protected)\s+[\w:]+)*)?"
# Check groups <attr>, <const>, <namespace>, <type>, <templates>, <name>
VAR_REGEX = fr"\[\[reflect::data{ATTR_LIST}\]\]\s*(?P<const>const)?\s*(?P<namespace>\w*::)*(?P<type>\w+)(?P<templates><[\w_,\s]*>)?(?P<pointer>(\*|\s|const)+)?(?P<ref>&)?\s+(?P<name>[\w_]+);"

def read_file(path):
    with open(path) as f:
        return f.read()


class ReflectedVariable:
    # TODO: REF / PTR SUPPORT
    def __init__(self, name, namespace, type, attribute_list, template_params, pointer_levels, is_const, is_ref):
        self.name = name
        self.namespace = namespace
        self.type = type
        self.attributes = attribute_list
        self.template_params = template_params
        self.pointer_levels = pointer_levels
        self.is_ref = is_ref
        self.is_const = is_const

    @property
    def fully_qualified_name(self):
        # TODO: RETURN NAME WITH NAMESPACES!
        return self.type

    @property
    def offset(self):
        # TODO: COMPUTE ACTUAL OFFSET
        return "0x0"

    @property
    def is_pointer(self):
        return self.pointer_levels > 0

    def __str__(self):
        constness = ("", "const")[self.is_const]
        templateness = "<{}>".format(self.template_params) if self.template_params is not None else ""
        return "[{}] {} {}{}{} {};".format(self.attributes, constness, self.namespace or "", self.type, templateness, self.name)


class ReflectedFunction:
    def __init__(self):
        self.return_type = ""
        self.name = ""
        self.args = []
        self.is_const = False

def static_id_counter():
    if not hasattr(static_id_counter, "count"):
        static_id_counter.count = -1

    static_id_counter.count += 1
    return static_id_counter.count

class ReflectedType:
    def __init__(self, name, base_list, attribute_list, is_templated):
        self.name = name
        self.base_list = base_list
        self.attributes = attribute_list
        self.is_templated = is_templated
        self.methods = []
        self.data_members = []
        self.id = static_id_counter()

    @property
    def fully_qualified_name(self):
        # TODO: RETURN NAME WITH NAMESPACES!
        return self.name

    def __str__(self):
        members = "\t" + "\n\t".join(str(member) for member in self.data_members)
        base_types = ": {}".format(", ".join(self.base_list)) if len(self.base_list) != 0 else ""
        return f"[{self.attributes}] class {self.name} {base_types}\n{members}"


def attribute_match_to_list(match):
    if match is None:
        return []
    return match[1:len(match) - 1].split(",")

def base_match_to_list(match):
    if match is None:
        return []
    with_namespaces = re.sub(r"public|protected|private|\s+", "", match).split(",")
    # TODO: May be only ignore the base if some option
    base_types = [re.sub(r"(\w+::)*(\w+)", r"\2", t) for t in with_namespaces]
    return base_types


def reflect_members_in_section(source):
    """
    Iterates over all reflected members of some type, assuming the passed source code is the type's code.
    No checks for the outer scope are done
    """
    members = []
    for match in re.finditer(VAR_REGEX, source, re.MULTILINE):
        name = match["name"]
        is_const = bool(match["const"])
        type = match["type"]
        namespaces = match["namespace"]
        template_params = match["templates"]
        attributes = attribute_match_to_list(match["attr"])
        pointer_levels = match["pointer"].count("*") if match["pointer"] else 0
        is_ref = match["ref"] is not None
        data_member = ReflectedVariable(name, namespaces, type, attributes, template_params, pointer_levels, is_const, is_ref)
        members.append(data_member)

    return members


def reflect_members_in_class(source, class_definition_start):
    assert(source[class_definition_start] == "{")
    opening_brace = class_definition_start
    prev_closing_brace = class_definition_start
    members = []
    while True:
        # Everything outside of a block {/} we can consider part of the class
        # This is to avoid dealing with nested classes / function code
        opening_brace = source.find("{", opening_brace + 1)
        next_closing_brace = source.find("}", opening_brace)
        members += reflect_members_in_section(source[prev_closing_brace + 1: opening_brace])
        if opening_brace < 0 or next_closing_brace < opening_brace:
            # end of class
            return members
        prev_closing_brace = next_closing_brace


def reflect_all_classes(source):
    """Iterates over all the reflected types in a string"""
    for match in re.finditer(CLASS_REGEX, source, re.MULTILINE):
        name = match["typename"]
        base_list = base_match_to_list(match["bases"])
        attributes = attribute_match_to_list(match["attr"])
        is_templated = match["templates"] is not None
        type = ReflectedType(name, base_list, attributes, is_templated)
        definition_start = source.find("{", match.end())
        assert(definition_start >= 0)
        type.data_members = reflect_members_in_class(source, definition_start)
        yield type

def generate_reflection_data_for_member(data_member):
    #{ type_id<float>(), "X", 0x0 },
    return \
f"""ReflectedVariable(type_id<{data_member.fully_qualified_name}>(), "{data_member.name}", """ +\
f"""{data_member.pointer_levels}, {data_member.is_const}, {data_member.is_ref}, {data_member.offset})"""


def generate_reflection_data_for_type(type):
    if len(type.data_members) == 0:
        return f"""
    {{ type_id<{type.fully_qualified_name}>(), "{type.name}", {{}} }}"""

    separator = ",\n            "
    return \
f"""
    {{
        type_id<{type.fully_qualified_name}>(), "{type.name}",
        {{
            {separator.join(generate_reflection_data_for_member(m) for m in type.data_members)}
        }}
    }}"""

def get_primitive_types_data():
    return [
        ReflectedType("EnableReflectionFromThis", [], [], True),
        ReflectedType("bool", [], [], False),
        ReflectedType("char", [], [], False),
        ReflectedType("int", [], [], False),
        ReflectedType("size_t", [], [], False),
        ReflectedType("float", [], [], False),
    ]


def reflect_file(source):
    file_contents = read_file(source)
    return list(reflect_all_classes(file_contents))


def update_type_refs(types):
    def find_type_by_name(types, name):
        result = next((t for t in types if t.name == name), None)
        if result is None:
            print(f"Type {name} is referenced in a reflected data but is not reflected itself!")
            sys.exit(-1)
        return result

    for type in types:
        for var in type.data_members:
            var.type = find_type_by_name(types, var.type) #///todo namespace
        type.base_list = [find_type_by_name(types, base_name) for base_name in type.base_list]


def generate_header(types, destination):
    file_start = f"""#pragma once
#include "{PRIMITIVE_TYPES_HEADER_NAME}"
namespace Vzor
{{
\t"""
    specialize_type = lambda t: "SPECIALIZE_TEMPLATED" if t.is_templated else "SPECIALIZE"
    all_type_ids = [f"{specialize_type(t)}({t.name}, {t.id}u)" for t in types]

    file_end = """
}
"""
    with open(destination, "w") as f:
        f.write(file_start + "\n\t".join(all_type_ids) + file_end)


def emit_bool(value):
    return str(value).lower()


def generate_type_registrator(type):
    def gen_variable(var):
        return f"ReflectedVariable({var.type.id}, \"{var.name}\", " + \
               f"{var.pointer_levels}, {emit_bool(var.is_const)}, {emit_bool(var.is_ref)}, {var.offset})"""

    member_sep = ",\n\t\t\t\t"
    registrator = f"""
        static TypeRegistrator Registrator_{type.name}({{
            TypeIdentifier({type.id}u), "{type.name}",
            {{
                {member_sep.join(gen_variable(v) for v in type.data_members)}
            }},
            {{ {", ".join(f"TypeIdentifier({t.id}u)" for t in type.base_list)} }},
        }});"""
    return registrator

def generate_database(reflected_types, destination):
    root_dir = os.path.dirname(os.path.abspath(__file__))
    template = read_file(os.path.join(root_dir, "vzor_database_template.cpp"))

    # Fill in the number of reflected types
    template = template.replace("/*REFLECTED_TYPES_COUNT*/", str(len(reflected_types)))

    generated_data = "\n\t".join(generate_type_registrator(t) for t in reflected_types)

    marker_registrators = "// REFLECTED DATA BEGINS HERE\n\n"
    marker_data_start = template.find(marker_registrators)
    database = \
        template[:marker_data_start + len(marker_registrators) + 1] + \
        generated_data + \
        template[marker_data_start + len(marker_registrators):]

    with open(destination, "w") as f:
        f.write(database)


def get_file_extension(filepath):
    filepath, extension = path.splitext(filepath)
    return extension

def gather_files_in_paths(filepaths):
    all_files = []
    for fsnode in filepaths:
        if path.isfile(fsnode):
            all_files.append(fsnode)
        elif path.isdir(fsnode):
            all_files += [fsnode + f for f in os.listdir(fsnode)]

    return (f for f in all_files if get_file_extension(f) in (".h", ".hpp"))


def main(filepaths_to_reflect, destination_dir):
    print("---Running Vzor reflection generator---")
    # Gather file list
    all_files = gather_files_in_paths(filepaths_to_reflect)
    types_per_file = {}
    types_per_file[PRIMITIVE_TYPES_HEADER_NAME] = get_primitive_types_data()

    # Reflect everything
    for filepath in all_files:
        print(f"Reflecting {filepath}")
        types_per_file[filepath] = reflect_file(filepath)

    # Flatten the list and generate all the data
    if not path.isdir(destination_dir):
        os.makedirs(destination_dir)

    all_reflected_types = [t for types_in_file in types_per_file.values() for t in types_in_file]
    update_type_refs(all_reflected_types)
    generate_database(all_reflected_types, path.join(destination_dir, DATABASE_FILE_NAME))
    for filepath, reflected_types in types_per_file.items():
        print(f"Generated {filepath}")
        header_name = path.basename(filepath)
        generate_header(reflected_types, path.join(destination_dir, header_name))
    
    print("---Vzor reflection data generated---")


main(["tests/"], "tests/vzorgenerated")
