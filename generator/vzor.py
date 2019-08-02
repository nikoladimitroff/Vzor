import os
import re

FILE_PATH = "test.h"
# Check groups <attr>, <typename>, <bases>
ATTR_LIST = r"(?P<attr>\((?:[\w_]+(?:(?:[\w_],)+)*)?\))?"
CLASS_REGEX = fr"(class|struct)\s*\[\[reflect::type{ATTR_LIST}\]\]\s*(?P<typename>\w+)(?:\s*:\s*(?P<bases>(?:public|private|protected)\s+\w+)(?:,\s*(?:public|private|protected)\s+\w+)*)?"
# Check groups <attr>, <const>, <namespace>, <type>, <templates>, <name>
VAR_REGEX = fr"\[\[reflect::data{ATTR_LIST}\]\]\s*(?P<const>const)?\s*(?P<namespace>\w*::)*(?P<type>\w+)(?P<templates><[\w_,\s]*>)?\s+(?P<name>[\w_]+);"

def read_file(path):
    with open(path) as f:
        return f.read()


class ReflectedVariable:
    # TODO: REF / PTR SUPPORT
    def __init__(self, name, namespace, type, attribute_list, is_const, template_params):
        self.name = name
        self.namespace = namespace
        self.type = type
        self.attributes = attribute_list
        self.is_const = is_const
        self.template_params = template_params

    @property
    def fully_qualified_name(self):
        # TODO: RETURN NAME WITH NAMESPACES!
        return self.type

    @property
    def offset(self):
        # TODO: COMPUTE ACTUAL OFFSET
        return "0x0"

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
    def __init__(self, name, base_list, attribute_list):
        self.name = name
        self.base_list = base_list
        self.attributes = attribute_list
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
    return re.sub(r"public|protected|private|\s+", "", match).split(",")


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
        data_member = ReflectedVariable(name, namespaces, type, attributes, is_const, template_params)
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
        type = ReflectedType(name, base_list, attributes)
        definition_start = source.find("{", match.end())
        assert(definition_start >= 0)
        type.data_members = reflect_members_in_class(source, definition_start)
        yield type

def generate_reflection_data_for_member(data_member):
    #{ type_id<float>(), "X", 0x0 },
    return \
f"""ReflectedVariable(type_id<{data_member.fully_qualified_name}>(), "{data_member.name}", {data_member.offset})"""


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
        ReflectedType("char", [], []),
        ReflectedType("int", [], []),
        ReflectedType("float", [], []),
    ]


def reflect_file(source):
    file_contents = read_file(source)
    types = get_primitive_types_data() + list(reflect_all_classes(file_contents))

    return types


def update_type_refs(types):
    for type in types:
        for var in type.data_members:
            var.type = next(t for t in types if t.name == var.type) #///todo namespace


def generate_header(types, destination):
    type_id_func_template = """
SPECIALIZE({}, {})
"""
    all_type_ids = [type_id_func_template.format(t.name, t.id) for t in types]

    with open(destination, "w") as f:
        f.write("".join(all_type_ids))

def generate_type_registrator(type):
    def gen_variable(var):
        return f"ReflectedVariable({var.type.id}, \"{var.name}\", {var.offset})"

    member_sep = ",\n\t\t\t\t"
    registrator = f"""
        static TypeRegistrator Registrator_{type.name}({{
            {type.id}, "{type.name}",
            {{
                {member_sep.join(gen_variable(v) for v in type.data_members)}
            }}
        }});"""
    return registrator

def generate_database(reflected_types, destination):
    root_dir = os.path.dirname(os.path.abspath(__file__))
    template = read_file(os.path.join(root_dir, "vzor_database_template.cpp"))

    indent = "\t\t"
    array_definition = f"{indent}static std::array<ReflectedType, {len(reflected_types)}> AllReflectedTypes;\n"
    generated_data = "\n\t".join(generate_type_registrator(t) for t in reflected_types)

    marker_registrators = "// REFLECTED DATA BEGINS HERE\n\n"
    marker_array = "// REFLECTED ARRAY DEFINITION HERE\n"
    marker_data_start = template.find(marker_registrators)
    marker_array_start = template.find(marker_array)
    database = template[:marker_array_start + len(marker_array) + 1] + \
        array_definition + \
        template[marker_array_start + len(marker_array):marker_data_start + len(marker_registrators) + 1] + \
        generated_data + \
        template[marker_data_start + len(marker_registrators):]

    with open(destination, "w") as f:
        f.write(database)

def main():
    reflected_types = reflect_file("tests/TestTypes.h")
    update_type_refs(reflected_types)
    generate_header(reflected_types, "tests/vzorgenerated/TestTypes.h")
    generate_database(reflected_types, "tests/vzor_database.cpp")

main()
