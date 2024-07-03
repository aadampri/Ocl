#pragma once

namespace Ocl{
    template<typename...>class Context;
    template<typename ...Args>
    Context(Args &&...)->Context<Args...>;

    template <auto query>
    concept QueryContextUInt = [](auto &&_query){switch(_query){
        case CL_CONTEXT_NUM_DEVICES:
        case CL_CONTEXT_REFERENCE_COUNT:
            return true;
        default:
            return false;
    }}(query);

/*
    template <auto query>
    concept QueryContextBool = [](auto &&_query){switch(_query){
        case CL_CONTEXT_D3D10_PREFER_SHARED_RESOURCES_KHR:
        case CL_CONTEXT_D3D11_PREFER_SHARED_RESOURCES_KHR:
            return true;
        default:
            return false;
    }}(query);
*/
    template<typename ...Cs, typename _Query>
    requires QueryContextUInt<Query(std::remove_cvref_t<_Query>())>
    class Info<Context<Cs...>&, _Query>{
       cl_uint desc;
    public:
        Info(Context<Cs...>&context, _Query &&) : desc([](auto &&context){
            cl_uint desc = 0;
            constexpr auto query = Query(std::remove_cvref_t<_Query>());
            if(auto &&result = Result(clGetContextInfo(context, query, sizeof(desc), &desc, nullptr)); result) return desc;
            return cl_uint{};
        }(context)){}

        operator cl_uint(){
            return desc;
        }
        operator cl_uint()const{
            return desc;
        }
    };

    template<typename ...Ds, typename ...Cs>
    class Enum<Device<Ds...>, Context<Cs...>&> : public std::vector<Id<Device<Ds...>>>{
       Result<cl_int> result;
       public:
         Enum(Device<Ds...> &&, Context<Cs...>&context) : result(
            [](auto &&ids, auto &&context){
                cl_uint size = 0;
                if(auto &&result = Result(clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES, sizeof(size), &size, nullptr)); result){
                    ids->resize(size);
                    cl_device_id &id = (*ids)[0];
                    return Result(clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id)*size, &id, nullptr));                     
                }
                else return result;
            }(this, context)){}
    };

    template<>
    class Handle<Context<>> : public Handle<cl_context>{};

    template<>
    class Context<> : public Handle<Context<>>, public Result<cl_int> {
    public:
        Context(auto &&handle) : Handle(decltype(handle)(handle)){}
        Context(auto &&handle, auto &&result) : Handle(decltype(handle)(handle)), Result(decltype(result)(result)){}
        ~Context(){ clReleaseContext(*this); }

        constexpr operator bool()const{
            return (Result const &)*this;
        }

        constexpr operator bool(){
            return (Context const &)*this;
        }
    };

    template<typename Devices>
    class Context<Devices> : public Context<>{
    public:
        Context(auto &&devices) : Context<>(clCreateContext(nullptr, devices.size(), &(cl_device_id const &)devices[0], nullptr, nullptr, (Result<cl_int> &)*this), (Result<cl_int> &)*this){}
        
        template<typename ...Args>auto operator()(Args...);

        template<typename _Id>
        requires std::same_as<std::remove_cvref_t<_Id>, Id<Device<>>>
        auto operator()(_Id device){
            return CommandQueue(*this, decltype(device)(device));
        }

        template<typename _Sources>
        requires (EqualT<std::remove_cvref_t<_Sources>, Sources>() == true)
        auto operator()(_Sources sources){
            return Program(decltype(sources)(sources), *this);
        }

        template<typename Source>
        requires std::convertible_to<std::remove_cvref_t<Source>, std::string_view>
        auto operator()(Source source){
            return Program(decltype(source)(source), *this);
        }
    };
}