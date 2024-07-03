#pragma once

#include <string_view>

namespace Ocl{
    template<typename...>class Result;
    template<typename ...Args>
    Result(Args &&...)->Result<Args...>;

    template<>
    class Result<cl_int>{
    protected:
        cl_int result;
    public:
        Result() : Result(CL_SUCCESS) {}
        Result(cl_int result) : result(result){}

        operator cl_int()const{
            return result;
        }

        operator cl_int *(){
            return &result;
        }

        operator bool()const{
            return result == CL_SUCCESS;
        }

        template<typename _Result>
        requires std::same_as<std::remove_cvref_t<_Result>, Result>
        auto & operator=(_Result rhs){
            return *new(this)Result(decltye(rhs)(rhs));
        }
    };

    template<typename _Result>
    requires (EqualT<std::remove_cvref_t<_Result>, Result>() == true || std::convertible_to<_Result, Result<cl_int> const&>)
    class Info<_Result>{
        _Result result;
    public:
        Info(auto &&result) : result(decltype(result)(result)){}

        operator std::string_view()const{
            switch((cl_int)(Result<cl_int>const&)result){
            case CL_OUT_OF_HOST_MEMORY:
                return {"CL_OUT_OF_HOST_MEMORY"};
            case CL_SUCCESS:
                return {"CL_SUCCESS"};
            case CL_INVALID_VALUE:
                return {"CL_INVALID_VALUE"};
            case CL_INVALID_GLOBAL_WORK_SIZE:
                return {"CL_INVALID_GLOBAL_WORK_SIZE"};
            case CL_INVALID_PROGRAM:
                return { "CL_INVALID_PROGRAM" };
            default:
                return {"CL_UNKNOWN"};
            }
        }
    };

    template<typename _Result>
    requires (EqualT<std::remove_cvref_t<_Result>, Result>() == true || std::convertible_to<_Result, Result<cl_int> const&>)
    auto & operator <<(std::basic_ostream<char> &lhs, Info<_Result> const & rhs){
        return lhs << (std::string_view)rhs;
    }
}
