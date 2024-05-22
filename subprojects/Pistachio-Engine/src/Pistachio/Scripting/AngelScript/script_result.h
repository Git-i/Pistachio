#pragma once
namespace Pistachio
{
    namespace Scripting
    {
        namespace Base
        {
            /*
                Script Results:
                0 - Executed As Expexted
                -- Positive Results Mean Execution Completed but there are some things to note --
                1 - File couldn't be opened with specified mode
            */
            class ScriptResult
            {
            public:
                ScriptResult(int _code) : code(_code) {}
                int code;
            };
        }
    }
}