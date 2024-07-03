#pragma once

namespace Ocl{

    template <auto query>
    concept QueryDeviceString = [](auto &&_query){switch(_query){
        case CL_DEVICE_VERSION:
        case CL_DEVICE_VENDOR:
        case CL_DEVICE_NAME:
        case CL_DRIVER_VERSION:
        case CL_DEVICE_PROFILE:
        case CL_DEVICE_OPENCL_C_VERSION:
            return true;
        default:
            return false;
    }}(query);

    template <auto query>
    concept QueryDeviceUInt = [](auto &&_query){switch(_query){
        case CL_DEVICE_VENDOR_ID:
        case CL_DEVICE_PARTITION_MAX_SUB_DEVICES:
        case CL_DEVICE_MAX_COMPUTE_UNITS:
        case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
        case CL_DEVICE_MAX_CLOCK_FREQUENCY:
        case CL_DEVICE_ADDRESS_BITS:
        case CL_DEVICE_MAX_ON_DEVICE_QUEUES:
        case CL_DEVICE_MAX_NUM_SUB_GROUPS:
        case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
            return true;
        default:
            return false;
    }}(query);

    template <auto query>
    concept QueryDeviceULong = [](auto &&_query){switch(_query){
        case CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE: //size_t
        case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
        case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
        case CL_DEVICE_GLOBAL_MEM_SIZE:
        case CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE: //size_t
        case CL_DEVICE_MAX_WORK_GROUP_SIZE: // size_t
        case CL_DEVICE_LOCAL_MEM_SIZE:
            return true;
        default:
            return false;
    }}(query);

    template <auto query>
    concept QueryDeviceBitfield = [](auto &&_query){switch(_query){
        case CL_DEVICE_TYPE:
        case CL_DEVICE_SINGLE_FP_CONFIG:
        case CL_DEVICE_DOUBLE_FP_CONFIG:
        case CL_DEVICE_EXECUTION_CAPABILITIES:
        case CL_DEVICE_SVM_CAPABILITIES:
        case CL_DEVICE_PARTITION_AFFINITY_DOMAIN:
        case CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES:
        case CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES:
            return true;
        default:
            return false;
    }}(query);

    template <auto query>
    concept EnumDeviceSize = [](auto &&_query){switch(_query){
        case CL_DEVICE_MAX_WORK_ITEM_SIZES:
            return true;
        default:
            return false;
    }}(query);

    template <auto query>
    concept EnumDeviceNameVersion = [](auto &&_query){switch(_query){
        case CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION:
        case CL_DEVICE_ILS_WITH_VERSION:
        case CL_DEVICE_EXTENSIONS_WITH_VERSION:
        case CL_DEVICE_OPENCL_C_ALL_VERSIONS:
        case CL_DEVICE_OPENCL_C_FEATURES:
            return true;
        default:
            return false;
    }}(query);

    template<typename...>class Device;
    template<typename ...Args>
    Device(Args &&...)->Device<Args...>;

    template<typename...Ds>
    class Id<Device<Ds...>> : public Handle<cl_device_id>{
    public:
        Id() {}
        Id(cl_device_id id) : Handle(id){}
    };

    template<typename ...Ds, typename ...Ps>
    class Enum<Device<Ds...>, Id<Platform<Ps...>>&> : public std::vector<Id<Device<Ds...>>>{
       Result<cl_int> result;
       public:
         Enum(Device<Ds...> &&, Id<Platform<Ps...>>&platform) : result(
            [](auto &&ids, auto &&platform){
                cl_uint size = 0;
                if(auto &&result = Result(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &size)); result){
                    ids->resize(size);
                    cl_device_id &id = (*ids)[0];
                    return Result(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, ids->size(), &id, nullptr));
                }
                else return result;
            }(this, platform)){}
    };

    template<typename ...Ds, typename _Query>
    requires EnumDeviceSize<Query(std::remove_cvref_t<_Query>())>
    class Enum<Id<Device<Ds...>> &, _Query> : public std::vector<size_t>{
       Result<cl_int> result;
       public:
         Enum(Id<Device<Ds...>> &device, _Query &&) : result(
            [](auto &&sizes, auto &&device){
                cl_uint size = 0;
                constexpr auto query = Query(std::remove_cvref_t<_Query>());
                static_assert(query == CL_DEVICE_MAX_WORK_ITEM_SIZES, "Enum Size[] supported only for: CL_DEVICE_MAX_WORK_ITEM_SIZES!");
                if(auto &&result = Result(clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(size), &size, nullptr)); result){
                    sizes->resize(size);
                    auto &&size0 = (*sizes)[0];
                    return Result(clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizes->size()*sizeof(size_t), &size0, nullptr));
                }
                else {std::cout << "Error: " << Info(result) << "\n";
                    return result;}
            }(this, device)){}
    };

    template<typename ...Ds, typename _Query>
    requires EnumDeviceNameVersion<Query(std::remove_cvref_t<_Query>())>
    class Enum<Id<Device<Ds...>> &, _Query> : public std::vector<cl_name_version>{
       Result<cl_int> result;
       public:
         Enum(Id<Device<Ds...>> &device, _Query &&) : result(
            [](auto &&sizes, auto &&device){
                cl_uint size = 1;
                constexpr auto query = Query(std::remove_cvref_t<_Query>());
                //if(auto &&result = Result(clGetDeviceInfo(device, query, sizeof(size), &size, nullptr)); result)
                {
                    sizes->resize(size);
                    auto &&size0 = (*sizes)[0];
                    return Result(clGetDeviceInfo(device, query, sizes->size()*sizeof(cl_name_version), &size0, nullptr));
                }
               // else {std::cout << "Error: " << Info(result) << "\n";
               //     return result;}
            }(this, device)){}
    };

    template<typename...Ds, typename _Query>
    requires QueryDeviceString<Query(std::remove_cvref_t<_Query>())>
    class Info<Id<Device<Ds...>>&, _Query>{
        std::string desc;
    public:
        Info(Id<Device<Ds...>> &id, _Query &&) : desc([](auto &&id){
            size_t size = 0;
            constexpr auto query = Query(std::remove_cvref_t<_Query>());
            if(auto &&result = Result(clGetDeviceInfo(id, query, 0, nullptr, &size)); !result) std::string("");
            auto desc = std::string(size - 1, 0);
            if(auto &&result = Result(clGetDeviceInfo(id, query, size, (void *)desc.c_str(), nullptr)); result) return desc;
            return std::string("");
        }(id)){}

        operator std::string const&()const{
            return desc;
        }
    };

    template<typename...Ds, typename _Query>
    requires QueryDeviceUInt<Query(std::remove_cvref_t<_Query>())>
    class Info<Id<Device<Ds...>>&, _Query>{
       cl_uint desc;
    public:
        Info(Id<Device<Ds...>> &id, _Query &&) : desc([](auto &&id){
            cl_uint desc = 0;
            constexpr auto query = Query(std::remove_cvref_t<_Query>());
            if(auto &&result = Result(clGetDeviceInfo(id, query, sizeof(desc), &desc, nullptr)); result) return desc;
            return 0u;
        }(id)){}

        operator cl_uint(){
            return desc;
        }
        operator cl_uint()const{
            return desc;
        }
    };

    template<typename...Ds, typename _Query>
    requires QueryDeviceULong<Query(std::remove_cvref_t<_Query>())>
    class Info<Id<Device<Ds...>>&, _Query>{
       cl_ulong desc;
    public:
        Info(Id<Device<Ds...>> &id, _Query &&) : desc([](auto &&id){
            cl_ulong desc = 0;
            constexpr auto query = Query(std::remove_cvref_t<_Query>());
            if(auto &&result = Result(clGetDeviceInfo(id, query, sizeof(desc), &desc, nullptr)); result) return desc;
            return cl_ulong{};
        }(id)){}

        operator cl_ulong(){
            return desc;
        }
        operator cl_ulong()const{
            return desc;
        }
    };

    template<typename...Ds, typename _Query>
    requires QueryDeviceBitfield<Query(std::remove_cvref_t<_Query>())>
    class Info<Id<Device<Ds...>>&, _Query>{
        cl_bitfield desc;
    public:
        Info(Id<Device<Ds...>> &id, _Query &&) : desc([](auto &&id){
            cl_bitfield desc = 0;
            constexpr auto query = Query(std::remove_cvref_t<_Query>());
            if(auto &&result = Result(clGetDeviceInfo(id, query, sizeof(desc), &desc, nullptr)); result) return desc;
            return cl_bitfield{};
        }(id)){}

        operator cl_bitfield(){
            return desc;
        }
        operator cl_bitfield()const{
            return desc;
        }
    };

    template<typename ...Ds>
    class Version<Device<Ds...>&>{
    public:
        uint8_t major;
        uint8_t minor;
        uint8_t patch;

        Version(Device<Ds...> &device){
        }
    };

    template<>class Device<>{};
}