# The MIT License
#
# Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
# For conditions of distribution and use, see the accompanying COPYING file.
#
#
from lxml import etree
import sys
import os
import os.path


class ExceptionChildNotFound(BaseException):
    def __init__(self, s):
        BaseException.__init__(self, s)


def getAttribute(elem, attributeName):
    try:
        return elem.attrib[attributeName]
    except KeyError:
        print(repr(attributeName), repr(elem.text), repr(elem))
        raise KeyError


def getChild(elem, childName):
    child = elem.find(childName)
    if child is None:
        raise ExceptionChildNotFound("Child '%s' not found %s"
                                     % (childName,
                                        "attributes:" + repr(elem.keys()) +
                                        " text:" + repr(elem.text) +
                                        " children:" + repr(elem.getchildren())))
    return child


def getChildOrNone(elem, childName):
    return elem.find(childName)

def print_elem(message, elem):
    print(message, elem.tag, repr(elem.attrib), elem.text, elem.tail)

class FunctionSignature:
    def __init__(self, elem, name=None):
        self.elem = elem
        if name is None:
            self.name = getChild(elem, "name")
        else:
            self.name = name

        self.api_id_define = None
        self.cmd_id_define = None

    def __str__(self):
        return self.name + " "  + self.elem.tag + " { " + repr(self.elem.attrib) + " } "

    def buildSignature(self):
        self.return_type = ""
        self.parameters = []  # [(type,name),...] ie [(GLenum, target),...]
        self.args = None  # lazily constructed "arg0,arg1,arg2"
        self.nameToGroup = {}  # [paramName: groupOrNone]

        proto = getChild(self.elem, "proto")

        if proto.text:
            # proto.text can be 'void '. => no return
            # proto.text can also be 'void *' => ptr return
            # it can also be 'const ' => return with the type in ptype.
            try:
                ptype_elem = getChild(proto, "ptype")
                self.return_type = proto.text.strip() + " " + ptype_elem.text + ptype_elem.tail
                self.hasReturnType = True
            except ExceptionChildNotFound:
                self.return_type = proto.text.strip()
                if self.return_type.endswith("*"):
                    self.hasReturnType = True
                else:
                    self.hasReturnType = False
                    if self.return_type != 'void':
                        self.hasReturnType = True
        else:
            # proto.text can also be empty, where the type is in ptype (no const)
            ptype_elem = getChild(proto, "ptype")
            self.return_type = ptype_elem.text + ptype_elem.tail

            self.hasReturnType = True

        for e in self.elem.findall("param"):
            elem_name = getChild(e, "name")
            pname = elem_name.text
            self.nameToGroup[pname] = e.attrib.get("group", None)
            if pname and elem_name.tail:
                postpend = elem_name.tail.strip() # array subscripts can end up here
            else:
                postpend = ""

            try:
                # form; <param><ptype>GLenum</ptype> <name>pname</name></param>
                # exception; This one differs from gl headers(?)
                # <param len="count">const <ptype>GLchar</ptype> *const*<name>string</name></param>
                # so were replacing "*const*" ... but only for glShaderSource as other *const* are left.
                # (This produces a function signature clash otherwise)
                ptype_elem = getChild(e, "ptype")
                if e.text is not None:
                    ptype = e.text + ptype_elem.text + ptype_elem.tail
                else:
                    ptype = ptype_elem.text + ptype_elem.tail
            except ExceptionChildNotFound:
                # form; <param len="size">const void *<name>data</name></param>
                ptype = e.text
            self.parameters.append((ptype, pname, postpend))

    def parameterList(self):
        if self.args is None:
            self.args = [t + " " + n + p for (t, n, p) in self.parameters]
            self.args = ", ".join(self.args)
        return self.args

    def parameterStructBody(self, name):
        ret  = "{\n"
        for ptype, pname, posttype in self.parameters:
            ret += "  " + ptype + " " + pname + " " + posttype + ";\n"
        ret += "}"
        return ret

    def declaration(self, prepend=""):
        return "extern DLL_EXPORT " + self.return_type + " " + prepend + self.name + "(" + self.parameterList() + ");"

    def extended_declaration(self, prepend=""):
        params = self.parameterList()
        if params:
            return "extern DLL_EXPORT " + self.return_type + " " + prepend + self.name + "(Type_" + self.name + " command," + self.parameterList() + ");"
        else:
            return "extern DLL_EXPORT " + self.return_type + " " + prepend + self.name + "(Type_" + self.name + " command);"

    def pointerDeclaration(self, storageClass="extern ", prepend="", name=None):
        if name:
            n = name
        else:
            if prepend:
                n = prepend + self.name
            else:
                n = self.name

        return self.return_type + " (*" + n + ")(" + self.parameterList() + ")"

    def pointerVariable(self, storageClass="static ", prepend="", toPrepend=None, name=None):
        if name:
            n = name
            toName = n
        else:
            if prepend:
                n = prepend + self.name
            else:
                n = self.name
            toName = self.name

        if toPrepend is None:
            return storageClass + self.return_type + " (*" + n + ")(" + self.parameterList() + ") = NULL;"
        else:
            return storageClass + self.return_type + " (*" + n + ")(" + self.parameterList() + ") = " + toPrepend + toName + ";"

    def pointerCast(self, prepend=""):
        return "(" + self.return_type + " (*)(" + self.parameterList() + "))"

    def typedefDeclaration(self):
        params = self.parameterList()
        if params:
            extended_params = "Type_" + self.name + " command," + params
        else:
            extended_params = "Type_" + self.name + " command"

        return "typedef " + self.return_type + " (*Type_" + self.name + ")(" + params + ");\n" + \
               "typedef " + self.return_type + " (*ExtendedType_" + self.name + ")(" + extended_params + ");\n"

# api xml struct for references to xml elements for lookup
class ApiXml:
    def __init__(self, filename=None, exclusions=None):
        self.includes = {}    # dependent types #include/#define
        self.types = {}       # gl types
        self.enums = {}       # gl enums
        self.funcs = {}       # gl funcs (or commands); as lazily constructed FunctionSignature.
        self.apis = {}        # apis; {"gl":[{version:(1,0),types:[..]}]} with require/remove/deprecated
        self.extensions = {}  # extensions; {"name": [{supported:"",types:[..]}], "gles": ...}

        if filename:
            self.read(filename)

        self.exclusions = exclusions or {}

    def __str__(self):
        ret = "Includes:\n"

        for k,v in self.includes.items():
            ret += " " + k + " {" + repr(v.attrib) + "}\n"

        ret += "Types:\n"

        for k,v in self.types.items():
            ret += " " + k + " {" + repr(v.attrib) + "}\n"

        ret += "Functions:\n"

        for k,v in self.funcs.items():
            ret += " " + k + " {" + str(v) + "}\n"

        ret += "Enums:\n"

        for k,v in self.enums.items():
            ret += " " + k + " {" + repr(v.attrib) + "}\n"

        ret += "APIs:\n"

        for k,v in self.apis.items():
            ret += " " + k + " {" + repr(v) + "}\n"

        ret += "Extensions:\n"

        for k,v in self.extensions.items():
            ret += " " + k + " {" + repr(v) + "}\n"

        return ret

    def read(self, filename_or_etree):
        try:
            self.etree = etree.parse(open(filename_or_etree))
        except:
            self.etree = filename_or_etree

        self._getTypes(self.etree)
        self._getEnums(self.etree)
        self._getCommands(self.etree)
        self._getAPIs(self.etree)
        self._getExtensions(self.etree)

    def versions(self, api):
        return [d["version"] for d in self.apis[api]]

    def addFunctionSignatures(self, commands, api_id_define):
        for c in commands:
            # signature = FunctionSignature(api_xml.funcs[c].elem, c)
            self.funcs[c].buildSignature()
            self.funcs[c].api_id_define = api_id_define
            self.funcs[c].cmd_id_define = makeCommandIdDefine(c)

    def functionSignature(self, functionName):
        return self.funcs[functionName].signature

    def findCommandElement(self, commandName):
        cmds = self.etree.find("commands")
        if cmds is not None:
            for c in cmds.iterchildren():
                proto = c.find("proto")
                if proto is not None:
                    name = proto.find("name")
                    if name is not None:
                        if name.text == commandName:
                            return c

    def _getTypes(self, tree):
        # get types
        for elem in tree.findall("types/type"):
            name = getChildOrNone(elem, "name")
            if name is not None:
                self.types[name.text] = elem
            else:
                name = getAttribute(elem, "name")
                if name:
                    self.includes[name] = elem
                else:
                    raise BaseException(elem.text)

    def _getEnums(self, tree):
        for elem in tree.findall("groups/group"):
            name = getAttribute(elem, "name")
            lst = []
            for elem_value in elem.findall("enum"):
                lst.append((getAttribute(elem_value, "name"), elem_value))
            self.enums[name] = lst

    def _getCommands(self, tree):
        for elem in tree.findall("commands/command"):
            elem_proto = getChild(elem, "proto")
            try:
                name = getChild(elem_proto, "name").text
            except KeyError:
                raise BaseException(repr(elem.text) + " " + repr(elem_proto.text))

            self.funcs[name] = FunctionSignature(elem, name)

    def __appendApiLists(self, element_iter, dictionary):
        for e in element_iter:
            for elem_type in e.findall("type"):
                dictionary["types"].append(getAttribute(elem_type, "name"))
            for elem_enum in e.findall("enum"):
                dictionary["enums"].append(getAttribute(elem_enum, "name"))
            for elem_command in e.findall("command"):
                dictionary["commands"].append(getAttribute(elem_command, "name"))

    def _getAPIs(self, tree):
        for elem in tree.findall("feature"):
            api_name = getAttribute(elem, "api")
            api_version = [int(x) for x in getAttribute(elem, "number").split(".")]
            api_define = getAttribute(elem, "name")
            try:
                self.apis[api_name].append({"version": api_version, "define": api_define})
            except KeyError:
                self.apis[api_name] = [{"version": api_version, "define": api_define}]

            A = self.apis[api_name][-1]

            A["require"] = {"types": [], "enums": [], "commands": []}
            A["remove"] = {"types": [], "enums": [], "commands": []}
            A["deprecate"] = {"types": [], "enums": [], "commands": []}

            self.__appendApiLists(elem.findall("require"), A["require"])
            self.__appendApiLists(elem.findall("remove"), A["remove"])
            self.__appendApiLists(elem.findall("deprecate"), A["deprecate"])

    def _getExtensions(self, tree):
        for elem in tree.findall("extensions/extension"):
            name = getAttribute(elem, "name")

            if name in self.exclusions:
                continue

            self.extensions[name] = {"supported": getAttribute(elem, "supported"),
                                    "types": [], "enums": [], "commands": []}

            self.__appendApiLists(elem.findall("require"), self.extensions[name])

    def writeGlApiTypes(self, output):
        output.write("// types\n")
        for name, t in self.types.items():
            n = getChild(t, "name")
            apientry = t.find("apientry")

            if t.text is not None:
                output.write(t.text)

            if apientry is not None:
                output.write(apientry.tail)

            output.write(name + n.tail + "\n")

class FunctionSignatureVulkan:
    def __init__(self, elem, name=None):
        self.elem = elem
        if name is None:
            self.name = getChild(elem, "name")
        else:
            self.name = name

        self.api_id_define = None
        self.cmd_id_define = None

    def __str__(self):
        return "-" * 10 + self.name + " "  + self.elem.tag + " { " + repr(self.elem.attrib) + " } "

    def buildSignature(self):
        self.return_type = ""
        self.parameters = []  # [(type,name),...] ie [(GLenum, target),...]
        self.args = None  # lazily constructed "arg0,arg1,arg2"
        self.nameToGroup = {}  # [paramName: groupOrNone]

        proto = getChild(self.elem, "proto")

        if proto.text:
            # proto.text can be 'void '. => no return
            # proto.text can also be 'void *' => ptr return
            # it can also be 'const ' => return with the type in ptype.
            try:
                ptype_elem = getChild(proto, "type")
                self.return_type = proto.text.strip() + " " + ptype_elem.text + ptype_elem.tail
                self.hasReturnType = True
            except ExceptionChildNotFound:
                self.return_type = proto.text.strip()
                if self.return_type.endswith("*"):
                    self.hasReturnType = True
                else:
                    self.hasReturnType = False
                    if self.return_type != 'void':
                        self.hasReturnType = True
        else:
            # proto.text can also be empty, where the type is in ptype (no const)
            ptype_elem = getChild(proto, "type")
            self.return_type = ptype_elem.text + ptype_elem.tail
            if ptype_elem.text.strip() == "void":
                self.hasReturnType = False
            else:
                self.hasReturnType = True

        for e in self.elem.findall("param"):
            elem_name = getChild(e, "name")
            pname = elem_name.text
            self.nameToGroup[pname] = e.attrib.get("group", None)
            if pname and elem_name.tail:
                postpend = elem_name.tail.strip()
            else:
                postpend = ""
            try:
                # form; <param><ptype>GLenum</ptype> <name>pname</name></param>
                # exception; This one differs from gl headers(?)
                # <param len="count">const <ptype>GLchar</ptype> *const*<name>string</name></param>
                # so were replacing "*const*" ... but only for glShaderSource as other *const* are left.
                # (This produces a function signature clash otherwise)
                # We also have this in vulkan
                # <param>const <type>float</type> <name>blendConstants</name>[4]</param>
                # where '[4]' is the name tag tail.
                ptype_elem = getChild(e, "type")
                if e.text is not None:
                    ptype = e.text + ptype_elem.text + ptype_elem.tail
                else:
                    ptype = ptype_elem.text + ptype_elem.tail
            except ExceptionChildNotFound:
                # form; <param len="size">const void *<name>data</name></param>
                ptype = e.text
            self.parameters.append((ptype, pname, postpend))

    def parameterList(self):
        if self.args is None:
            self.args = [t + " " + n + p for (t, n, p) in self.parameters]
            self.args = ", ".join(self.args)
        return self.args

    def declaration(self, prepend=""):
        return "extern VKAPI_ATTR " + self.return_type + " VKAPI_CALL " + prepend + self.name + "(" + self.parameterList() + ");"

    def extended_declaration(self, prepend=""):
        params = self.parameterList()
        if params:
            return "extern DLL_EXPORT " + self.return_type + prepend + self.name + "(Type_" + self.name + " command," + self.parameterList() + ");"
        else:
            return "extern DLL_EXPORT " + self.return_type + prepend + self.name + "(Type_" + self.name + " command);"

    def pointerDeclaration(self, storageClass="extern ", prepend="", name=None):
        if name:
            n = name
        else:
            if prepend:
                n = prepend + self.name
            else:
                n = self.name

        return self.return_type + " (*" + n + ")(" + self.parameterList() + ")"

    def pointerVariable(self, storageClass="static ", prepend="", toPrepend=None, name=None):
        if name:
            n = name
        else:
            if prepend:
                n = prepend + self.name
            else:
                n = self.name

        if toPrepend is None:
            return storageClass + self.return_type + " (*" + n + ")(" + self.parameterList() + ") = NULL;"
        else:
            return storageClass + self.return_type + " (*" + n + ")(" + self.parameterList() + ") = " + toPrepend + n + ";"

    def pointerCast(self, prepend=""):
        return "(" + self.return_type + " (*)(" + self.parameterList() + "))"

    def typedefDeclaration(self):
        return "typedef VKAPI_ATTR " + self.return_type + " (*Type_" + self.name + ")(" + self.parameterList() + ");\n" + \
          "typedef " + self.return_type + " (*ExtendedType_" + self.name + ")(Type_" + self.name + " command," + self.parameterList() + ");\n"

class ApiXmlVulkan(ApiXml):
    def __init__(self, filename=None, exclusions = None):
        if exclusions is None:
            exclusions = {}

        exclusions.update({
            "VK_NV_external_memory_win32":1,
            "VK_KHR_xlib_surface":1,
            "VK_KHR_xcb_surface":1,
            "VK_KHR_mir_surface":1,
            "VK_KHR_wayland_surface":1,
            "VK_KHR_win32_surface":1,
            "VK_NV_external_memory_capabilities":1   # lunarG vulkan.h has this but platform android-24 doesnt
            })

        ApiXml.__init__(self, filename, exclusions)

    def _getCommands(self, tree):
        for elem in tree.findall("commands/command"):
           
            if "name" in elem.attrib: 
                name = elem.attrib["name"]
                alias =  elem.attrib["alias"]
                self.funcs[name] = self.funcs[alias]
            else:
                elem_proto = getChild(elem, "proto")
                try:
                    name = getChild(elem_proto, "name").text
                except KeyError:
                    raise BaseException(repr(elem.text) + " " + repr(elem_proto.text))

                self.funcs[name] = FunctionSignatureVulkan(elem, name)

    def writeGlApiTypes(self, output):
        output.write("// types\n")


def ApiXmlClass(api):
    if api == "vulkan":
        return ApiXmlVulkan
    else:
        return ApiXml


def getCommandList(api_xml,
                   api_name,  # gl,gles1,gles2,
                   version=None,   # "4.5", None for latest
                   ):
    if version is None:
        MajorMinor = api_xml.versions(api_name)[-1]
        print("Generating functions for ",repr(api_name), repr(MajorMinor), "(Using latest version as not defined on command line)")
    else:
        MajorMinor = version #[int(x) for x in version.split(".")]
        print("Generating functions for ",repr(api_name), repr(MajorMinor), "(On command line)", repr(version))

    enums = {}
    types = {}
    all_commands = {}

    # @todo stop at the correct version...
    for api in api_xml.apis[api_name]:
        if api["version"] > MajorMinor:
            continue
        for type_name in api["require"]["types"]:
            types[type_name] = None
        for enum_name in api["require"]["enums"]:
            enums[enum_name] = None
        for command_name in api["require"]["commands"]:
            all_commands[command_name] = None

        for type_name in api["remove"]["types"]:
            del types[type_name]
        for enum_name in api["remove"]["enums"]:
            del enums[enum_name]
        for command_name in api["remove"]["commands"]:
            del all_commands[command_name]

    # @todo this seems to pull in lots of commands.
    # (Perhaps this is okay as dlopen will just fail to load the function)
    for _, value in api_xml.extensions.items():
        if api_name in value["supported"]:
            for cmd in value["commands"]:
                all_commands[cmd] = None

    sorted_commands = list(all_commands.keys())

    sorted_commands.sort()

    return sorted_commands

def writeDefaultExportedFunctions(function_signature, name, api, api_id, output_buffer):
    parameters = [pname for ptype, pname, posttype in function_signature.parameters]
    parameter_call_list = ",".join(parameters)
    if parameter_call_list:
        extended_parameter_call_list = "original_%s, " % name + parameter_call_list
    else:
        extended_parameter_call_list = "original_%s" % name
    # posttype typically as array info i.e. 'GLuint  baseAndCount[2]' which we treat as array. Also note we
    # just copy pointers and not backing data
    parameter_decl_list = ";".join([ptype + "* " + pname
                                    if "[" in posttype else ptype + " " + pname
                                    for ptype, pname, posttype in function_signature.parameters])

    output_buffer.write(function_signature.declaration()[:-1])
    if function_signature.hasReturnType:
        output_buffer.write('''
{{
  return reinterpret_cast<ExtendedType_{function}>(CommandPointers[{id_define}])({parameters});
}}

'''.format(function=name, id_define=makeCommandIdDefine(name), parameters=extended_parameter_call_list))
    else:
        output_buffer.write('''
{{
  reinterpret_cast<ExtendedType_{function}>(CommandPointers[{id_define}])({parameters});
}}

'''.format(function=name, id_define=makeCommandIdDefine(name), parameters=extended_parameter_call_list))

    # passthrough_
    output_buffer.write(function_signature.extended_declaration(prepend="passthrough_")[:-1])
    if function_signature.hasReturnType:
        output_buffer.write('''
{{
  return command({parameters});
}}

'''.format(function=name, parameters=parameter_call_list))
    else:
        output_buffer.write('''
{{
  command({parameters});
}}
'''.format(function=name, parameters=parameter_call_list))

    if api in ["gl", "gles2", "gles3"]:
        geterror = "int32_t err = static_cast<int32_t>(original_glGetError());"
    elif api in ["egl"]:
        geterror = "int32_t err = static_cast<int32_t>(original_eglGetError());"
    else:
        geterror = "int32_t err = 0;"

    # log parameters_
    output_buffer.write(function_signature.extended_declaration(prepend="recordParameters_")[:-1])
    if function_signature.hasReturnType:
        output_buffer.write('''
{{
  ShimBeginCommand({api_id}, {id_define});
  {R} ret = command({parameters});
  {geterror}
  #pragma pack(0)
  struct Params {{ ParamsHeader _header; uint32_t _api_id; uint32_t _cmd_id; int32_t _err; uint32_t _hasReturn;  {R} _ret; {parameter_decl_list} }} __attribute__((packed)) p = {{ {{0, 0, 0}}, {api_id}, {id_define}, err, 1, ret, {parameters_list} }};
  #pragma pack()
  ShimStoreParameters( p._header, sizeof(p) );
  ShimEndCommand({api_id}, {id_define});
  return ret;
}}

'''.format(api_id=api_id, id_define=makeCommandIdDefine(name), R=function_signature.return_type, function=name,
       parameters=parameter_call_list,
       parameters_list=parameter_call_list if parameter_call_list else "",
       parameter_decl_list=parameter_decl_list + ";" if parameter_decl_list else "",
       geterror=geterror))
    else:
        output_buffer.write('''
{{
  ShimBeginCommand({api_id}, {id_define});
  command({parameters});
  {geterror}
  #pragma pack(0)
  struct Params {{ ParamsHeader _header; uint32_t _api_id; uint32_t _cmd_id; int32_t _err; uint32_t _hasReturn; {parameter_decl_list} }} __attribute__((packed)) p = {{ {{0, 0, 0}}, {api_id}, {id_define}, err, 0, {parameters_list} }};
  #pragma pack()
  ShimStoreParameters( p._header, sizeof(p) );
  ShimEndCommand({api_id}, {id_define});
}}

'''.format(api_id=api_id, id_define=makeCommandIdDefine(name), R=function_signature.return_type, function=name,
       parameters=parameter_call_list,
       parameters_list=parameter_call_list if parameter_call_list else "",
       parameter_decl_list=parameter_decl_list + ";" if parameter_decl_list else "",
       geterror=geterror))

    # shim_ function
    output_buffer.write(function_signature.extended_declaration(prepend="shim_")[:-1])
    if function_signature.hasReturnType:
        output_buffer.write('''
{{
  ShimBeginCommand({api_id}, {id_define});
  {R} ret = command({parameters});
  ShimEndCommand({api_id}, {id_define});
  return ret;
}}

'''.format(api_id=api_id, id_define=makeCommandIdDefine(name), R=function_signature.return_type,
           function=name, parameters=parameter_call_list))
    else:
        output_buffer.write('''
{{
  ShimBeginCommand({api_id}, {id_define});
  command({parameters});
  ShimEndCommand({api_id}, {id_define});
}}

'''.format(api_id=api_id, id_define=makeCommandIdDefine(name),
           function=name, parameters=parameter_call_list))

def writeVulkanLayerFunctions(function_signature, name, api, api_id, output_buffer):
    parameters = [pname for ptype, pname, posttype in function_signature.parameters]
    parameter_call_list = ",".join(parameters)
    # posttype typically as array info i.e. 'GLuint  baseAndCount[2]' which we treat as array. Also note we
    # just copy pointers and not backing data
    parameter_decl_list = ";".join([ptype + "* " + pname
                                    if "[" in posttype else ptype + " " + pname
                                    for ptype, pname, posttype in function_signature.parameters])

    #output_buffer.write(function_signature.declaration()[:-1])
    output_buffer.write("extern VKAPI_ATTR " + function_signature.return_type + " VKAPI_CALL " + \
                            function_signature.name[2:] + "(" + function_signature.parameterList() + ")\n")

    parameters = [pname for ptype, pname, posttype in function_signature.parameters]

    if function_signature.hasReturnType:
        output_buffer.write('''
{{
  ShimBeginCommand({api_id}, {id_define});
  Log(LogVerbose,"%s %d", "{function}", {id_define});
  {R} ret = ({R})0;
  auto table = GetDispatchTable({first_parameter});
  if(table && table->{struct_name})
  {{
    Log(LogVerbose,"  %x", (long long*)table->{struct_name});
    ret = reinterpret_cast<ExtendedType_{function}>(CommandPointers[{id_define}])(table->{struct_name}, {parameters});
  }}
  else
  {{
    Log(LogError, "Missing Command Called:%s", pChar_{function});
  }}
  ShimEndCommand({api_id}, {id_define});
  return ret;
}}

'''.format(api_id=api_id, id_define=makeCommandIdDefine(name), R=function_signature.return_type,
           struct_name=name[2:], # in the struct its not prepended with vk
           function=name, first_parameter=parameters[0], parameters=parameter_call_list))
    else:
        output_buffer.write('''
{{
  ShimBeginCommand({api_id}, {id_define});
  auto table = GetDispatchTable({first_parameter});
  Log(LogVerbose,"%s %d", "{function}", {id_define});
  if(table && table->{struct_name})
  {{
    Log(LogVerbose,"  %x", (long long*)table->{struct_name});
    reinterpret_cast<ExtendedType_{function}>(CommandPointers[{id_define}])(table->{struct_name}, {parameters});
  }}
  else
  {{
    Log(LogError, "Missing Command Called:%s", pChar_{function});
  }}
  ShimEndCommand({api_id}, {id_define});
}}

'''.format(api_id=api_id, id_define=makeCommandIdDefine(name),
           struct_name=name[2:], # in the struct its not prepended with vk
           function=name, first_parameter=parameters[0], parameters=parameter_call_list))

    # passthrough_
    output_buffer.write(function_signature.extended_declaration(prepend="passthrough_")[:-1])
    if function_signature.hasReturnType:
        output_buffer.write('''
{{
  return command({parameters});
}}

'''.format(function=name, R=function_signature.return_type,
           struct_name=name[2:], # in the struct its not prepended with vk
           first_parameter=parameters[0], parameters=parameter_call_list))
    else:
        output_buffer.write('''
{{
  command({parameters});
}}
'''.format(function=name,
           struct_name=name[2:], # in the struct its not prepended with vk
           first_parameter=parameters[0], parameters=parameter_call_list))

    if api in ["gl", "gles2", "gles3"]:
        geterror = "int32_t err = static_cast<int32_t>(original_glGetError());"
    else:
        geterror = "int32_t err = 0;"

    # log parameters_
    # The Params structure will be written into the CFS File by ShimStoreParameters() ,which will update the values in _header.
    # __attribute__((packed) is required for the collector to correctly read the data from the file.
    output_buffer.write(function_signature.extended_declaration(prepend="recordParameters_")[:-1])
    if function_signature.hasReturnType:
        output_buffer.write('''
{{
  ShimBeginCommand({api_id}, {id_define});
  {R} ret = command({parameters});
  {geterror}
  #pragma pack(0)
  struct Params {{ ParamsHeader _header; uint32_t _api_id; uint32_t _cmd_id; int32_t _err; uint32_t _hasReturn; {R} _ret; {parameter_decl_list}}} __attribute__((packed)) p = {{ {{0, 0, 0}}, {api_id}, {id_define}, err, 1, ret, {parameters_list}  }};
  #pragma pack()
  ShimStoreParameters( p._header, sizeof(p) );
  ShimEndCommand({api_id}, {id_define});
  return ret;
}}

'''.format(api_id=api_id, id_define=makeCommandIdDefine(name), R=function_signature.return_type, function=name,
       parameters=parameter_call_list,
       parameters_list=parameter_call_list if parameter_call_list else "",
       parameter_decl_list=parameter_decl_list + ";" if parameter_decl_list else "",
       geterror=geterror))
    else:
        output_buffer.write('''
{{
  ShimBeginCommand({api_id}, {id_define});
  command({parameters});
  {geterror}
  #pragma pack(0)
  struct Params {{ ParamsHeader _header; uint32_t _api_id; uint32_t _cmd_id; int32_t _err; uint32_t _hasReturn; {parameter_decl_list} }} __attribute__((packed)) p = {{ {{0, 1, 0}}, {api_id}, {id_define}, err, 0, {parameters_list} }};
  #pragma pack()
  ShimStoreParameters( p._header, sizeof(p) );
  ShimEndCommand({api_id}, {id_define});
}}

'''.format(api_id=api_id, id_define=makeCommandIdDefine(name), R=function_signature.return_type, function=name,
       parameters=parameter_call_list,
       parameters_list=parameter_call_list if parameter_call_list else "",
       parameter_decl_list=parameter_decl_list + ";" if parameter_decl_list else "",
       geterror=geterror))

    # shim_ function
    output_buffer.write(function_signature.extended_declaration(prepend="shim_")[:-1])
    if function_signature.hasReturnType:
        output_buffer.write('''
{{
  ShimBeginCommand({api_id}, {id_define});
  {R} ret = command({parameters});
  ShimEndCommand({api_id}, {id_define});
  return ret;
}}

'''.format(api_id=api_id, id_define=makeCommandIdDefine(name), R=function_signature.return_type,
           function=name, parameters=parameter_call_list))
    else:
        output_buffer.write('''
{{
  ShimBeginCommand({api_id}, {id_define});
  command({parameters});
  ShimEndCommand({api_id}, {id_define});
}}

'''.format(api_id=api_id, id_define=makeCommandIdDefine(name),
           function=name, parameters=parameter_call_list))

def writeExportedFunctions(sorted_commands, excluded_commands, api, api_xml, api_id, output_buffer):
    for c in sorted_commands:
        if c in excluded_commands:
            continue

        if api == "vulkan":
            writeVulkanLayerFunctions(api_xml.funcs[c], c, api, api_id, output_buffer)
        else:
            writeDefaultExportedFunctions(api_xml.funcs[c], c, api, api_id, output_buffer)

def writeXmlCommandParameterFunction(all_commands, signatures, apis, header, cpp):
    cpp.write("""
static char const * unknown = "unknown";

struct CommandParam {{
  char const* type;
  int size;
  char const* name;
  CommandParam() : type(unknown), size(0), name(unknown) {{}}
  CommandParam(char const* t, size_t s, char const* n) : type(t), size(s), name(n) {{}}
}};

typedef std::vector<CommandParam> ParamList;

struct ApiCommand {{
  int cmd_id;
  const char* name;
  const char*return_type;
  int return_type_size;
  ParamList commandParams;
  ApiCommand(): cmd_id(0), name(unknown), return_type(unknown), return_type_size(0) {{}}
  ApiCommand(int id, const char* n, const char* rt, size_t rs): cmd_id(id), name(n), return_type(rt), return_type_size(rs) {{}}
}};

typedef std::vector<ApiCommand> CmdList;

typedef std::map<int, CmdList> ApiCommands;
static ApiCommands apiCommands;

// static std::vector<CommandParam> CommandParams[{params_len}];

""".format(params_len=len(all_commands)))

    cpp.write("""
void InitCommandParameters()
{
  static bool done = false;
  if(done)
    return;
  else
    done = true;

  CmdList* pCmdList = NULL;
  ParamList* pParamList = NULL;
""")

    cmd_counts = {}
    for c in all_commands:
        function_signature = signatures[c]
        try:
            cmd_counts[function_signature.api_id_define] += 1
        except KeyError:
            cmd_counts[function_signature.api_id_define] = 0

    for api in apis:
        apiid_define = api_to_apiid_define(api)
        cpp.write("""
  pCmdList = &apiCommands[{apiid_define}];
  pCmdList->reserve({cmd_count});
""".format(apiid_define=apiid_define,
           cmd_count = cmd_counts[apiid_define]))

    current_api_id_define = None
    for c in all_commands:
        function_signature = signatures[c]
        parameters = [(ptype+"*", pname) if posttype else (ptype, pname)
                          for ptype, pname, posttype in function_signature.parameters]

        # mostly they are in order of api so we dont need to do the pointer lookup again
        if current_api_id_define == function_signature.api_id_define:
            set_pointer = ""
        else:
            set_pointer = "pCmdList = &(apiCommands[{apiid_define}]);".format(
                apiid_define=function_signature.api_id_define)

            current_api_id_define = function_signature.api_id_define

        return_type=function_signature.return_type
        cpp.write("""
  {set_pointer}
  pCmdList->push_back(ApiCommand({cmd_id_define}, {cmd_name}, "{return_type}", {return_type_size}));
  pParamList = &pCmdList->back().commandParams;
  pParamList->reserve({param_count});""".format(
      api_id_define=function_signature.api_id_define,
      cmd_id_define=function_signature.cmd_id_define,
      return_type=return_type,
      return_type_size="sizeof(%s)" % return_type if function_signature.hasReturnType else "0",
      param_count=len(parameters),
      set_pointer=set_pointer,
      cmd_name="pChar_%s" % c))

        for ptype, pname in parameters:
            cpp.write("""
  pParamList->push_back( CommandParam("{ptype}", sizeof({ptype}), "{pname}") );""".format(
      ptype=ptype.strip(),
      pname=pname.strip()))

    cpp.write("""
}
""")

    cpp.write("""
using namespace tinyxml2;

void WriteXmlCommandParameters(XMLDocument &doc, XMLElement& appendTo)
{
  InitCommandParameters();
  if(XMLElement* params = doc.NewElement("Commands"))
  {
    appendTo.InsertEndChild(params);
    ApiCommands::iterator apiIter = apiCommands.begin();
    ApiCommands::iterator apiEnd = apiCommands.end();
    for(; apiIter != apiEnd; ++apiIter)
    {
      if(XMLElement* api = doc.NewElement("Api"))
      {
        params->InsertEndChild(api);
        api->SetAttribute("Id", apiIter->first);
        CmdList& cmdList = apiIter->second;
        CmdList::iterator cmdIter = cmdList.begin();
        CmdList::iterator cmdEnd = cmdList.end();
        for(; cmdIter != cmdEnd; ++cmdIter)
        {
          if(XMLElement* cmd = doc.NewElement("Command"))
          {
            api->InsertEndChild(cmd);
            cmd->SetAttribute("Id", cmdIter->cmd_id);
            cmd->SetAttribute("Name", cmdIter->name);
            cmd->SetAttribute("ReturnType", cmdIter->return_type);
            cmd->SetAttribute("ReturnSize", cmdIter->return_type_size);
            ParamList::iterator paramIter = cmdIter->commandParams.begin();
            ParamList::iterator paramEnd = cmdIter->commandParams.end();
            for(; paramIter != paramEnd; ++paramIter)
            {
              if(XMLElement* param = doc.NewElement("Parameter"))
              {
                cmd->InsertEndChild(param);
                param->SetAttribute("Type", paramIter->type);
                param->SetAttribute("Size", paramIter->size);
                param->SetAttribute("Name", paramIter->name);
              }
            }
          }
        }
      }
    }
  }
}
""")


def writeVulkanFunctionPointerTable(sorted_commands, output):
    output.write('''
struct { const char* name; PFN_vkVoidFunction pFunction;} vulkan_lut[] = {
''')
    for c in sorted_commands:
        output.write('    {{"{name}", reinterpret_cast<PFN_vkVoidFunction>({function_name})}},\n'.format(name=c, function_name=c[2:]))

    output.write('''    {NULL, NULL}
};
''')

    output.write('''

  PFN_vkVoidFunction FindFunctionInThisLayer(const char* name)
  {
    int index = 0;
    const char* table_name = vulkan_lut[index].name;

    while(table_name)
    {
      if(0 == strcmp(table_name, name))
      {
        return vulkan_lut[index].pFunction;
      }
      index++;
      table_name = vulkan_lut[index].name;
    }

    return NULL;
  }
''')


def writeTypedefsForGlApi(sorted_commands, api_xml, output):
    for c in sorted_commands:
        output.write(api_xml.funcs[c].typedefDeclaration())
        output.write("\n")  # self, prepend):

def makeCommandIdDefine(command):
    return "ID_" + command

def writeCommandIds(sorted_commands, output, startIndex = 0):
    index = startIndex
    for c in sorted_commands:
        output.write('#define ')
        output.write(makeCommandIdDefine(c))
        output.write(' ')
        output.write(str(index))
        output.write('\n')
        index += 1

    return index

def writeTableHeaderDefines(sorted_commands, prepend, output):
    output.write('''

  extern char const * CommandNames[];
  // If this is const it gets optimzed away breaking linking
  extern int CommandCount;
''')

    for c in sorted_commands:
        output.write('#define ')
        output.write(prepend)
        output.write(c)
        output.write(' (CommandNames[')
        output.write(makeCommandIdDefine(c))
        output.write('])\n')

    output.write("\n")

def writeInitFunctionPointers(all_commands, permitted_commands, api_xml, api, dll_library, cpp):

    cpp.write('''
extern void LogPrint(const char *format, ...);

int %s_InitFunctionPointers()
{
  /* RTLD_LOCAL would be better. But it wont work as libGL.so loads the driver *.so
   * and exposes symbols to it.
   */
    void *dl = OpenLibrary(%s);

    if(!dl)
    {
        LogPrint("%%s\\n", dlerror());
        return -1;
    }
    else
    {
        LogPrint(" Loaded library: %%s\\n", %s);
    }
''' % (api, dll_library, dll_library))

    def writeCommand(c):
        f = api_xml.funcs[c]
        cpp.write('    original_%s = %sLookupSymbol(dl, pChar_%s);\n' % (c, "(Type_%s)" % f.name, c))

    # lookup functions we want to put first
    skip_indices = []
    if api == "egl":
        c = "eglGetProcAddress"
        index = all_commands.index(c)
        if index > 0:
            writeCommand(c)
            skip_indices.append(index)

    for index, c in enumerate(all_commands):
        writeCommand(c)

    cpp.write('''
    return 0;
}
''')


def writeWrapperFile(config):
    directory = config.directory
    name = config.name
    command_excludes = config.exclude

    if not name:
        name = "generated"

    if directory:
        include_header_path ="%s.h" % name.strip()
        header = open(os.path.join(directory, include_header_path), "w")
        cpp = open(os.path.join(directory, "%s.cpp" % name), "w")
    else:
        include_header_path ="STDOUT"
        header = sys.stdout
        cpp = sys.stdout

    header.write('''
//
// Autogenerated file from %s
//
#ifndef %s_AUTOGENERATED_API_SHIM
#define %s_AUTOGENERATED_API_SHIM
''' % (sys.argv[0], name.upper(), name.upper()))

    cpp.write('''
//
// Autogenerated file from %s
//
#include "%s"

#include <sys/types.h>
#include <string.h> // strcmp for FindFunctionId

extern void* OpenLibrary(const char*path);
extern void* LookupSymbol(void* library, const char* name);

''' % (sys.argv[0], include_header_path))

    header.write('''
#include <dlfcn.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <map>

#include "export.h"
#include "tinyxml2.h"

#include "logger.h"
#include "api_id.h"

extern "C" {
extern void* CommandPointers[];
extern void InitCommandPointers(void);
}; // extern C"

''')

    # 'name:xml_file'
    # 'name(version):xml_file'
    # 'name(version):xml_file:alternative_loadlib'
    functionIndex = 0
    functionNames = []
    functionSignatures = {}
    apis = []
    vulkan_enabled = False
    for spec in config.source:
        dll = None
        version = None
        split = spec.split(":")
        if len(split) > 2:
            dll = split[2]
        pos = split[0].find('(')
        if pos > 0:  # has version
            api = split[0][: pos]
            version = split[0][ pos+1: split[0].find(')') ]
            version = [int(x) for x in version.split(".")]
        else:
            api = split[0]

        apis.append(api)

        api_xml_class = ApiXmlClass(api)
        api_xml = api_xml_class()
        if config.android_abi:
            api_xml.android_abi = config.android_abi
            if "-" in config.android_platform:
                # "android-9"
                api_xml.android_platform = int(config.android_platform.split("-")[1])
            else:
                # or "9"
                api_xml.android_platform = int(config.android_platform)

        api_xml.read(expandUser(split[1]))
        names, permitted_names = _writeWrapperFile(api_xml, cpp, header, api, version, dll, command_excludes, functionIndex)

        functionIndex += len(names)
        functionNames.extend(names)
        functionSignatures.update(api_xml.funcs)
        if api == "vulkan":
            vulkan_enabled = True

    cpp.write('''
extern "C" {{
void* CommandPointers[{}] ;

void InitCommandPointers(void)
{{
'''.format(len(functionNames)))
    for c in functionNames[:-1]:
        if c in command_excludes:
            cpp.write('  CommandPointers[{}] = NULL;\n'.format(makeCommandIdDefine(c)))
        else:
            cpp.write('  CommandPointers[{}] = reinterpret_cast<void*>(shim_{});\n'.format(makeCommandIdDefine(c), c))

    c = functionNames[-1]
    if c in command_excludes:
        cpp.write('  CommandPointers[{}] = NULL;\n'.format(makeCommandIdDefine(c)))
    else:
        cpp.write('  CommandPointers[{}] = reinterpret_cast<void*>(shim_{});\n'.format(makeCommandIdDefine(c), c))

    cpp.write('''
}
''')

    cpp.write('''
}; // extern "C"
''')
    if directory:
        cpp_names = open(os.path.join(directory, "%s_names.cpp" % name), "w")
    else:
        cpp_names = sys.stdout

    cpp_names.write('''
//
// Autogenerated file from %s
//
// If this is const it gets optimzed away breaking linking
int CommandCount = %d;
char const * CommandNames[%d] = {
''' % (sys.argv[0], len(functionNames), len(functionNames)))

    for c in functionNames[:-1]:
        cpp_names.write('  "')
        cpp_names.write(c)
        cpp_names.write('",\n')

    cpp_names.write('  "')
    cpp_names.write(functionNames[-1])
    cpp_names.write('"\n')
    cpp_names.write('};\n')

    cpp_names.write('''
''')

    cpp.write('''
namespace {{

}} // anon namespace

void ShimBeginCommandNoOp(int /*api*/, int /*id*/)
{{
}}

void ShimEndCommandNoOp(int /*api*/, int /*id*/)
{{
}}

void (*ShimBeginCommand)(int api, int id) = ShimBeginCommandNoOp;
void (*ShimEndCommand)(int api, int id) = ShimEndCommandNoOp;

int FindFunctionId(const char* str)
{{
  for(int i = 0; i < CommandCount; i++)
    if( 0 == strcmp(CommandNames[i], str) )
    {{
      return i;
    }}

  // else not found so
  return -1;
}}


'''.format())

    header.write("""

// called at begin/end of a command
extern void (*ShimBeginCommand)(int api, int id);
extern void (*ShimEndCommand)(int api, int id);

extern void ShimBeginCommandNoOp(int api, int id);
extern void ShimEndCommandNoOp(int api, int id);

struct ParamsHeader {
 uint64_t frameNumber;
 uint64_t drawNumber;
 uint64_t threadId;
}__attribute__((packed));

extern void ShimStoreParameters( ParamsHeader& params, size_t noBytes);

extern int FindFunctionId(const char* str);

#endif // header
""")

    header.write("""
""".format())

    writeXmlCommandParameterFunction(functionNames, functionSignatures, apis, header, cpp)


def api_to_apiid_define(api):
    return {"gles2": "APIID_GLES2",
            "gles3": "APIID_GLES3",
            "egl": "APIID_EGL",
            "gl": "APIID_GL",
            "cl": "APIID_CL",
            "vulkan": "APIID_VULKAN"}[api]


def _writeWrapperFile(api_xml, cpp, header, api, version, dll, excluded_commands, functionIndex):
    #
    # default libaries are symbol names. If dll is supplied its expected to be the path
    #
    if api == "gles2":
        api_id = api_to_apiid_define(api)
        api_headers = '''
#include <GLES2/gl2.h>
#define GENERATED_GLES2
''' + api_xml.includes["inttypes"].text
        default_library = "GLES2_LIBRARY_PATH" # "/system/lib/libGLESv2.so"

    if api == "gles3":
        api_id = api_to_apiid_define(api)
        api_headers = '''
#include <GLES3/gl3.h>
#define GENERATED_GLES3
''' + api_xml.includes["inttypes"].text
        default_library = "GLES3_LIBRARY_PATH" # "/system/lib/libGLESv3.so"

    if api == "egl":
        api_id = api_to_apiid_define(api)
        api_headers = '''
#include <EGL/egl.h>
#define GENERATED_EGL
#define MGB_FORCE_INTTYPES 1
typedef void *EGLLabelKHR;
typedef khronos_ssize_t EGLsizeiANDROID;
'''
        default_library = "EGL_LIBRARY_PATH" #"/system/lib/libEGL.so"

    if api == "gl":
        api_id = api_to_apiid_define(api)
        api_headers = '''
#include <GL/gl.h>
#define GENERATED_GL
#include <KHR/khrplatform.h>
''' + api_xml.includes["inttypes"].text
        default_library = "GL_LIBRARY_PATH" # "libGL.so.1"

    if api == "cl":
        api_id = api_to_apiid_define(api)
        api_headers = '''
#include <CL/cl.h>
#define GENERATED_CL
''' + api_xml.includes["inttypes"].text
        default_library = "CL_LIBRARY_PATH" # "libCL.so.1"

    if api == "vulkan":
        api_id = api_to_apiid_define(api)
        api_headers = '''
#include "vulkan/vulkan.h"
#define GENERATED_VULKAN
#include "vulkan_dispatch.h"

PFN_vkVoidFunction FindFunctionInThisLayer(const char* name);

'''
        ## defines = api_xml.includes

        ## + api_xml.includes["inttypes"].text
        default_library = "VULKAN_LIBRARY_PATH" # "libCL.so.1"

    if dll:
        dll_library = '"%s"' % dll
    else:
        dll_library = "%s.c_str()" % default_library
        dll_library_as_symbol = default_library

    if api == "gles3":
        # gles2 without specifying the version is the latest 'gles2' style api
        # ie 3.2
        all_commands = getCommandList(api_xml, "gles2", version)
    else:
        all_commands = getCommandList(api_xml, api, version)

    permitted_commands = []
    for c in all_commands: # @todo can we do this with a set?
        if c not in excluded_commands:
            permitted_commands.append(c)

    api_xml.addFunctionSignatures(all_commands, api_id)

    header.write(api_headers)

    header.write("\n")

#    header.write('''
#static int %s_InitFunctionPointers();
#
#''' % api)

    # GL types needed for gl functions
    api_xml.writeGlApiTypes(header)

    # Gl function types as typedefs (with prepend)
    writeTypedefsForGlApi(all_commands, api_xml, header)

    writeCommandIds(all_commands, header, functionIndex)

    writeTableHeaderDefines(all_commands, "pChar_", header)

    if api != "vulkan":
        for c in all_commands:
            f = api_xml.funcs[c]
            header.write("extern ")
            header.write(f.pointerDeclaration(prepend="original_"))
            header.write(";\n")

    ## if api != "vulkan":
    ##     writeCommandPointersForHeader(permitted_commands, api_xml, "_", header)

    header.write("\n")

    if api != "vulkan":
        # function pointers for dl open ie 'glProgram' -> '(*_glProgram) = NULL'
        for c in all_commands:
            f = api_xml.funcs[c]
            cpp.write(f.pointerVariable(storageClass="", prepend="original_")) # to NULL
            cpp.write("\n")

    ## if api != "vulkan":
    ##     writeCommandFunctionPointers(permitted_commands, api_xml, "_", cpp)

    # vulkan doesnt need InitFunctionPointer()
    if api != "vulkan":
        writeInitFunctionPointers(all_commands, permitted_commands, api_xml, api, dll_library, cpp)

    cpp.write('''
extern "C" {
''')

    # Exported wrapper functions
    writeExportedFunctions(all_commands, excluded_commands, api, api_xml, api_id, cpp)

    cpp.write('''
} // extern "C"
''')

    if api == "vulkan":
        writeVulkanFunctionPointerTable(permitted_commands, cpp)

    return all_commands, permitted_commands

def expandUser(path):
    try:
        return os.path.expanduser(path)
    except:
        return ""


def getConfiguration():
    import argparse
    import configparser

    # allow a config file to override the defaults
    parser = argparse.ArgumentParser(
        description="GPU API Shim/wrapper generator",
        # these are needed to handle '--help' properly with argparse split over
        # config reading
        formatter_class=argparse.RawDescriptionHelpFormatter,
        add_help=False
        )
    parser.add_argument("-c", "--config",
                        help="Specify config file", metavar="FILE")
    args, remaining_argv = parser.parse_known_args()

    # some of these defaults are expected to be there
    # (passed on as globalconfig and config)
    defaults = {"directory": None,
                "name": None,
                "android_abi": None,        # eg armabi
                "android_platform": None,   # eg android-9
                }

    name, _ = os.path.splitext(sys.argv[0])
    filename = name + ".cfg"
    if os.path.isfile(filename):  # side by side config file
        args.config = filename

    # set the defaults
    if args.config:
        config = configparser.SafeConfigParser()
        config.read([args.config])
        defaults.update(dict(config.items("Defaults")))

    # allow the command line to override
    parser = argparse.ArgumentParser(parents=[parser])
    parser.set_defaults(**defaults)
    parser.add_argument("--directory", type=str, help="Directory path to write to. Stdout if unset")
    parser.add_argument("--name", type=str, help="Postfix filename ie {directory}/{name}.cpp/h. Default is %s" % defaults["name"])
    parser.add_argument("--android-abi", type=str, help="Android ABI. Default is %s" % defaults["android_abi"])
    parser.add_argument("--android-platform", type=str, help="Android Platform. Default is %s" % defaults["android_platform"])


    parser.add_argument("--exclude",
                        action='append', default=[],
                        help="Exclude a function by name(for manual overrides). Multiple allowed")

    parser.add_argument("--source",
                        action='append', default=[],
                        help="Colon separated source  ie 'name:xml_file', 'name(version):xml_file' 'name(version):xml_file:alternative_loadlib' . Multiple allowed")

    parser.add_argument("--apiid", type=int, help="APIID of a particular ID. 0 if unset")

    ns = parser.parse_args(remaining_argv)

    return ns


if __name__ == "__main__":

    config = getConfiguration()

    writeWrapperFile(config)
