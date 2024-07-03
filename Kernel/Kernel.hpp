#pragma once
#include <initializer_list>
		
namespace Ocl{
    template <auto query>
    concept QueryKernelULong = [](auto &&_query){switch(_query){
        case CL_KERNEL_WORK_GROUP_SIZE: //size_t
        case CL_KERNEL_LOCAL_MEM_SIZE:
        case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE: //size_t
        case CL_KERNEL_PRIVATE_MEM_SIZE:
            return true;
        default:
            return false;
    }}(query);

    template <auto query>
    concept QueryKernelSize3 = [](auto &&_query){switch(_query){
        case CL_KERNEL_GLOBAL_WORK_SIZE:
        case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
            return true;
        default:
            return false;
    }}(query);


    template<typename...>class Kernel;
    template<typename ...Args>
    Kernel(Args &&...)->Kernel<Args...>;

    template<typename _Kernel, typename...Ds, typename _Query>
    requires (EqualT<std::remove_cvref_t<_Kernel>, Kernel>() == true && QueryKernelULong<Query(std::remove_cvref_t<_Query>())>)
    class Info<_Kernel &, Id<Device<Ds...>>&, _Query>{
       cl_ulong desc;
    public:
        Info(_Kernel &kernel, Id<Device<Ds...>> &id, _Query &&) : desc([](auto &&kernel, auto &&id){
            cl_ulong desc = 0;
            constexpr auto query = Query(std::remove_cvref_t<_Query>());
            if(auto &&result = Result(clGetKernelWorkGroupInfo(kernel, id, query, sizeof(desc), &desc, nullptr)); result) return desc;
            return cl_ulong{};
        }(kernel, id)){}

        operator cl_ulong(){
            return desc;
        }
        operator cl_ulong()const{
            return desc;
        }
    };

    template<typename ...Ks, typename...Ds, typename _Query>
    requires (QueryKernelSize3<Query(std::remove_cvref_t<_Query>())>)
    class Info<Kernel<Ks...>&, Id<Device<Ds...>>&, _Query>{
       using Desc = std::array<size_t, 3>;
       Desc desc;
    public:
        Info(Kernel<Ks...>&kernel, Id<Device<Ds...>> &id, _Query &&) : desc([](auto &&kernel, auto &&id){
            Desc desc = {};
            constexpr auto query = Query(std::remove_cvref_t<_Query>());
            if(auto &&result = Result(clGetKernelWorkGroupInfo(kernel, id, query, sizeof(desc), &desc[0], nullptr)); result) return desc;
            return Desc{};
        }(kernel, id)){}

        operator Desc &(){
            return desc;
        }
        operator Desc const &()const{
            return desc;
        }
    };

    template<typename ...Ms>
    constexpr auto GetKernelArgSize(Handle<Memory<Ms...>> const&){
        return sizeof(cl_mem);
    }

    template<typename ...Ms>
    constexpr auto GetKernelArgSize(Memory<Ms...> const&){
        return sizeof(cl_mem);
    }

    template<typename Arg>
    constexpr auto GetKernelArgSize(Arg const&){
        return sizeof(Arg);
    }

    template<typename _SizeOf>
    requires (EqualT<std::remove_cvref_t<_SizeOf>, SizeOf>() == true)
    constexpr size_t GetKernelArg(_SizeOf &&sizeOf){
        return sizeOf;
    }
    
    template<typename Arg>
    constexpr auto GetKernelArg(Arg &&arg){
        return &arg;
    }

    template<typename _SizeOf>
    requires (EqualT<std::remove_cvref_t<_SizeOf>, SizeOf>() == true)
    constexpr void* GetKernelArg(_SizeOf &&){
        return nullptr;
    }

    template<> class Kernel<>{};

    template<>
    class Handle<Kernel<>> : public Handle<cl_kernel>{
    public:
        Handle(){}
        Handle(Kernel<>&&){}
        Handle(Handle &&handle) : Handle<cl_kernel>((Handle<cl_kernel> &&)handle){}
        Handle(Handle const&handle) : Handle<cl_kernel>((Handle<cl_kernel> const &)handle){}
        template<typename _Handle>
        requires (std::same_as<cl_kernel, std::remove_cvref_t<_Handle>>)
        Handle(_Handle handle) : Handle<cl_kernel>(decltype(handle)(handle)){}

        Handle &operator=(Handle const&handle){
            return *new(this)Handle(handle);
        }

        template<auto...vs>
        auto operator()(Value<vs...> &&value)const{
            return Info(*this, decltype(value)(value));
        }
    };

    template<typename _Handle>
    requires (std::same_as<_Handle, std::remove_cvref_t<decltype(Handle(Kernel()))>>)
    class Kernel<_Handle> : public _Handle, public Result<cl_int> {
        template<size_t ...Is>
        constexpr Kernel& apply(std::index_sequence<Is...> const &, auto &&...args){
            (clSetKernelArg(*this, Is, GetKernelArgSize(args), GetKernelArg(decltype(args)(args))), ...);
            return *this;
        }
    public:
        Kernel(){}
        Kernel(_Handle &&handle) : _Handle(decltype(handle)(handle)){}
        Kernel(auto &&handle, auto &&result) : _Handle(decltype(handle)(handle)), Result(decltype(result)(result)){}
        Kernel(Kernel const&kernel) : _Handle(kernel), Result(kernel){clRetainKernel(*this);}
        Kernel(Kernel &&kernel) : _Handle(std::move(kernel)), Result(std::move(kernel)){}
        ~Kernel(){ clReleaseKernel(*this); }

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
            return (Kernel const &)*this;
        }

       Kernel & operator=(Kernel const&rhs){
            this->~Kernel();
            return *new(this)Kernel(decltype(rhs)(rhs)); 
        }
       Kernel & operator=(Kernel &&rhs){
            this->~Kernel();
            return *new(this)Kernel(decltype(rhs)(rhs)); 
        }
        template<typename T>
        operator T() = delete;

        template<typename T>
        operator T()const = delete;

        template<typename...Indices, typename ...Args>
        constexpr Kernel& operator()(std::tuple<Indices, Args> &&...indexedArgs){
            (clSetKernelArg(*this, std::get<0>(indexedArgs), GetKernelArgSize(std::get<1>(indexedArgs)), &std::get<1>(indexedArgs)), ...);
            return *this;
        }
        constexpr Kernel& operator()(auto &&...args){
            return apply(std::make_index_sequence<sizeof...(args)>{}, decltype(args)(args)...);
        }
    };

    template<typename Program, typename Name>
    class Kernel<Program, Name> : public Kernel<Handle<Kernel<>>>{
    public:
        Kernel(auto &&program, auto &&name) : Kernel<Handle<Kernel<>>>(clCreateKernel(program, &name[0], (Result<cl_int> &)*this), (Result<cl_int> &)*this){}
    };

    template<typename _Kernel, typename _CommandQueue>
    requires (EqualT<std::remove_cvref_t<_CommandQueue>, CommandQueue>() == true && EqualT<std::remove_cvref_t<_Kernel>, Kernel>() == true)
    class Enqueue<_CommandQueue, _Kernel> : public Enqueue<Enqueue<_CommandQueue, _Kernel> &>{
        _Kernel kernel;
        _CommandQueue commandQueue;
    public:
        Enqueue(auto &&commandQueue, auto &&kernel) : Enqueue<Enqueue<_CommandQueue, _Kernel> &>(*this), kernel(decltype(kernel)(kernel)), commandQueue(decltype(commandQueue)(commandQueue)){}

        template <typename ... Gs>
        auto& Push(Size<Gs...> const&globalSize, auto &&...tail){
            clEnqueueNDRangeKernel(commandQueue, kernel, sizeof...(Gs), nullptr, &globalSize[0], nullptr, decltype(tail)(tail)...);
            return *this;
        }

        template <typename ... Gs, typename ...Ls>
        requires (sizeof...(Gs) == sizeof...(Ls))
        auto& Push(Size<Gs...> const&globalSize, Size<Ls...> &&localSize, auto &&...tail){
            clEnqueueNDRangeKernel(commandQueue, kernel, sizeof...(Gs), nullptr, &globalSize[0], &localSize[0], decltype(tail)(tail)...);
            return *this;
        }
        template <typename ... Os,typename ...Ls>
        requires (sizeof...(Os) == sizeof...(Ls))
        auto& Push(std::array<Size<Os...>, 2> &&globalSize, Size<Ls...> &&localSize, auto &&...tail){
            clEnqueueNDRangeKernel(commandQueue, kernel, sizeof...(Os), &globalSize[0][0], &globalSize[1][0], &localSize[0], decltype(tail)(tail)...);
            return *this;
        }
    };
}