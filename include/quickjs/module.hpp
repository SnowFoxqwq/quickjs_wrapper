#pragma once

#include "type_converter.hpp"
#include "type_traits.hpp"
#include "exception.hpp"

#include <quickjs.h>

#include <string>
#include <vector>

namespace js
{
    class Module;

    namespace detail
    {
        struct ModuleExportEntry
        {
            std::string name;
            JSValue value;
        };
    }

    template <typename T>
    class ClassBuilder;

    class Module
    {
    public:
        Module(const std::string& name, JSContext* ctx);
        ~Module();

        Module(const Module&) = delete;
        Module& operator=(const Module&) = delete;

        Module(Module&& other) noexcept;
        Module& operator=(Module&& other) noexcept;

        // add function to module
        template <auto Func>
        Module& function(const std::string& name)
        {
            using Wrapper = FreeFunctionWrapper<Func>;
            using Traits = detail::FunctionTraits<decltype(Func)>;

            JSValue func = JS_NewCFunction(_ctx, Wrapper::call, name.c_str(), Traits::arity);
            JS_AddModuleExport(_ctx, _mod, name.c_str());
            _exports.push_back({name, func});

            return *this;
        }

        // add class to module
        template <typename T>
        ClassBuilder<T> add_class(const std::string& name)
        {
            return ClassBuilder<T>{*this, name, _ctx};
        }

        const std::string& name() const { return _name; }
        JSContext* context() const { return _ctx; }
        JSModuleDef* module_def() const { return _mod; }

        void add_export(const std::string& name, JSValue value);

    private:
        template <auto Func>
        class FreeFunctionWrapper;

        template <typename T>
        friend class ClassBuilder;

        // Static members for module initialization
        static inline Module* current_module = nullptr;
        static int module_init_callback(JSContext* ctx, JSModuleDef* m);

        JSContext* _ctx{nullptr};
        JSModuleDef* _mod{nullptr};
        std::string _name;
        std::vector<detail::ModuleExportEntry> _exports{};
    };

    // class builder
    template <typename T>
    class ClassBuilder
    {
    public:
        ClassBuilder(Module& module, const std::string& name, JSContext* ctx)
            : _module(module), _name(name), _context(ctx), _proto(JS_UNDEFINED)
        {
        }

        ClassBuilder(const ClassBuilder&) = delete;
        ClassBuilder& operator=(const ClassBuilder&) = delete;

        ClassBuilder(ClassBuilder&& other) noexcept
            : _module(other._module), _name(std::move(other._name)), _context(other._context), _proto(other._proto)
        {
            other._proto = JS_UNDEFINED;
        }

        ClassBuilder& operator=(ClassBuilder&& other) noexcept
        {
            if (this != &other)
            {
                if (!JS_IsUndefined(_proto))
                {
                    JS_FreeValue(_context, _proto);
                }
                _module = std::move(other._module);
                _name = std::move(other._name);
                _context = other._context;
                _proto = other._proto;
                other._proto = JS_UNDEFINED;
            }
            return *this;
        }

        ~ClassBuilder()
        {
            // _proto is owned by the constructor
        }

        template <typename... Args>
        ClassBuilder& constructor(const std::string& ctor_name = "");

        template <auto Member>
        ClassBuilder& function(const std::string& name);

    private:
        template <typename ClassType, auto Member>
        struct MemberFunctionWrapper;

        template <typename ClassType, auto Member>
        struct MemberPropertyWrapper;

        // Get or create class ID for type T
        JSClassID get_or_create_class_id()
        {
            static JSClassID class_id = 0;
            if (class_id == 0)
            {
                JSRuntime* rt = JS_GetRuntime(_context);
                JS_NewClassID(rt, &class_id);

                if (!JS_IsRegisteredClass(rt, class_id))
                {
                    JSClassDef def = {
                        _name.c_str(),
                        // Finalizer
                        [](JSRuntime* rt, JSValue obj) noexcept
                        {
                            void* opaque = JS_GetOpaque(obj, JS_GetClassID(obj));
                            if (opaque)
                            {
                                T** pptr = static_cast<T**>(opaque);
                                delete *pptr; // delete the object
                                delete pptr;  // delete the pointer wrapper
                            }
                        },
                        nullptr, nullptr, nullptr};
                    JS_NewClass(rt, class_id, &def);
                }
            }
            return class_id;
        }

        Module& _module;
        std::string _name;
        JSContext* _context;
        JSValue _proto;
    };

    template <auto Func>
    class Module::FreeFunctionWrapper
    {
    public:
        using Traits = detail::FunctionTraits<decltype(Func)>;
        using ReturnType = typename Traits::ReturnType;

        static JSValue call(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv) noexcept
        {
            try
            {
                return invoke(ctx, argc, argv);
            }
            catch (...)
            {
                return JS_EXCEPTION;
            }
        }

    private:
        template <size_t... Is>
        static JSValue invoke_impl(JSContext* ctx, JSValueConst* argv, std::index_sequence<Is...>)
        {
            if constexpr (std::is_void_v<ReturnType>)
            {
                Func(detail::TypeConverter<detail::remove_cvref_t<typename Traits::template ArgType<Is>>>::from_js(ctx, argv[Is])...);
                return JS_UNDEFINED;
            }
            else
            {
                return detail::TypeConverter<ReturnType>::to_js(ctx, Func(detail::TypeConverter<detail::remove_cvref_t<typename Traits::template ArgType<Is>>>::from_js(ctx, argv[Is])...));
            }
        }

        static JSValue invoke(JSContext* ctx, int argc, JSValueConst* argv)
        {
            constexpr size_t N = Traits::arity;

            if constexpr (N == 0)
            {
                if constexpr (std::is_void_v<ReturnType>)
                {
                    Func();
                    return JS_UNDEFINED;
                }
                else
                {
                    return detail::TypeConverter<ReturnType>::to_js(ctx, Func());
                }
            }
            else if constexpr (N == 1 && is_rest_v<detail::remove_cvref_t<typename Traits::template ArgType<0>>>)
            {
                using Arg0 = typename Traits::template ArgType<0>;

                using ElementType = typename Arg0::value_type;

                Arg0 args;

                for (int i = 0; i < argc; ++i)
                {
                    args.push_back(detail::TypeConverter<ElementType>::from_js(ctx, argv[i]));
                }

                if constexpr (std::is_void_v<ReturnType>)
                {
                    Func(args);
                    return JS_UNDEFINED;
                }
                else
                {
                    return detail::TypeConverter<ReturnType>::to_js(ctx, Func(args));
                }
            }
            else
            {
                return invoke_impl(ctx, argv, std::make_index_sequence<N>{});
            }
        }
    };

    // Helper for constructor argument unwrapping
    namespace detail
    {
        template <typename T, typename... Args>
        struct ConstructorWrapper
        {
            template <size_t... Is>
            static T* create_impl(JSContext* ctx, JSValueConst* argv, std::index_sequence<Is...>)
            {
                return new T(TypeConverter<remove_cvref_t<Args>>::from_js(ctx, argv[Is])...);
            }

            static T* create(JSContext* ctx, int argc, JSValueConst* argv)
            {
                if (argc < static_cast<int>(sizeof...(Args)))
                {
                    JS_ThrowTypeError(ctx, "Expected %zu arguments but received %d", sizeof...(Args), argc);
                    return nullptr;
                }
                return create_impl(ctx, argv, std::make_index_sequence<sizeof...(Args)>{});
            }
        };

        template <typename T>
        struct ConstructorWrapper<T>
        {
            static T* create(JSContext* ctx, int, JSValueConst*)
            {
                (void)ctx;
                return new T();
            }
        };
    }

    // Helper to get class ID for type T at runtime
    namespace detail
    {
        template <typename T>
        struct ClassIDHolder
        {
            static JSClassID class_id;
        };

        template <typename T>
        JSClassID ClassIDHolder<T>::class_id = 0;
    }

    template <typename T>
    template <typename... Args>
    ClassBuilder<T>& ClassBuilder<T>::constructor(const std::string& ctor_name)
    {
        std::string cn = ctor_name.empty() ? _name : ctor_name;

        if (JS_IsUndefined(_proto))
        {
            _proto = JS_NewObject(_context);
        }

        JSClassID class_id = get_or_create_class_id();
        detail::ClassIDHolder<T>::class_id = class_id;

        auto wrapper = [](JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) -> JSValue
        {
            try
            {
                // Get prototype from constructor
                JSAtom proto_atom = JS_NewAtom(ctx, "prototype");
                JSValue proto = JS_GetProperty(ctx, this_val, proto_atom);
                JS_FreeAtom(ctx, proto_atom);

                if (JS_IsException(proto))
                {
                    JS_FreeValue(ctx, proto);
                    return JS_EXCEPTION;
                }

                // Create object with correct class
                JSClassID cid = detail::ClassIDHolder<T>::class_id;
                JSValue jsobj = JS_NewObjectProtoClass(ctx, proto, cid);
                JS_FreeValue(ctx, proto);

                if (JS_IsException(jsobj))
                {
                    return jsobj;
                }

                T* obj = detail::ConstructorWrapper<T, Args...>::create(ctx, argc, argv);
                if (!obj)
                {
                    JS_FreeValue(ctx, jsobj);
                    return JS_EXCEPTION;
                }

                T** pptr = new T*(obj);
                JS_SetOpaque(jsobj, pptr);
                return jsobj;
            }
            catch (const std::exception& e)
            {
                JS_ThrowInternalError(ctx, "Constructor failed: %s", e.what());
                return JS_EXCEPTION;
            }
            catch (...)
            {
                JS_ThrowInternalError(ctx, "Constructor failed: unknown exception");
                return JS_EXCEPTION;
            }
        };

        JSValue ctor = JS_NewCFunction2(_context, wrapper, cn.c_str(), sizeof...(Args), JS_CFUNC_constructor, 0);

        JSAtom proto_atom = JS_NewAtom(_context, "prototype");
        JS_DefinePropertyValue(_context, ctor, proto_atom, JS_DupValue(_context, _proto), JS_PROP_CONFIGURABLE | JS_PROP_WRITABLE);
        JS_FreeAtom(_context, proto_atom);

        JSAtom ctor_atom = JS_NewAtom(_context, "constructor");
        JS_DefinePropertyValue(_context, _proto, ctor_atom, JS_DupValue(_context, ctor), JS_PROP_CONFIGURABLE | JS_PROP_WRITABLE);
        JS_FreeAtom(_context, ctor_atom);

        JS_SetClassProto(_context, class_id, JS_DupValue(_context, _proto));

        _module.add_export(cn, ctor);

        return *this;
    }

    template <typename T>
    template <auto Member>
    ClassBuilder<T>& ClassBuilder<T>::function(const std::string& name)
    {
        using MemberType = decltype(Member);

        if constexpr (std::is_member_function_pointer_v<MemberType>)
        {
            using Wrapper = MemberFunctionWrapper<T, Member>;
            using Traits = detail::FunctionTraits<MemberType>;

            JSAtom atom = JS_NewAtom(_context, name.c_str());
            JSValue func = JS_NewCFunction2(_context, Wrapper::call, name.c_str(), Traits::arity, JS_CFUNC_generic, 0);

            if (JS_IsException(func))
            {
                JS_FreeAtom(_context, atom);
                throw js::Exception("Failed to create function: " + name);
            }

            JS_DefinePropertyValue(_context, _proto, atom, func, JS_PROP_CONFIGURABLE | JS_PROP_ENUMERABLE | JS_PROP_WRITABLE);
            JS_FreeAtom(_context, atom);
        }
        else
        {
            using Wrapper = MemberPropertyWrapper<T, Member>;

            JSAtom atom = JS_NewAtom(_context, name.c_str());
            JS_DefinePropertyGetSet(_context, _proto, atom,
                                    JS_NewCFunction2(_context, Wrapper::getter, name.c_str(), 0, JS_CFUNC_generic, 0),
                                    JS_NewCFunction2(_context, Wrapper::setter, name.c_str(), 1, JS_CFUNC_generic, 0),
                                    JS_PROP_CONFIGURABLE | JS_PROP_ENUMERABLE);
            JS_FreeAtom(_context, atom);
        }

        return *this;
    }

    template <typename T>
    template <typename ClassType, auto Member>
    struct ClassBuilder<T>::MemberFunctionWrapper
    {
        using Traits = detail::FunctionTraits<decltype(Member)>;
        using ReturnType = typename Traits::ReturnType;

        static JSValue call(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) noexcept
        {
            T** pptr = static_cast<T**>(JS_GetOpaque(this_val, JS_GetClassID(this_val)));
            if (!pptr || !*pptr)
            {
                return JS_ThrowTypeError(ctx, "Invalid C++ object");
            }

            ClassType* obj = *pptr;

            try
            {
                return invoke(ctx, obj, argc, argv);
            }
            catch (const std::exception& e)
            {
                JS_ThrowInternalError(ctx, "C++ exception: %s", e.what());
                return JS_EXCEPTION;
            }
            catch (...)
            {
                JS_ThrowInternalError(ctx, "Unknown C++ exception");
                return JS_EXCEPTION;
            }
        }

    private:
        template <size_t... Is>
        static JSValue invoke_impl(JSContext* ctx, ClassType* obj, JSValueConst* argv, std::index_sequence<Is...>)
        {
            if constexpr (std::is_void_v<ReturnType>)
            {
                (obj->*Member)(detail::TypeConverter<detail::remove_cvref_t<typename Traits::template ArgType<Is>>>::from_js(ctx, argv[Is])...);
                return JS_UNDEFINED;
            }
            else
            {
                return detail::TypeConverter<ReturnType>::to_js(ctx, (obj->*Member)(detail::TypeConverter<detail::remove_cvref_t<typename Traits::template ArgType<Is>>>::from_js(ctx, argv[Is])...));
            }
        }

        static JSValue invoke(JSContext* ctx, ClassType* obj, int, JSValueConst* argv)
        {
            constexpr size_t N = Traits::arity;

            if constexpr (N == 0)
            {
                if constexpr (std::is_void_v<ReturnType>)
                {
                    (obj->*Member)();
                    return JS_UNDEFINED;
                }
                else
                {
                    return detail::TypeConverter<ReturnType>::to_js(ctx, (obj->*Member)());
                }
            }
            else
            {
                return invoke_impl(ctx, obj, argv, std::make_index_sequence<N>{});
            }
        }
    };

    template <typename T>
    template <typename ClassType, auto Member>
    struct ClassBuilder<T>::MemberPropertyWrapper
    {
        static T** get_pptr(JSContext* ctx, JSValueConst this_val)
        {
            return static_cast<T**>(JS_GetOpaque(this_val, JS_GetClassID(this_val)));
        }

        static JSValue getter(JSContext* ctx, JSValueConst this_val, int, JSValueConst*) noexcept
        {
            T** pptr = get_pptr(ctx, this_val);
            if (!pptr || !*pptr)
            {
                return JS_UNDEFINED;
            }

            auto& val = (*pptr)->*Member;
            return detail::TypeConverter<detail::remove_cvref_t<decltype(val)>>::to_js(ctx, val);
        }

        static JSValue setter(JSContext* ctx, JSValueConst this_val, int, JSValueConst* argv) noexcept
        {
            T** pptr = get_pptr(ctx, this_val);
            if (!pptr || !*pptr)
            {
                return JS_EXCEPTION;
            }

            auto& member = (*pptr)->*Member;
            member = detail::TypeConverter<detail::remove_cvref_t<decltype(member)>>::from_js(ctx, argv[0]);
            return JS_UNDEFINED;
        }
    };
}
