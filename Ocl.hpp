#pragma once

#include <CL/Cl.h>

#include<array>

namespace Ocl{
    template<auto...>class Value{};

    template<typename T, template<typename...>typename TT>
    class EqualT : public std::false_type{};

    template<template<typename...>typename TT, typename ...Ts>
    class EqualT<TT<Ts...>, TT> : public std::true_type{};

    template<typename...>class IsTRank1{};
    template<template<typename...>typename TT, typename T0, typename...Ts>
    class IsTRank1<TT<T0, Ts...>> : public std::true_type{ public: using First = T0; constexpr static size_t size = 1 + sizeof...(Ts); };

    template<template<typename...>typename TT, typename T0, typename T1, typename...Ts>
    class IsTRank1<TT<T0, T1, Ts...>> : public std::true_type{ public: using First = T0; using Second = T1; constexpr static size_t size = 2 + sizeof...(Ts); };

    template<typename T>
    class IsTRank1<T> : public std::false_type{public: constexpr static size_t size = 0;};

    template<typename...>class IsTVRank1{};
    template<template<typename, auto, typename...>typename TT, typename T, auto V, typename...Ts>
    class IsTVRank1<TT<T, V, Ts...>> : public std::true_type{ public: using First = T; constexpr static decltype(V) second = V; };

    template<typename T>
    class IsTVRank1<T> : public std::false_type{};

    template<typename...>class Id;
    template<typename ...Args>
    Id(Args &&...)->Id<Args...>;

    template<typename...>class Enum;
    template<typename ...Args>
    Enum(Args &&...)->Enum<Args...>;   

    template<typename...>class Version;
    template<typename ...Args>
    Version(Args &&...)->Version<Args...>;

    template<typename...>class Read;
    template<typename ...Args>
    Read(Args &&...)->Read<Args...>;

    template<typename...>class Write;
    template<typename ...Args>
    Write(Args &&...)->Write<Args...>;

    template<typename...>class Size;
    template<typename ...Args>
    Size(Args &&...)->Size<Args...>;

    template<typename...Gs>
    class Size{
        std::array<size_t, sizeof...(Gs)> sizes;
    public:
        Size(auto &&...sizes) : sizes({sizes...}){}

        operator size_t const *()const{
            return &sizes[0];
        }
    };

    template<typename...>class SizeOf;
    template<typename ...Args>
    SizeOf(Args &&...)->SizeOf<Args...>;

    template<typename...Gs>
    class SizeOf{
        size_t size;
    public:
        SizeOf(auto &&...counts) : size(((sizeof(Gs)*counts)+...)){}
        SizeOf() : size((sizeof(Gs)+...)){}

        operator size_t ()const{
            return size;
        }
    };
}

#include "Info/Info.hpp"
#include "Query/Query.hpp"
#include "Result/Result.hpp"
#include "Handle/Handle.hpp"
#include "Event/Event.hpp"
#include "Platform/Platform.hpp"
#include "Device/Device.hpp"
#include "CommandQueue/CommandQueue.hpp"
#include "Sources/Sources.hpp"
#include "Program/Program.hpp"
#include "Context/Context.hpp"
#include "Memory/Memory.hpp"
#include "Kernel/Kernel.hpp"