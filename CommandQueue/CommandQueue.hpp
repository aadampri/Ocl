#pragma once
#include <array>
namespace Ocl{
    template <auto query>
    concept QueryCommandQueueDeviceId = [](auto &&_query){switch(_query){
        case CL_QUEUE_DEVICE:
            return true;
        default:
            return false;
    }}(query);

    template<typename...>class Enqueue;
    template<typename ...Args>
    Enqueue(Args &&...)->Enqueue<Args...>;
    
    template<typename...Es>
    class Enqueue<Enqueue<Es...>&>{
        auto _Enqueue(auto &&...args){
            auto eventHandle = Handle(Event());
            cl_event &event = eventHandle;
            
            ((Enqueue<Es...>&)*this).Push(decltype(args)(args)..., 0, nullptr, &event);
            return Event(std::move(eventHandle));
        }

        template <auto waitEventCount>
        auto _Enqueue(std::array<Handle<Event<>>, waitEventCount> &&waitEvents, auto &&...args){
            auto eventHandle = Handle(Event());
            cl_event &event = eventHandle;
            cl_event &waitEvent = waitEvents[0];

            ((Enqueue<Es...>&)*this).Push(decltype(args)(args)..., waitEvents.size(), &waitEvent, &event);
            return Event(std::move(eventHandle));
        }
    
        public:
            Enqueue(auto &&enqueue){}

            auto operator()()->decltype(auto) {
                return this->_Enqueue();
            }
            
            template<typename ...Args>
            requires (!std::convertible_to<std::remove_cvref_t<Args>, Handle<Event<>>> && ...)
            auto operator()(Args &&...args)->decltype(auto) {
                return this->_Enqueue(decltype(args)(args)...);
            }

            template<typename ...Events>
            requires (std::convertible_to<std::remove_cvref_t<Events>, Handle<Event<>>> && ...)
            auto operator()(Events &&...events)->decltype(auto) {
                return [this, &...events = decltype(events)(events)](auto &&...args){
                    return this->_Enqueue(std::array{Handle<Event<>>(events)...}, decltype(args)(args)...);
                };
            }
    };
    
    template<typename...>class CommandQueue;
    template<typename ...Args>
    CommandQueue(Args &&...)->CommandQueue<Args...>;

    template<typename ...Cs, typename _Query>
    requires QueryCommandQueueDeviceId<Query(std::remove_cvref_t<_Query>())>
    class Info<CommandQueue<Cs...>&, _Query>{
       cl_device_id desc;
    public:
        Info(CommandQueue<Cs...>&commandQueue, _Query &&) : desc([](auto &&commandQueue){
            cl_device_id desc = 0;
            constexpr auto query = Query(std::remove_cvref_t<_Query>());
            if(auto &&result = Result(clGetCommandQueueInfo(commandQueue, query, sizeof(desc), &desc, nullptr)); result) return desc;
            return cl_device_id{};
        }(commandQueue)){}

        operator cl_device_id(){
            return desc;
        }
        operator cl_device_id()const{
            return desc;
        }
    };

    template<> class CommandQueue<>{};

    template<>
    class Handle<CommandQueue<>> : public Handle<cl_command_queue>{
    public:
        Handle(){}
        Handle(CommandQueue<>&&){}
        Handle(Handle &&handle) : Handle<cl_command_queue>((Handle<cl_command_queue> &&)handle){}
        Handle(Handle const&handle) : Handle<cl_command_queue>((Handle<cl_command_queue> const &)handle){}
        Handle(auto &&handle) : Handle<cl_command_queue>(decltype(handle)(handle)){}

        Handle &operator=(Handle const&handle){
            return *new(this)Handle(handle);
        }

        template<auto...vs>
        auto operator()(Value<vs...> &&value)const{
            return Info(*this, decltype(value)(value));
        }
    };

    template<typename _Handle>
    requires (std::same_as<_Handle, std::remove_cvref_t<decltype(Handle(CommandQueue()))>>)
    class CommandQueue<_Handle> : public _Handle, public Result<cl_int> {
    public:
        CommandQueue(_Handle &&handle) : _Handle(decltype(handle)(handle)){}
        CommandQueue(auto &&handle, auto &&result) : _Handle(decltype(handle)(handle)), Result(decltype(result)(result)){}
        CommandQueue(CommandQueue const&memory) : _Handle(memory), Result(memory){clRetainCommandQueue(*this);}
        CommandQueue(CommandQueue &&memory) : _Handle(std::move(memory)), Result(std::move(memory)){}
        ~CommandQueue(){ clReleaseCommandQueue(*this); }

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
            return (CommandQueue const &)*this;
        }
   };

    template<typename Context, typename Device>
    class CommandQueue<Context, Device> : public CommandQueue<Handle<CommandQueue<>>>{
    public:
        CommandQueue(auto &&context, auto &&device) : CommandQueue<Handle<CommandQueue<>>>(clCreateCommandQueueWithProperties(context, device, nullptr, (Result<cl_int> &)*this), (Result<cl_int> &)*this){}

        auto &operator()(){
            clFlush(*this);
            return *this;
        }

        template<typename...Args>
        requires (sizeof...(Args) > 0)
        auto operator()(Args &&...objects){
            return Enqueue(*this, decltype(objects)(objects)...);
        }
    };


    template<typename _CommandQueue>
    requires (EqualT<std::remove_cvref_t<_CommandQueue>, CommandQueue>() == true || std::same_as<std::remove_cvref_t<_CommandQueue>, Handle<CommandQueue<>>>)
    class Wait<_CommandQueue> : Result<cl_int>{
    public:
        Wait(_CommandQueue &&commandQueue) : Result<cl_int>([](auto &&commandQueue){ 
                return clFinish(commandQueue);
        }(decltype(commandQueue)(commandQueue))) {}
    };
}