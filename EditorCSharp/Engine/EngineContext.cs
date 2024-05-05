using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using PistachioCS;

namespace EditorCSharp.Engine
{
    internal static class EngineContext
    {
        private static PistachioCS.Application? _application;
        private static bool initialized = false;

        static void Initialize()
        {
            if(!initialized)
            {
                _application = new()
            }
        }

    }
}
