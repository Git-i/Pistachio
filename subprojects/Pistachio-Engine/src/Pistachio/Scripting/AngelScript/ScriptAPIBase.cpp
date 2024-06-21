#include "Pistachio/Core.h"
#include "Pistachio/Core/Log.h"
#include "Pistachio/Core/UUID.h"
#include "ptpch.h"
#include "angelscript.h"
#include "scriptstdstring.h"
#include "scriptarray.h"
#include <algorithm>
#include <cstdint>
#include <regex>
#include <string>
#include <unordered_map>
#include "ScriptAPICommon.h"
namespace Pistachio 
{
    namespace Scripting
    {
        asIScriptEngine* engine;
        asITypeInfo* debugInterface;
        std::unordered_map<int, print_fn_t> print_function_map;
        namespace Base 
        {
            std::string as_var_to_string(std::pair<int, void*> entry)
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
                        asITypeInfo* info = engine->GetTypeInfoById(entry.first);
                        //handle enums
                        if(uint32_t numVals = info->GetEnumValueCount(); numVals)
                        {
                            std::unordered_map<int, std::string> enum_to_string;
                            for(uint32_t i = 0; i < numVals; i++)
                            {
                                int value;
                                const char* name = info->GetEnumValueByIndex(i, &value);
                                if(value == *(int*)entry.second) return name;
                            }
                        }
                        //handle application types
                        const char* name = info->GetName();
                        if(strcmp(name, "array") == 0)
                        {
                            CScriptArray* arr = (CScriptArray*)entry.second;
                            std::string returnVal = "[";
                            uint32_t size = arr->GetSize();
                            int tid = arr->GetElementTypeId();
                            for(uint32_t i = 0; i < size-1; i++)
                            {
                                returnVal += as_var_to_string(std::pair<int, void *>{tid, arr->At(i)});
                                returnVal += ", ";
                            }
                            if(size) returnVal += as_var_to_string(std::pair<int, void *>{tid, arr->At(size-1)});
                            returnVal += "]";
                            return returnVal;
                        }
                        return print_function_map[entry.first](entry.second);
                    }
                }
                return "error";
            }
            std::string replace_var(const std::string& in, std::unordered_map<std::string, std::pair<int,void*>>& map)
            {
                auto it = map.find(in);
                if(it == map.end()) return in;
                auto& entry = (*it).second;
                return as_var_to_string(entry);

            }
            std::string format(const std::string& msg)
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
                    std::sregex_iterator(msg.begin(), msg.end(), var_replace_regex);
                auto words_end = std::sregex_iterator();
                std::smatch match;
                for(auto i = words_begin; i != words_end; ++i)
                {
                    match = *i;
                    final_str += match.prefix();
                    auto match_str = match.str();
                    if(msg[match.position()-1] == '\\')
                    {
                        final_str.erase(final_str.end()-1);
                        final_str += match_str;
                    } 
                    else final_str += replace_var(std::string(match_str.begin()+1, match_str.end()-1), type_names_to_id);
                }
                if(!match.empty()) final_str += match.suffix();
                else final_str = msg;
                return final_str;
            }
            void print(const std::string& msg)
            {
                PT_INFO(format(msg));
            }
            void UUID_Construct(void* memory)
            {
                new(memory)UUID;
            }
            void UUID_Construct_ID(uint64_t id, void* memory)
            {
                new(memory)UUID(id);
            }
            void UUID_Construct_UUID(const UUID& other, void* memory)
            {
                new(memory)UUID(other);
            }
            //angelscript reclaims its memory
            void UUID_Destruct(void* memory){}
        }
        //TODO: Customize this Message Callback
        void MessageCallback(const asSMessageInfo *msg, void *param)
        {
          const char *type = "ERR ";
          if( msg->type == asMSGTYPE_WARNING ) 
            type = "WARN";
          else if( msg->type == asMSGTYPE_INFORMATION ) 
            type = "INFO";
          printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
        }
        void Initialize()
        {
            engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
            engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);
            RegisterStdString(engine);
            RegisterScriptArray(engine, true);
            RegisterStdStringUtils(engine);
            int r;
            r = engine->RegisterInterface("IDebug"); PT_CORE_ASSERT(r >= 0);
            r = engine->RegisterInterfaceMethod("IDebug", "string ToString()");PT_CORE_ASSERT(r >= 0);

            r = engine->RegisterGlobalFunction("void print(const string &in msg)", asFUNCTION(Base::print), asCALL_CDECL); PT_CORE_ASSERT(r >= 0);
            r = engine->RegisterGlobalFunction("string format(const string &in msg)", asFUNCTION(Base::format), asCALL_CDECL); PT_CORE_ASSERT(r >= 0);
            debugInterface = engine->GetTypeInfoByName("IDebug");

            //Pistachio Specific Stuff
            r = engine->SetDefaultNamespace("Pistachio"); PT_CORE_ASSERT(r >= 0);
            r = engine->RegisterObjectType("UUID", sizeof(Pistachio::UUID), asOBJ_VALUE|asGetTypeTraits<UUID>()); PT_CORE_ASSERT(r >= 0);
            r = engine->RegisterObjectBehaviour("UUID", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Base::UUID_Construct), asCALL_CDECL_OBJLAST); PT_CORE_ASSERT(r >= 0);
            r = engine->RegisterObjectBehaviour("UUID", asBEHAVE_CONSTRUCT, "void f(uint64 ID)", asFUNCTION(Base::UUID_Construct), asCALL_CDECL_OBJLAST); PT_CORE_ASSERT(r >= 0);
            r = engine->RegisterObjectBehaviour("UUID", asBEHAVE_CONSTRUCT, "void f(const UUID &in ID)", asFUNCTION(Base::UUID_Construct), asCALL_CDECL_OBJLAST); PT_CORE_ASSERT(r >= 0);
            r = engine->RegisterObjectBehaviour("UUID", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Base::UUID_Destruct), asCALL_CDECL_OBJLAST); PT_CORE_ASSERT(r >= 0);
            r = engine->RegisterObjectMethod("UUID", "bool opEquals(const UUID& in) const", asMETHODPR(UUID, operator==, (const UUID& other) const, bool), asCALL_THISCALL); PT_CORE_ASSERT(r >= 0);
            r = engine->SetDefaultNamespace(""); PT_CORE_ASSERT(r >= 0);
        }
    }
}