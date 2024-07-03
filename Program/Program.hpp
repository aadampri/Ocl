#pragma once
#include <string>
namespace Ocl{
    template <auto query>
    concept QueryProgramSize = [](auto &&_query){switch(_query){
        case CL_PROGRAM_NUM_KERNELS:
            return true;
        default:
            return false;
    }}(query);

    template <auto query>
    concept QueryProgramString = [](auto &&_query){switch(_query){
        case CL_PROGRAM_SOURCE:
        case CL_PROGRAM_KERNEL_NAMES:
        case CL_PROGRAM_BUILD_LOG:
            return true;
        default:
            return false;
    }}(query);

    template <auto query>
    concept QueryProgramUInt = [](auto &&_query){switch(_query){
        case CL_PROGRAM_REFERENCE_COUNT:
        case CL_PROGRAM_NUM_DEVICES:
            return true;
        default:
            return false;
    }}(query);

    template <auto query>
    concept EnumKernelName = [](auto &&_query){switch(_query){
        case CL_PROGRAM_KERNEL_NAMES:
            return true;
        default:
            return false;
    }}(query);

    template <auto query>
    concept EnumProgramDevice = [](auto &&_query){switch(_query){
        case CL_PROGRAM_DEVICES:
            return true;
        default:
            return false;
    }}(query);

    template<typename...>class Program;
    template<typename ...Args>
    Program(Args &&...)->Program<Args...>;

    template<typename _Program, typename _Query>
    requires QueryProgramSize<Query(std::remove_cvref_t<_Query>())>
    class Info<_Program, _Query>{
        size_t desc;
    public:
        Info(_Program &&program, _Query &&) : desc([](auto &&program){
            constexpr auto query = Query(std::remove_cvref_t<_Query>());
            size_t desc = 0;
            if(auto &&result = Result(clGetProgramInfo(program, query, sizeof(desc), &desc, nullptr)); result) return desc;
            return size_t{};
        }(program)){}

        operator size_t const&()const{
            return desc;
        }
    };

    template<typename _Program, typename _Query>
    requires QueryProgramUInt<Query(std::remove_cvref_t<_Query>())>
    class Info<_Program, _Query>{
        cl_uint desc;
    public:
        Info(_Program &&program, _Query &&) : desc([](auto &&program){
            constexpr auto query = Query(std::remove_cvref_t<_Query>());
            cl_uint desc = {};
            if(auto &&result = Result(clGetProgramInfo(program, query, sizeof(cl_uint), &desc, nullptr)); !result) return cl_uint{};
            return desc;
        }(program)){}

        operator cl_uint const&()const{
            return desc;
        }
    };

    template<typename _Program, typename _Query>
    requires QueryProgramString<Query(std::remove_cvref_t<_Query>())>
    class Info<_Program, _Query>{
        std::string desc;
    public:
        Info(_Program &&program, _Query &&) : desc([](auto &&program){
            constexpr auto query = Query(std::remove_cvref_t<_Query>());
            size_t length = 0;
            clGetProgramInfo(program, query, 0, nullptr, &length);
            auto desc = std::string(length - 1, ' ');
            if(auto &&result = Result(clGetProgramInfo(program, query, length, &desc[0], nullptr)); !result) return std::string{};
            return desc;
        }(program)){}

        operator std::string const&()const{
            return desc;
        }
    };

    template<typename _Program, typename ...Ds, typename _Query>
    requires QueryProgramString<Query(std::remove_cvref_t<_Query>())>
    class Info<_Program, Id<Device<Ds...>>&, _Query>{
        std::string desc;
    public:
        Info(_Program program, Id<Device<Ds...>> &device, _Query &&) : desc([](auto &&program, auto &&device){
            constexpr auto query = Query(std::remove_cvref_t<_Query>());
            size_t length = 0;
            clGetProgramBuildInfo(program, device, query, 0, nullptr, &length);
            auto desc = std::string(length - 1, ' ');
            if(auto &&result = Result(clGetProgramBuildInfo(program, device, query, length, &desc[0], nullptr)); !result) return std::string{};
            return desc;
        }(program, device)){}

        operator std::string const&()const{
            return desc;
        }
    };

    template<typename _Program, typename _Query>
    requires EnumKernelName<Query(std::remove_cvref_t<_Query>())>
    class Enum<_Program, _Query> : public Info<_Program, Value<CL_PROGRAM_KERNEL_NAMES>>, public std::vector<std::string_view>{
       Result<cl_int> result;
       public:
         Enum(_Program &&program, _Query &&) : Info<_Program, Value<CL_PROGRAM_KERNEL_NAMES>>(program, Value<CL_PROGRAM_KERNEL_NAMES>()), result(
            [](auto &&kernelNames, auto &&program){
                size_t size = 0;
                if(auto &&result = Result(clGetProgramInfo(program, CL_PROGRAM_NUM_KERNELS, sizeof(size), &size, nullptr)); result){
                    if(size == 0) return result;
                    kernelNames->reserve(size);
                    std::string const&names = *kernelNames;

                    size_t old = 0;
                    while(true){
                        auto &&pos = names.find_first_of(';', old);
                        if(pos == std::string::npos){
                            kernelNames->emplace_back(std::string_view{&names[old], &names[names.size()]});
                            break;
                        }
                        else
                            kernelNames->emplace_back(std::string_view{&names[old], &names[pos]});
                        old = pos + 1;
                    };
                   
                    return result;
                }
                else return result;
            }(this, program)){}
    };

    template<typename _Program, typename _Query>
    requires EnumProgramDevice<Query(std::remove_cvref_t<_Query>())>
    class Enum<_Program, _Query> : public Info<_Program, Value<CL_PROGRAM_NUM_DEVICES>>, public std::vector<Id<Device<>>>{
       Result<cl_int> result;
       public:
         Enum(_Program &&program, _Query &&) : Info<_Program, Value<CL_PROGRAM_NUM_DEVICES>>(program, Value<CL_PROGRAM_NUM_DEVICES>()), result(
            [](auto &&kernelIds, auto &&program){
                constexpr auto query = Query(std::remove_cvref_t<_Query>());
                cl_uint const&count = *kernelIds;
                kernelIds->resize(count);
                cl_device_id &id = (*kernelIds)[0];
                return Result(clGetProgramInfo(program, CL_PROGRAM_DEVICES, count * sizeof(cl_device_id), &id, nullptr));
            }(this, program)){}
    };

    template<> class Program<>{};

    template<>
    class Handle<Program<>> : public Handle<cl_program>{
    public:
        Handle(){}
        Handle(Program<>&&){}
        Handle(Handle &&handle) : Handle<cl_program>((Handle<cl_program> &&)handle){}
        Handle(Handle const&handle) : Handle<cl_program>((Handle<cl_program> const &)handle){}
        template<typename _Handle>
        requires (std::same_as<cl_program, std::remove_cvref_t<_Handle>>)
        Handle(_Handle handle) : Handle<cl_program>(decltype(handle)(handle)){}

        Handle &operator=(Handle const&handle){
            return *new(this)Handle(handle);
        }

        template<auto...vs>
        auto operator()(Value<vs...> &&value)const{
            return Info(*this, decltype(value)(value));
        }
    };

    template<typename _Handle>
    requires (std::same_as<_Handle, std::remove_cvref_t<decltype(Handle(Program()))>>)
    class Program<_Handle> : public _Handle, public Result<cl_int> {
    public:
        Program(_Handle &&handle) : _Handle(decltype(handle)(handle)){}
        Program(auto &&handle, auto &&result) : _Handle(decltype(handle)(handle)), Result(decltype(result)(result)){}
        Program(Program const&program) : _Handle(program), Result(program){clRetainProgram(*this);}
        Program(Program &&program) : _Handle(std::move(program)), Result(std::move(program)){}
        ~Program(){ clReleaseProgram(*this); }

        operator _Handle&(){
            return *this;
        }

        operator _Handle const&()const{
            return *this;
        }

        constexpr operator bool()const{
            return (Result const &)*this;
        }

        constexpr operator bool(){
            return (Program const &)*this;
        }

        template<typename Rhs>
        requires std::same_as<std::remove_cvref_t<Rhs>, Program>
        Program & operator=(Rhs &&rhs){
            this->~Program();
            return *new(this)Program(decltype(rhs)(rhs)); 
        }

        auto&& operator()() &&{
            Result<cl_int> &result = *this;
            result = Result(clBuildProgram(*this, 0, nullptr, nullptr, nullptr, nullptr));
            return std::move(*this);
        }

        template<typename Options>
        requires std::convertible_to<std::remove_cvref_t<Options>, std::string_view const &>
        auto&& operator()(Options &&options) &&{
            Result<cl_int> &result = *this;
            std::string_view const&_options = options;
            result = Result(clBuildProgram(*this, 0, nullptr, &_options[0], nullptr, nullptr));
            return std::move(*this);
        }

        template<typename T>
        auto operator()(void(*callback)(cl_program, T*)) &&{
            return [programHandle = std::move(*this), callback](auto &&...args)mutable{
                Result<cl_int> &result = programHandle;
                if constexpr(sizeof...(args) == 0)
                    result = Result(clBuildProgram(programHandle, 0, nullptr, "-cl-std=CL2.0 -cl-mad-enable", (void(*)(cl_program, void *))callback, nullptr));
                else if constexpr(sizeof...(args) == 1){
                    result = [&programHandle, &callback](auto &&arg0){
                        if constexpr(std::is_convertible<std::remove_cvref_t<decltype(arg0)>, std::string_view const &>()){
                            std::string_view const&_options = arg0;
                            return Result(clBuildProgram(programHandle, 0, nullptr, &_options[0], (void(*)(cl_program, void *))callback, nullptr));
                        }
                        else
                            return Result(clBuildProgram(programHandle, 0, nullptr, nullptr, (void(*)(cl_program, void *))callback, &arg0));
                    }(decltype(args)(args)...);
                }
                else if constexpr(sizeof...(args) == 2){
                    result = [&programHandle, &callback](auto &&arg0, auto &&arg1){
                        if constexpr(std::is_convertible<std::remove_cvref_t<decltype(arg0)>, std::string_view const &>()){
                            std::string_view const&_options = arg0;
                            return Result(clBuildProgram(programHandle, 0, nullptr, &_options[0], (void(*)(cl_program, void *))callback, &arg1));
                        }
                        else
                            return Result(clBuildProgram(programHandle, 0, nullptr, nullptr, (void(*)(cl_program, void *))callback, &arg1));
                    }(decltype(args)(args)...);
                }
    
                return std::move(programHandle);
            };
        }
    };

    template<typename Sources, typename Context>
    class Program<Sources, Context> : public Program<Handle<Program<>>>{
    public:
        Program(auto &&sources, auto &&context) : Program<Handle<Program<>>>([](auto &&sources, auto &&context, auto &&result){
            auto &&program = clCreateProgramWithSource(context, sources, sources, sources, result);
            return program;
        }(sources, context, (Result<cl_int> &)*this), *(Result<cl_int>*)this){}
    };

    template<typename Source, typename Context>
    requires std::convertible_to<std::remove_cvref_t<Source>, std::string_view>
    class Program<Source, Context> : public Program<Handle<Program<>>>{
    public:
        Program(auto &&source, auto &&context) : Program<Handle<Program<>>>([](auto &&source, auto &&context, auto &&result){
            char const *src = &source[0];
            size_t size = source.size();
            auto &&program = clCreateProgramWithSource(context, 1, &src, &size, result);
            return program;
        }(source, context, (Result<cl_int> &)*this), *(Result<cl_int>*)this){}
    };
}