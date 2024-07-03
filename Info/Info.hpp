#pragma once

#include <string>

namespace Ocl{
    template<typename...>class Info;
    template<typename ...Args>
    Info(Args &&...)->Info<Args...>; 

    template<typename...Is>
    requires std::convertible_to<Ocl::Info<Is...> const&, std::string const&>
    auto & operator <<(std::basic_ostream<char> &lhs, Ocl::Info<Is...> const&rhs){
        return lhs << (std::string const&)rhs;
    }

    template<typename...Is>
    requires std::convertible_to<Ocl::Info<Is...> const&, std::array<size_t, 3> const&>
    auto & operator <<(std::basic_ostream<char> &lhs, Ocl::Info<Is...> const&rhs){
        std::array<size_t, 3> const& array = rhs;
        for(auto &&e : array)
            lhs << e << " ";
        return lhs;
    }
}