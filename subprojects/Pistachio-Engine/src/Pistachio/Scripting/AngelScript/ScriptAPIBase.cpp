#include "Pistachio/Core/Log.h"
#include "ptpch.h"
#include "angelscript.h"
#include "scriptstdstring.h"
#include "scriptarray.h"
#include <algorithm>
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
            void print(const std::string& msg)
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

                PT_CORE_INFO(final_str);
            }
        }
        void Initialize()
        {
            engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
            RegisterStdString(engine);
            RegisterStdStringUtils(engine);
            RegisterScriptArray(engine, true);

            engine->RegisterInterface("IDebug");
            engine->RegisterInterfaceMethod("IDebug", "string ToString()");

            engine->RegisterGlobalFunction("void print(const string& msg)", asFUNCTION(Base::print), asCALL_CDECL);
            debugInterface = engine->GetTypeInfoByName("IDebug");
        }
    }
}