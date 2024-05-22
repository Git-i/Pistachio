#pragma once
#include "angelscript.h"
#include "scriptstdstring.h"
#include "scriptarray.h"
#include <unordered_map>
namespace Pistachio 
{
    namespace Scripting
    {
        typedef std::string(*print_fn_t)(void*);
        extern asIScriptEngine* engine;
        extern asITypeInfo* debugInterface;
        extern std::unordered_map<int, print_fn_t> print_function_map;
    }
}