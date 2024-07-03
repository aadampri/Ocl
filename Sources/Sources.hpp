#pragma once

#include<map>
namespace Ocl{
    template<typename...>class Sources;
    template<typename ...Args>
    Sources(Args &&...)->Sources<Args...>;

    template<typename Container>
    requires (IsTRank1<std::remove_cvref_t<Container>>::size >= 2 && std::convertible_to<typename IsTRank1<std::remove_cvref_t<Container>>::First, std::string_view> && std::convertible_to<typename IsTRank1<std::remove_cvref_t<Container>>::Second, std::string_view>)
    class Sources<Container &>{
        std::vector<char const*> sources;
        std::vector<size_t> sizes;
    public:
        Sources(Container &sources){
            Sources::sources.reserve(sources.size());
            Sources::sizes.reserve(sources.size());
            for(auto &&[_, source] : sources){
                std::string_view const &sv = source;
                Sources::sources.emplace_back(&sv[0]);
                Sources::sizes.emplace_back(sv.size());
            }
        }

        operator char const **(){
            return &sources[0];
        }

        operator size_t const *(){
            return &sizes[0];
        }

        operator cl_uint(){
            return sources.size();
        }
    };

    template<typename Container>
    requires (IsTRank1<std::remove_cvref_t<Container>>::size >= 2 && std::convertible_to<typename IsTRank1<std::remove_cvref_t<Container>>::Second, std::string_view>)
    class Sources<Container>{
        std::vector<char const*> sources;
        std::vector<size_t> sizes;
        Container container;
    public:
        Sources(Container sources) : container(decltype(sources)(sources)){
            Sources::sources.reserve(container.size());
            Sources::sizes.reserve(container.size());
            for(auto &&[_, source] : container){
                std::string_view const &sv = source;
                Sources::sources.emplace_back(&sv[0]);
                Sources::sizes.emplace_back(sv.size());
            }
        }

        operator char const **(){
            return &sources[0];
        }

        operator size_t const *(){
            return &sizes[0];
        }

        operator cl_uint(){
            return sources.size();
        }
    };

    template<typename Container>
    requires (IsTRank1<std::remove_cvref_t<Container>>::size == 1 && std::convertible_to<typename IsTRank1<std::remove_cvref_t<Container>>::First, std::string_view>) ||
             (IsTRank1<std::remove_cvref_t<Container>>::size > 1 && std::convertible_to<typename IsTRank1<std::remove_cvref_t<Container>>::First, std::string_view> && !std::convertible_to<typename IsTRank1<std::remove_cvref_t<Container>>::Second, std::string_view>) ||
             (IsTVRank1<std::remove_cvref_t<Container>>() == true && std::convertible_to<typename IsTVRank1<std::remove_cvref_t<Container>>::First, std::string_view>)
    class Sources<Container &>{
        std::vector<char const*> sources;
        std::vector<size_t> sizes;
    public:
        Sources(Container &sources){
            Sources::sources.reserve(sources.size());
            Sources::sizes.reserve(sources.size());
            for(auto &&source : sources){
                std::string_view const &sv = source;
                Sources::sources.emplace_back(&sv[0]);
                Sources::sizes.emplace_back(sv.size());
            }
        }

        operator char const **(){
            return &sources[0];
        }

        operator size_t const *(){
            return &sizes[0];
        }

        operator cl_uint(){
            return sources.size();
        }
    }; 

    template<typename Container>
    requires (IsTRank1<std::remove_cvref_t<Container>>::size == 1 && std::convertible_to<typename IsTRank1<std::remove_cvref_t<Container>>::First, std::string_view>) ||
             (IsTRank1<std::remove_cvref_t<Container>>::size > 1 && std::convertible_to<typename IsTRank1<std::remove_cvref_t<Container>>::First, std::string_view> && !std::convertible_to<typename IsTRank1<std::remove_cvref_t<Container>>::Second, std::string_view>) ||
             (IsTVRank1<std::remove_cvref_t<Container>>() == true && std::convertible_to<typename IsTVRank1<std::remove_cvref_t<Container>>::First, std::string_view>)
    class Sources<Container>{
        std::vector<char const*> sources;
        std::vector<size_t> sizes;
        Container container;
    public:
        Sources(Container sources) : container(decltype(sources)(sources)){
            Sources::sources.reserve(container.size());
            Sources::sizes.reserve(container.size());
            for(auto &&source : container){
                std::string_view const &sv = source;
                Sources::sources.emplace_back(&sv[0]);
                Sources::sizes.emplace_back(sv.size());
            }
        }

        operator char const **(){
            return &sources[0];
        }

        operator size_t const *(){
            return &sizes[0];
        }

        operator cl_uint(){
            return sources.size();
        }
    };   
}