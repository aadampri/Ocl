#pragma once

#include<vector>

namespace Ocl{
    template<typename...>class Platform;
    template<typename ...Args>
    Platform(Args &&...)->Platform<Args...>;

    template<typename...Ps>
    class Id<Platform<Ps...>> : public Handle<cl_platform_id>{
    public:
        Id(){}
        Id(cl_platform_id id) : Handle(id){}
    };

    template<typename ...Ps>
    class Enum<Platform<Ps...>> : public std::vector<Id<Platform<Ps...>>>{
        Result<cl_int> result;

       public:
         Enum(Platform<Ps...>&&) : result(
            [](auto &&ids){
                cl_uint size = 0;
                if(auto &&result = Result(clGetPlatformIDs(0, nullptr, &size)); result){
                    ids->resize(size);
                    cl_platform_id &id = (*ids)[0];
                    return Result(clGetPlatformIDs(ids->size(), &id, nullptr));
                }
                else return result;
            }(this)){}
    };

    template<typename...Ps, template<auto>typename Query, auto query>
    class Info<Id<Platform<Ps...>>&, Query<query>>{
        std::string desc;
    public:
        Info(Id<Platform<Ps...>> &id, Query<query> &&) : desc([](auto &&id){
            size_t size = 0;
            if(auto &&result = Result(clGetPlatformInfo(id, query, 0, nullptr, &size)); !result) return std::string("");
            auto desc = std::string(size - 1, 0);
            if(auto &&result = Result(clGetPlatformInfo(id, query, size, (void *)desc.c_str(), nullptr)); result) return desc;
            return std::string("");
        }(id)){}

        operator std::string const&()const{
            return desc;
        }
    };

 template<typename...Ps, template<auto>typename Query, auto query>
 auto & operator <<(std::basic_ostream<char> &lhs, Ocl::Info<Ocl::Id<Ocl::Platform<Ps...>>&, Query<query>> const&rhs){
   return lhs << (std::string const&)rhs;
 }
   template<typename...Ps>
    class Version<Id<Platform<Ps...>>&>{
    public:
        uint8_t major;
        uint8_t minor;
        uint8_t patch;

        Version(Id<Platform<Ps...>>& id){

        }
    };

    template<> class Platform<>{};
}