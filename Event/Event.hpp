#pragma once
namespace Ocl{
    template<typename...>class Event;
    template<typename ...Args>
    Event(Args &&...)->Event<Args...>;

    template<> class Event<>{};

    template<>
    class Handle<Event<>> : public Handle<cl_event>{
    public:
        Handle(){}
        Handle(Event<>&&){}
        Handle(Handle &&handle) : Handle<cl_event>((Handle<cl_event> &&)handle){}
        Handle(Handle const&handle) : Handle<cl_event>((Handle<cl_event> const &)handle){}
    };

    template<typename _Handle>
    requires (std::same_as<_Handle, std::remove_cvref_t<decltype(Handle(Event()))>>)
    class Event<_Handle> : public _Handle, public Result<cl_int> {
    public:
        Event(_Handle &&handle) : _Handle(decltype(handle)(handle)){}
        Event(auto &&handle, auto &&result) : _Handle(decltype(handle)(handle)), Result(decltype(result)(result)){}
        Event(Event const&event) : _Handle(event), Result(event){ clRetainEvent(*this);}
        Event(Event &&event) : _Handle(std::move(event)), Result(std::move(event)){}
        ~Event(){ clReleaseEvent(*this); }

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
            return (Event const &)*this;
        }
/*
        Event const& operator()(cl_int type, void (*callback)(cl_event, cl_int, void *)){
            return (*this)(type, callback, nullptr);
        }*/
        Event&& operator()(cl_int type, void (*callback)(cl_event, cl_int, void *))&&{
            return std::move(*this)(type, callback, nullptr);
        }

        Event&& operator()(cl_int type, void (*callback)(cl_event, cl_int, void *), void *data)&&{
            (Result<cl_int> &)*this = Result(clSetEventCallback(*this, type, callback, data));
            return std::move(*this);
        }

        template<typename Rhs>
        requires std::same_as<std::remove_cvref_t<Rhs>, Event>
        Event & operator=(Rhs &&rhs){
            this->~Event();
            return *new(this)Event(decltype(rhs)(rhs)); 
        }
    };

    template<typename Context>
    class Event<Context> : public decltype(Event(Handle(Event()))){
    public:
        Event(auto &&context) : decltype(Event(Handle(Event())))(clCreateUserEvent(context, (Result<cl_int> &)*this), (Result<cl_int> &)*this){}
    };


    template<typename ...>class Wait;
    template<typename...Args>
    Wait(Args &&...) -> Wait<Args...>;

    template<typename...Events>
    class Wait : public Result<cl_int>{
    public:
        Wait(Events &&...events) : Result<cl_int>([](auto &&...events){ 
            if constexpr(constexpr auto count = sizeof...(events); count == 1){
                return clWaitForEvents(1, &(cl_event &)events...);
            }else{
                auto handles = std::array<Handle<Event<>>, count>{events...};
                return clWaitForEvents(count, &(cl_event &)handles[0]);
            }
        }(decltype(events)(events)...)) {}
    };
}