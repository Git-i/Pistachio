#pragma once
#include "../../Core.h"

namespace Pistachio
{
    template<typename T>
    class JobHandle
    {
        public:
            bool IsDone();
            T WaitTillDone();
        private:
            JobHandle() = default;
            bool m_done;
    };
}