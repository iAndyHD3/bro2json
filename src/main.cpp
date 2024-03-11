#include <broma.hpp>
#include <matjson.hpp>
#include "popl.hpp"
#include <filesystem>
#include <matjson/stl_serialize.hpp>

std::optional<ptrdiff_t> n_forPlatform(const broma::PlatformNumber& n, broma::Platform platform)
{
    ptrdiff_t ret = -1;
    switch(platform)
    {
        case broma::Platform::Windows: ret = n.win; break;
        case broma::Platform::Mac: ret = n.mac; break;
        case broma::Platform::Android64: ret = n.android64; break;
        case broma::Platform::Android32: ret = n.android32; break;
        case broma::Platform::iOS: ret = n.ios; break;
        default: return {}; break;
    }

    if(ret == -1)
    {
        return {};
    }
    
    return ret;
}


namespace matjson
{
    template<>
	struct Serialize<broma::Platform>
    {
        static Value to_json(broma::Platform p)
        {
            switch(p)
            {
            case broma::Platform::Windows: return "win";
            case broma::Platform::Mac: return "mac";
            case broma::Platform::Android64: return "android64";
            case broma::Platform::Android32: return "android32";
            case broma::Platform::Android: return "android";
            case broma::Platform::iOS: return "ios";
            case broma::Platform::None: return "none";
            }
        }
    };
}


void setIfNotEmpty(matjson::Array& arr, const matjson::Value& val)
{
    auto helper = [&](const auto& hasempty){
        if(!hasempty.empty()) arr.push_back(val);
    };

    switch(val.type())
    {
        case matjson::Type::Array: return helper(val.as_array());
        case matjson::Type::Object: return helper(val.as_object());
        case matjson::Type::String: return helper(val.as_string());
        case matjson::Type::Bool: return val.as_bool() ? arr.push_back(val) : [](){}();
        default: return;
    }
}



void setIfNotEmpty(matjson::Object& obj, std::string_view key, const matjson::Value& val)
{
    auto helper = [&](const auto& hasempty){
        if(!hasempty.empty()) obj[key] = val;
    };

    switch(val.type())
    {
        case matjson::Type::Array: return helper(val.as_array());
        case matjson::Type::Object: return helper(val.as_object());
        case matjson::Type::String: return helper(val.as_string());
        default: return;
    }
}

void setIfNotEmpty(matjson::Object& obj, std::string_view key, broma::Platform plat)
{
    if(plat != broma::Platform::None) obj[key] = plat;
}

namespace matjson
{
    template<>
    struct Serialize<broma::PlatformNumber>
    {
        static Value to_json(broma::PlatformNumber number)
        {
            matjson::Object ret;

            auto get_set_n = [&](std::string_view platstr, broma::Platform plat){
                if(auto addr = n_forPlatform(number, plat)) ret[platstr] = *addr;
            };

            get_set_n("win", broma::Platform::Windows);
            get_set_n("mac", broma::Platform::Mac);
            get_set_n("android32", broma::Platform::Android32);
            get_set_n("android64", broma::Platform::Android64);
            get_set_n("ios", broma::Platform::iOS);
            return ret;
        }  
    };

        template<>
    struct Serialize<broma::Type>
    {
        static Value to_json(const broma::Type& fntype)
        {
            matjson::Object ret;
            ret["name"] = fntype.name;
            setIfNotEmpty(ret, "struct", fntype.is_struct);
            return ret;
        }
    };

    template<>
    struct Serialize<std::pair<broma::Type, std::string>>
    {
        static Value to_json(const std::pair<broma::Type, std::string>& arg)
        {
            matjson::Object ret;
            ret["type"] = arg.first;
            ret["name"] = arg.second;
            return ret;
        }
    };
    template<>
    struct Serialize<std::vector<std::pair<broma::Type, std::string>>>
    {
        static Value to_json(const std::vector<std::pair<broma::Type, std::string>>& args)
        {
            matjson::Array ret;
            for(const auto& arg : args)
            {
                ret.emplace_back(arg);
            }
            return ret;
        }
    };


    template<>
    struct Serialize<broma::Attributes>
    {
        static Value to_json(const broma::Attributes& atr)
        {
            matjson::Object ret;
            setIfNotEmpty(ret, "depends", atr.depends);
            setIfNotEmpty(ret, "docs", atr.docs);
            setIfNotEmpty(ret, "links", atr.links);
            setIfNotEmpty(ret, "missing", atr.missing);
            return ret;
        }
    };

    template<>
    struct Serialize<broma::FunctionProto>
    {
        static Value to_json(const broma::FunctionProto& f)
        {
            matjson::Object ret;
            setIfNotEmpty(ret, "arguments", f.args);
            setIfNotEmpty(ret, "attributes", f.attributes);
            ret["name"] = f.name;
            ret["ret"] = f.ret;
            return ret;
        }
    };


    template<>
    struct Serialize<broma::Function>
    {
        static Value to_json(const broma::Function& fn)
        {
            matjson::Object ret;
            setIfNotEmpty(ret, "addresses", fn.binds);
            setIfNotEmpty(ret, "prototype", fn.prototype);
            return ret;
        };
    };

    template<>
    struct Serialize<broma::FunctionType>
    {
        static Value to_json(const broma::FunctionType& type)
        {
            switch(type)
            {
                case broma::FunctionType::Ctor: return "ctor";
                case broma::FunctionType::Dtor: return "dtor";
                case broma::FunctionType::Normal: return "normal";
            }
        }
    };



    template<>
    struct Serialize<broma::MemberFunctionProto>
    {
        static Value to_json(const broma::MemberFunctionProto& fn)
        {
            matjson::Object ret;

            ret["name"] = fn.name;
            setIfNotEmpty(ret, "type", fn.type);
            setIfNotEmpty(ret, "ret", fn.ret);

            setIfNotEmpty(ret, "arguments", fn.args);

            
            if(fn.attributes.links != broma::Platform::None) ret["links"] = fn.attributes.links;
            if(fn.attributes.missing != broma::Platform::None) ret["missing"] = fn.attributes.missing;

            if(!fn.attributes.depends.empty()) ret["depends"] = fn.attributes.depends;
            if(fn.is_callback) ret["callback"] = true;
            if(fn.is_const) ret["const"] = true;
            if(fn.is_static) ret["static"] = true;
            if(fn.is_virtual) ret["virtual"] = true;
            
            return ret;
        }
    };

    template<>
    struct Serialize<broma::FunctionBindField>
    {
        static Value to_json(const broma::FunctionBindField& fn)
        {
            matjson::Object ret;
            setIfNotEmpty(ret, "binds", fn.binds);
            ret["prototype"] = fn.prototype;
            ret["type"] = "function";
            return ret;
        }
    };

    template <>
    struct Serialize<broma::Field>
    {
        static Value to_json(const broma::Field& field)
        {
            if(auto* fnp = std::get_if<broma::FunctionBindField>(&field.inner))
            {
                return *fnp;
            }
            
            return matjson::Object{};
        }
    };


    template <>
    struct Serialize<std::vector<broma::Field>>
    {
        static Value to_json(const std::vector<broma::Field>& fields)
        {
            matjson::Array ret;
            ret.reserve(fields.size());
            for(const auto& field : fields)
            {
                setIfNotEmpty(ret, field);
            }
            return ret;
        }
    };


    template<>
    struct Serialize<broma::Class>
    {
        static Value to_json(const broma::Class& cl)
        {
            matjson::Object ret;
            ret["name"] = cl.name;

            setIfNotEmpty(ret, "attributes", cl.attributes);
            setIfNotEmpty(ret, "inherits", cl.superclasses);
            setIfNotEmpty(ret, "fields", cl.fields);
            return ret;
        }
    };

    template <>
    struct Serialize<std::vector<broma::Class>>
    {
        static Value to_json(const std::vector<broma::Class>& classes)
        {
            matjson::Array ret;
            ret.reserve(classes.size());
            for(const auto& cl : classes)
            {
                ret.emplace_back(cl);
            }
            return ret;
        }
    };

    template<>
    struct Serialize<broma::Root>
    {
        static Value to_json(const broma::Root& root)
        {
            matjson::Object ret;
            ret["classes"] = root.classes;

            matjson::Array freefuncs;
            for(const auto& f : root.functions)
            {
                freefuncs.emplace_back(f);
            }
            return ret;
        }
    };
}

using namespace broma;


int main(int argc, char* argv[])
{
    popl::OptionParser parser;

    auto help_option = parser.add<popl::Switch>("h", "help", "show this help message");
    auto broma_option = parser.add<popl::Value<std::string>>("b", "broma", "full path to broma file");

    auto indent_option = parser.add<popl::Value<int>>("i", "indentation", "how many levels to indent (default: 0)");

    parser.parse(argc, argv);

    
    if(help_option->is_set() || !broma_option->is_set())
    {
        std::cout << parser;
        return 0;
    }

    std::filesystem::path broma_path(broma_option->value());

    if(!std::filesystem::exists(broma_path)) 
    {
        std::cout << "Path does not exist";
        return 1;
    }
    else if(!std::filesystem::is_regular_file(broma_path))
    {
        std::cout << "Path is not a file";
        return 1;
    }

    try
    {
        matjson::Value v = broma::parse_file(broma_path.string());

        int indent = indent_option->is_set() ? indent_option->value() : 0;
        if(indent < 0) indent = 0;
        std::cout << v.dump(indent);
    }
    catch(std::exception& e)
    {
        std::cout << e.what();
        return 1;
    }
}