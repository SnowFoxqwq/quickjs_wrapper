#include "module.hpp"

namespace js
{
    // Static members defined in header as inline

    Module::Module(const std::string& name, JSContext* ctx)
        : _ctx(ctx), _mod(nullptr), _name(name)
    {
        current_module = this;
        _mod = JS_NewCModule(ctx, name.c_str(), module_init_callback);
    }

    Module::~Module()
    {
        if (!_ctx) return;

        for (auto& entry : _exports)
        {
            if (!JS_IsUndefined(entry.value))
            {
                JS_FreeValue(_ctx, entry.value);
            }
        }
    }

    Module::Module(Module&& other) noexcept
        : _ctx(other._ctx), _mod(other._mod), _name(std::move(other._name)), _exports(std::move(other._exports))
    {
        other._ctx = nullptr;
        other._mod = nullptr;
    }

    Module& Module::operator=(Module&& other) noexcept
    {
        if (this != &other)
        {
            for (auto& entry : _exports)
            {
                if (_ctx && !JS_IsUndefined(entry.value))
                {
                    JS_FreeValue(_ctx, entry.value);
                }
            }
            _exports.clear();

            _ctx = other._ctx;
            _mod = other._mod;
            _name = std::move(other._name);
            _exports = std::move(other._exports);
            other._ctx = nullptr;
            other._mod = nullptr;
        }
        return *this;
    }

    int Module::module_init_callback(JSContext* ctx, JSModuleDef* m)
    {
        (void)ctx;
        (void)m;

        if (!current_module)
        {
            return -1;
        }

        for (auto& entry : current_module->_exports)
        {
            JSValue val = JS_DupValue(current_module->_ctx, entry.value);
            JS_SetModuleExport(current_module->_ctx, current_module->_mod, entry.name.c_str(), val);
        }

        return 0;
    }

    void Module::add_export(const std::string& name, JSValue value)
    {
        JS_AddModuleExport(_ctx, _mod, name.c_str());
        _exports.push_back({name, value});
    }
}
