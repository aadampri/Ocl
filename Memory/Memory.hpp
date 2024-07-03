namespace Ocl{
    template <auto query>
    concept QueryMemoryFlags = [](auto &&_query){switch(_query){
        case CL_MEM_FLAGS:
            return true;
        default:
            return false;
    }}(query);

    template <auto query>
    concept QueryMemorySize = [](auto &&_query){switch(_query){
        case CL_MEM_SIZE:
            return true;
        default:
            return false;
    }}(query);

    template<typename...>class Memory;
    template<typename ...Args>
    Memory(Args &&...)->Memory<Args...>;

    template<typename _Memory, typename _Query>
    requires (QueryMemoryFlags<Query(std::remove_cvref_t<_Query>())> && std::convertible_to<std::remove_cvref_t<_Memory>, Handle<Memory<>>>)
    class Info<_Memory, _Query>{
       using Desc = cl_mem_flags;
       Desc desc;
    public:
        Info(_Memory &memory, _Query &&) : desc([](auto &&memory){
            Desc desc = {};
            constexpr auto query = Query(std::remove_cvref_t<_Query>());
            if(auto &&result = Result(clGetMemObjectInfo(memory, query, sizeof(desc), &desc, nullptr)); result) return desc;
            return Desc{};
        }(memory)){}

        operator Desc(){
            return desc;
        }
        operator Desc()const{
            return desc;
        }
    };

    template<typename _Memory, typename _Query>
    requires (QueryMemorySize<Query(std::remove_cvref_t<_Query>())> && std::convertible_to<std::remove_cvref_t<_Memory>, Handle<Memory<>>>)
    class Info<_Memory, _Query>{
       using Desc = size_t;
       Desc desc;
    public:
        Info(_Memory &memory, _Query &&) : desc([](auto &&memory){
            Desc desc = {};
            constexpr auto query = Query(std::remove_cvref_t<_Query>());
            if(auto &&result = Result(clGetMemObjectInfo(memory, query, sizeof(desc), &desc, nullptr)); result) return desc;
            return Desc{};
        }(memory)){}

        operator Desc(){
            return desc;
        }
        operator Desc()const{
            return desc;
        }
    };

    template<> class Memory<>{};

    template<>
    class Handle<Memory<>> : public Handle<cl_mem>{
    public:
        Handle(){}
        Handle(Memory<>&&){}
        Handle(Handle &&handle) : Handle<cl_mem>((Handle<cl_mem> &&)handle){}
        Handle(Handle const&handle) : Handle<cl_mem>((Handle<cl_mem> const &)handle){}
        Handle(auto &&handle) : Handle<cl_mem>(decltype(handle)(handle)){}

        Handle &operator=(Handle const&handle){
            return *new(this)Handle(handle);
        }

        template<auto...vs>
        auto operator()(Value<vs...> &&value)const{
            return Info(*this, decltype(value)(value));
        }
    };

    template<typename _Handle>
    requires (std::same_as<_Handle, std::remove_cvref_t<decltype(Handle(Memory()))>>)
    class Memory<_Handle> : public _Handle, public Result<cl_int> {
    public:
        Memory(_Handle &&handle) : _Handle(decltype(handle)(handle)){}
        Memory(auto &&handle, auto &&result) : _Handle(decltype(handle)(handle)), Result(decltype(result)(result)){}
        Memory(Memory const&memory) : _Handle(memory), Result(memory){clRetainMemObject(*this);}
        Memory(Memory &&memory) : _Handle(std::move(memory)), Result(std::move(memory)){}
        ~Memory(){ clReleaseMemObject(*this); }

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
            return (Memory const &)*this;
        }

        template<typename Rhs>
        requires std::same_as<std::remove_cvref_t<Rhs>, Memory>
        Memory & operator=(Rhs &&rhs){
            this->~Memory();
            return *new(this)Memory(decltype(rhs)(rhs)); 
        }
    };

    template<typename Context, typename _Size, template<auto> typename Query, auto query>
    requires (EqualT<std::remove_cvref_t<_Size>, SizeOf>() == true)
    class Memory<Context, _Size, Query<query>> : public Memory<Handle<Memory<>>>{
    public:
        Memory(auto &&context, _Size &&size, auto &&) :  Memory<Handle<Memory<>>>(clCreateBuffer(context, query, size, nullptr, (Result<cl_int> &)*this), *(Result<cl_int>*)this){
        }
    };

    template<typename Context, typename _Size>
    requires (EqualT<std::remove_cvref_t<_Size>, Size>() == true)
    class Memory<Context, _Size> : public Memory<Handle<Memory<>>>{
    public:
        Memory(auto &&context, _Size &&size) : Memory<Handle<Memory<>>>(clCreateBuffer(context, CL_MEM_READ_WRITE, size[0], nullptr, (Result<cl_int> &)*this), *(Result<cl_int>*)this){
        }
    };


    template<typename Buffer>
    class Memory<Buffer *> : public Memory<Handle<Memory<>>>{
    public:
        Memory(Buffer *&&buffer) : Memory<Handle<Memory<>>>((cl_mem &&)buffer, *(Result<cl_int>*)this){}
    };

    template<typename _Memory, typename _CommandQueue>
    //  std::convertible_to<Handle<Memory<>> &, std::remove_reference_t<_Memory>>
    requires (EqualT<std::remove_cvref_t<_CommandQueue>, CommandQueue>() == true && std::convertible_to<std::remove_cvref_t<_Memory>, Handle<Memory<>>>)
    class Enqueue<_CommandQueue, _Memory> : public Enqueue<Enqueue<_CommandQueue, _Memory> &>{
        _Memory memory;
        _CommandQueue commandQueue;

        auto MapFlags() const{
            auto &&flags = memory(Value<CL_MEM_FLAGS>());
            if(flags & CL_MEM_HOST_READ_ONLY) return CL_MAP_READ;
            if(flags & CL_MEM_HOST_WRITE_ONLY) return CL_MAP_WRITE;

            return CL_MAP_WRITE | CL_MAP_READ;
        }

    public:
        Enqueue(auto &&commandQueue, auto &&memory) : Enqueue<Enqueue<_CommandQueue, _Memory> &>(*this), memory(decltype(memory)(memory)), commandQueue(decltype(commandQueue)(commandQueue)){}

        template<typename ...Tail>
        requires (sizeof...(Tail) == 3)
        auto& Push(Tail &&...tail){
            cl_mem &mem = (Handle<Memory<>> &)memory;
            clEnqueueMigrateMemObjects(commandQueue, 1, &mem, 0, decltype(tail)(tail)...);
            return *this;
        }

        auto& Push(cl_mem_migration_flags flags, auto &&...tail){
            cl_mem &mem = (Handle<Memory<>> &)memory;
            clEnqueueMigrateMemObjects(commandQueue, 1, &mem, flags, decltype(tail)(tail)...);
            return *this;
        }
        
        auto& Push(auto *&buffer, auto &&...tail){
            (void *&)buffer = clEnqueueMapBuffer(commandQueue, memory, CL_FALSE, MapFlags(), 0, memory(Value<CL_MEM_SIZE>()), decltype(tail)(tail)..., nullptr);
            return *this;
        }
        
        template <typename S>
        auto& Push(auto *&buffer, Size<S> const &size, auto &&...tail){
            (void *&)buffer = clEnqueueMapBuffer(commandQueue, memory, CL_FALSE, MapFlags(), 0, size[0], decltype(tail)(tail)..., nullptr);
            return *this;
        }

        template <typename S0, typename S1>
        auto& Push(auto *&buffer, Size<S0, S1> const &size, auto &&...tail){
            (void *&)buffer = clEnqueueMapBuffer(commandQueue, memory, CL_FALSE, MapFlags(), size[0], size[1], decltype(tail)(tail)..., nullptr);
            return *this;
        }

        auto& Push(auto *&&buffer, auto &&...tail){
            cl_mem &mem = (Handle<Memory<>> &)memory;
            clEnqueueUnmapMemObject(commandQueue, mem, buffer, decltype(tail)(tail)...);
            return *this;
        }
    };


    template<typename ...Memories, typename _CommandQueue>
    requires ((EqualT<std::remove_cvref_t<_CommandQueue>, CommandQueue>() == true) && ... && (EqualT<std::remove_cvref_t<Memories>, Memory>() == true))
    class Enqueue<_CommandQueue, Memories...> : public Enqueue<Enqueue<_CommandQueue, Memories...> &>{
        std::array<Handle<Memory<>>, sizeof...(Memories)> memories;
        _CommandQueue commandQueue;
    public:
        Enqueue(auto &&commandQueue, auto &&...memories) : Enqueue<Enqueue<_CommandQueue, Memories...> &>(*this), memories{decltype(memories)(memories)...}, commandQueue(decltype(commandQueue)(commandQueue)){}

        template<typename ...Tail>
        requires (sizeof...(Tail) == 3)
        auto& Push(Tail &&...tail){
            cl_mem &mem = memories[0];
            clEnqueueMigrateMemObjects(commandQueue, memories.size(), &mem, 0, decltype(tail)(tail)...);
            return *this;
        }

        auto& Push(cl_mem_migration_flags flags, auto &&...tail){
           cl_mem &mem = memories[0];
            clEnqueueMigrateMemObjects(commandQueue, memories.size(), &mem, flags, decltype(tail)(tail)...);
            return *this;
        }
    };
}