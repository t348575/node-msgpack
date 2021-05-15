#include <napi.h>
#include <cstdint>
#include <tuple>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <any>
#include "msgpack.hpp"
#include <chrono>

namespace node_msgpack {

    // #define PROFILING 1
    #ifdef PROFILING
    #define PROFILE_SCOPE(name) VisualProfilerTimer timer##__LINE__(name)
    #define PROFILE_FUNCTION()  PROFILE_SCOPE(__FUNCSIG__)
    #else
    #define PROFILE_SCOPE(name)
    #define PROFILE_FUNCTION()
    #endif

    void PrintCurrentUsage() {
        std::cout << "Current usage: " << s_AllocationMetrics.CurrentUsage() << " Total Allocated: " << s_AllocationMetrics.TotalAllocated << " Total Freed: " << s_AllocationMetrics.TotalFreed << std::endl;
    }

    using namespace std;
    using namespace Napi;

    bool is_int(Env& env, Value& num) {
        return env.Global().Get("Number").ToObject().Get("isInteger").As<Function>().Call({ num }).ToBoolean().Value();
    }

    void pack(Env& env, vector<Value>& src, msgpack_byte::container& dest, bool initial = true);

    void unpack(Env& env, vector<Value>& dest, msgpack_byte::container& src, uint64_t pos);

    void pack(Env& env, Value& src, msgpack_byte::container& dest, bool initial = false) {
        PROFILE_FUNCTION();
        switch (src.Type()) {
            case napi_boolean: {
                msgpack::pack(src.ToBoolean().Value(), dest);
                break;
            }
            case napi_number: {
                if (is_int(env, src)) {
                    int64_t num = src.ToNumber().Int64Value();
                    if (num >= 0) {
                        msgpack::pack_uint(num, dest);
                    }
                    else {
                        msgpack::pack_int(num, dest);
                    }
                }
                else {
                    msgpack::pack(src.ToNumber().DoubleValue(), dest);
                }
                break;
            }
            case napi_string: {
                PROFILE_SCOPE("str_conversion");
                msgpack::pack(src.ToString().Utf8Value(), dest);
                break;
            }
            case napi_null:
            case napi_undefined: {
                msgpack::pack(nullptr, dest);
                break;
            }
            case napi_object: {
                if (src.IsArray()) {
                    PROFILE_SCOPE("array");
                    Array input_js_arr = src.As<Array>();
                    const uint32_t n = input_js_arr.Length();
                    vector<Value> input_vec;
                    input_vec.resize(n);
                    for (uint32_t i = 0; i < n; i++) {
                        input_vec[i] = (input_js_arr.Get(i));
                    }
                    pack(env, input_vec, dest);
                }
                else {
                    PROFILE_SCOPE("object");
                    Object input_js_obj = src.As<Object>();
                    Array obj_props = input_js_obj.GetPropertyNames();
                    const uint32_t n = obj_props.Length();
                    if (initial) {
                        dest.check_resize(size_t(msgpack::LengthOf(src) * compression_percent));
                    }
                    if (n <= 15) {
                        dest.push_back(fixmap_t(n));
                    }
                    else if (n <= umax16) {
                        dest.push_back(uint8_t(map16));
                        dest.push_back(uint16_t(n));
                    }
                    else if (n <= umax32) {
                        dest.push_back(uint8_t(map32));
                        dest.push_back(uint32_t(n));
                    }
                    for (uint32_t i = 0; i < n; i++) {
                        PROFILE_SCOPE("object_iter");
                        Value key = obj_props.Get(i);
                        Value val = input_js_obj.Get(key);
                        pack(env, key, dest, false);
                        pack(env, val, dest, false);
                    }
                    if (initial) {
                        dest.shrink_to_fit();
                    }
                }
                break;
            }
        }
    }

    void pack(Env& env, vector<Value>& src, msgpack_byte::container& dest, bool initial) {
        PROFILE_FUNCTION();
        size_t n = src.size();
        if (initial) {
            dest.check_resize(size_t(msgpack::LengthOf(src) * compression_percent));
        }
        if (n <= 15) {
            dest.push_back(fixarray_t(n));
        }
        else if (n <= umax16) {
            dest.push_back(uint8_t(arr16));
            dest.push_back(uint16_t(n));
        }
        else if (n <= umax32) {
            dest.push_back(uint8_t(arr32));
            dest.push_back(uint32_t(n));
        }
        for (uint32_t i = 0; i < n; i++) {
            pack(env, src[i], dest, false);
        }
        if (initial) {
            dest.shrink_to_fit();
        }
    }

    void unpack(Env& env, Value& dest, msgpack_byte::container& src, uint64_t pos) {

    }

    String stringify(const CallbackInfo& info) {
        Env env = info.Env();
        msgpack_byte::container dest;
        if (info.Length() == 0) {
            Error::New(env, "Expected an argument").ThrowAsJavaScriptException();
            return String::New(env, "");
        }
        VisualProfiler::Instance().beginSession("test1");
        Value input = info[0];
        pack(env, input, dest, true);
        VisualProfiler::Instance().endSession();
        // msgpack_byte::to_stringstream(dest, true).str()
        return String::New(env, msgpack_byte::to_string(dest));
    }

    Object parse(const CallbackInfo& info) {
        Env env = info.Env();
        msgpack_byte::container dest;
        if (info.Length() == 0) {
            Error::New(env, "Expected an argument").ThrowAsJavaScriptException();
            return Object::New(env);
        }
        Value input = info[0];
        pack(env, input, dest, true);
    }
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "stringify"), Napi::Function::New(env, node_msgpack::stringify));
    exports.Set(Napi::String::New(env, "parse"), Napi::Function::New(env, node_msgpack::parse));
    return exports;
}

NODE_API_MODULE(addon, Init)