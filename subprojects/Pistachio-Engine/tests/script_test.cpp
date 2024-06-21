#include "Pistachio/Scene/Entity.h"
#include "ptpch.h"
#include "Pistachio/Core/Application.h"
#include "Pistachio/Scene/Scene.h"
#include "angelscript.h"
#include "scriptarray.h"
#include "scriptstdstring.h"
#include <csignal>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <regex>
#include "Pistachio/Scripting/AngelScript/ScriptAPICommon.h"
namespace Pistachio {
    namespace Scripting {
        extern void Initialize();
        namespace ECS {
            extern void Initialize();
            extern void SetCurrentScene(Scene* scene);
        }
    }
}
asIScriptEngine* engine = Pistachio::Scripting::engine;
asITypeInfo* debugInterface = Pistachio::Scripting::debugInterface;
std::string as_type_to_string(std::pair<int, void*> entry)
{
    switch (entry.first) {
        case asTYPEID_DOUBLE: return std::to_string(*(double*)entry.second);
        case asTYPEID_INT64: return std::to_string(*(asINT64*)entry.second);
        case asTYPEID_UINT64: return std::to_string(*(asQWORD*)entry.second);
        case asTYPEID_BOOL: return *(bool*)entry.second == true ? "true" : "false";
        case asTYPEID_INT16: return std::to_string(*(asINT16*)entry.second);
        case asTYPEID_FLOAT: return std::to_string(*(float*)entry.second);
        case asTYPEID_INT32: return std::to_string(*(asINT32*)entry.second);
        case asTYPEID_INT8: return std::to_string(*(asINT8*)entry.second);
        case asTYPEID_UINT16: return std::to_string(*(asWORD*)entry.second);
        case asTYPEID_UINT32: return std::to_string(*(asDWORD*)entry.second);
        case asTYPEID_UINT8: return std::to_string(*(asBYTE*)entry.second);
        default:
        {
            if(entry.first == engine->GetTypeIdByDecl("string"))return *(std::string*)entry.second;
            if(entry.first & asTYPEID_SCRIPTOBJECT)
            {
                asIScriptObject* obj = (asIScriptObject*)entry.second;
                auto type = obj->GetObjectType();
                if(type->Implements(debugInterface))
                {
                    auto ctx = engine->RequestContext();
                    ctx->Prepare(type->GetMethodByDecl("string ToString()"));
                    ctx->SetObject(obj);
                    ctx->Execute();
                    std::string* res = (std::string*)ctx->GetReturnObject();
                    std::string retVal = *res;
                    ctx->Release();
                    return retVal;
                };
                return "<IDebug not implemented>";
            }
            //handle template types
            asITypeInfo* info = engine->GetTypeInfoById(entry.first);
            if(uint32_t numVals = info->GetEnumValueCount(); numVals)
            {
                std::unordered_map<int, std::string> enum_to_string;
                for(uint32_t i = 0; i < numVals; i++)
                {
                    int value;
                    const char* name = info->GetEnumValueByIndex(i, &value);
                    enum_to_string[value] = name;
                }
                return enum_to_string[*(int*)entry.second];
            }
            const char* name = info->GetName();
            if(strcmp(name, "array") == 0)
            {
                CScriptArray* arr = (CScriptArray*)entry.second;
                std::string returnVal = "[";
                uint32_t size = arr->GetSize();
                int tid = arr->GetElementTypeId();
                for(uint32_t i = 0; i < size-1; i++)
                {
                    returnVal += as_type_to_string(std::pair<int, void *>{tid, arr->At(i)});
                    returnVal += ", ";
                }
                if(size) returnVal += as_type_to_string(std::pair<int, void *>{tid, arr->At(size-1)});
                returnVal += "]";
                return returnVal;
            }
        }
    }
    return "error";
}
std::string replace_var(const std::string& in, std::unordered_map<std::string, std::pair<int,void*>>& map)
{
    auto it = map.find(in);
    if(it == map.end()) return in;
    auto& entry = (*it).second;
    return as_type_to_string(entry);
    
}

struct Stream{};
Stream* pt_info = (Stream*)100;
Stream* pt_warn = (Stream*)200;
Stream* pt_error = (Stream*)300;
class Printer{
public: static void print(const std::string& in, Stream* str)
{
    asIScriptContext* context = asGetActiveContext();
    std::unordered_map<std::string, std::pair<int,void*>> type_names_to_id;
    uint32_t var_count = context->GetVarCount();
    for(uint32_t n = 0; n < var_count; n++)
    {
        const char* name;
        int type_id;
        context->GetAddressOfVar(0,0);
        context->GetVar(n, 0, &name, &type_id);
        type_names_to_id[std::string(name)] = {type_id, context->GetAddressOfVar(n)};
    }
    std::regex var_replace_regex(R"(\{\w+})");
    std::string final_str;
    auto words_begin = 
        std::sregex_iterator(in.begin(), in.end(), var_replace_regex);
    auto words_end = std::sregex_iterator();
    std::smatch match;
    for(auto i = words_begin; i != words_end; ++i)
    {
        match = *i;
        final_str += match.prefix();
        auto match_str = match.str();
        if(in[match.position()-1] == '\\')
        {
            final_str.erase(final_str.end()-1);
            final_str += match_str;
        } 
        else final_str += replace_var(std::string(match_str.begin()+1, match_str.end()-1), type_names_to_id);
    }
    if(!match.empty()) final_str += match.suffix();
    else final_str = in;
    if(str == pt_error)
        std::cout << "error: ";
    if(str == pt_warn)
        std::cout << "warn: ";
    if(str == pt_info)
        std::cout << "info: ";
    if(str == nullptr)
        std::cout << "info: ";
    std::cout << final_str << std::endl;
}
};
int doa(int code)
{
    raise(SIGTRAP);
}
class App : public Pistachio::Application
{
    public:
    App() : Pistachio::Application("")
    {

    }
};
Pistachio::Application* Pistachio::CreateApplication()
{
    return new App();
}
void LineCallback(asIScriptContext* ctx, void* data)
{
    const char* sc;
    int ln = ctx->GetLineNumber(0, 0, &sc);
    if(ln == 25)
    {
        ctx->Suspend();
        uint32_t var_count = ctx->GetVarCount();
        for(uint32_t n = 0; n < var_count; n++)
        {
            const char* name;
            int type_id;
            ctx->GetVar(n, 0, &name, &type_id);
            auto ptr = ctx->GetAddressOfVar(n);
            if(type_id == engine->GetTypeIdByDecl("array<Pistachio::ECS::Entity>"))
            {
                CScriptArray* arr = (CScriptArray*)ptr;
                if(arr->GetElementTypeId() != engine->GetTypeIdByDecl("Pistachio::ECS::Entity")) raise(SIGTRAP);
                Pistachio::Entity* e = (Pistachio::Entity*)arr->At(0);
                if(!e->IsValid()) raise(SIGTRAP);
            }
        }
    }
    int a = 9;
}
int main()
{
    auto app = Pistachio::CreateApplication();
    Pistachio::Scripting::Initialize();
    Pistachio::Scripting::ECS::Initialize();
    engine = Pistachio::Scripting::engine;
    engine->RegisterObjectType("Lpo", 0, asOBJ_REF);
    Pistachio::Scene scene;
    Pistachio::Scripting::ECS::SetCurrentScene(&scene);
    debugInterface = Pistachio::Scripting::debugInterface;
    auto mod=engine->GetModule("mod", asGM_ALWAYS_CREATE);
    const char* code = R"(
        class Custom : IDebug
        {
            string ToString()
            {
                return "Hello";
            }
        }
        class lola : IDebug
        {
            int a;
            lola() {a = 0;}
            lola(int init)
            {
                a = init;
            }
            string ToString()
            {
                return formatInt(a);
            }
        }
        void function()
        {
            int a = 5;
            string lol = format("This is lol {5}");
            print(lol);
        }
    )";
    mod->AddScriptSection("script.as", code);
    mod->Build();
    asIScriptContext* ctx = engine->RequestContext();
    //ctx->SetLineCallback(asFUNCTION(LineCallback), 0, asCALL_CDECL);
    ctx->Prepare(mod->GetFunctionByDecl("void function()"));
    ctx->Execute();
}