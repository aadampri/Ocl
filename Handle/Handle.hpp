#pragma once

namespace Ocl{
    template<typename...>class Handle;
    template<typename ...Args>
    Handle(Args &&...)->Handle<Args...>;

    template<typename Type>
    class Handle<Type>{
        Type handle;
    public:
        Handle() : handle{}{}
        Handle(auto &&handle) : handle(decltype(handle)(handle)){}
        Handle(Handle &&handle) : handle(std::move(handle.handle)){handle.handle = {};}
        Handle(Handle const&handle) : handle(handle.handle){}

        operator Type&(){
            return handle;
        }
/*
        operator Type*(){
            return &handle;
        }*/

        operator Type const&()const{
            return handle;
        }

        Handle &operator=(Handle const&handle){
            return *new(this)Handle(handle);
        }
    };
}