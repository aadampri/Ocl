#pragma once

namespace Ocl{
    template<typename...>class CommandBuffer;
    template<typename ...Args>
    CommandBuffer(Args &&...)->CommandBuffer<Args...>;
}