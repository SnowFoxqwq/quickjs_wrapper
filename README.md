# QuickJS Wrapper

[简体中文](README_CN.md)

A modern C++ wrapper for [QuickJS](https://bellard.org/quickjs/) - a small and embeddable JavaScript engine.

## Features

- **Modern C++17 API** - Clean, intuitive interface with RAII memory management
- **Type Safety** - Automatic type conversion between C++ and JavaScript
- **Module System** - Expose C++ functions and classes to JavaScript as modules
- **Class Binding** - Easy binding of C++ classes with constructors, methods, and properties
- **Exception Handling** - C++ exception support
- **Static Library** - Easy to integrate into your projects

## Requirements

- C++17 compatible compiler
- [xmake](https://xmake.io/) build system
- QuickJS library (automatically fetched by xmake)

## License

This project is licensed under the **Apache License 2.0**.

See [LICENSE](LICENSE) for details.

## Quick Start

### Build the library

```bash
xmake
```

This will build the static library to `lib/$(arch)-$(mode)/`.

### Use in your project

```cpp
#include <quickjs/quickjs.hpp>
#include <iostream>

int add(int a, int b) 
{
    return a + b;
}

int main() 
{
    js::Runtime runtime;
    js::Context context(runtime);

    context.add_module("MyModule")
        .function<&add>("add");

    context.eval(R"(
        import * as m from 'MyModule';
        print(m.add(10, 20));  // Output: 30
    )", "<script>", js::JSEvalOptions::TYPE_MODULE);

    return 0;
}
```

### Link with xmake

```lua
-- xmake.lua
set_project("myapp")
set_languages("c++17")

add_requires("quickjs-ng", {alias = "quickjs", configs = {libc = true}})

local LIB_ROOT = "path/to/quickjs_wrapper"

target("myapp")
    set_kind("binary")
    add_files("main.cpp")

    add_includedirs(path.join(LIB_ROOT, "include"))
    add_linkdirs(path.join(LIB_ROOT, "lib", "$(arch)-$(mode)"))
    add_links("quickjs_wrapper")
    add_packages("quickjs")
target_end()
```

See `examples/xmake.lua` for a complete example.

## Examples

### Class Binding

```cpp
#include <quickjs/quickjs.hpp>

class Calculator 
{
public:
    double value = 0;
    void add(double n) { value += n; }
    double get_result() const { return value; }
};

int main() 
{
    js::Runtime runtime;
    js::Context context(runtime);

    context.add_module("Calc")
        .add_class<Calculator>("Calculator")
        .constructor<>()
        .function<&Calculator::add>("add")
        .function<&Calculator::get_result>("get_result");

    context.eval(R"(
        import { Calculator } from 'Calc';
        const calc = new Calculator();
        calc.add(10);
        print(calc.get_result());  // Output: 10
    )", "<script>", js::JSEvalOptions::TYPE_MODULE);

    return 0;
}
```

### Exception Handling

```cpp
try 
{
    context.eval("invalid javascript code");
} 
catch (const js::Exception& e) 
{
    std::cerr << "Error: " << e.what() << std::endl;
}
```

### Global Variables

```cpp
// Add variables to global object
context.add_variable("myVar", 42);
context.add_variable("myString", "Hello from C++");
context.add_variable("myDouble", 3.14);

// Add constants to global object
context.add_constant("PI", 3.14159);
context.add_constant("APP_NAME", "MyApp");

// Use in JavaScript
context.eval(R"(
    print(myVar);        // Output: 42
    print(myString);     // Output: Hello from C++
    print(PI);           // Output: 3.14159
)");
```

## API Overview

### Runtime
```cpp
js::Runtime runtime;  // Creates a new QuickJS runtime
```

### Context
```cpp
js::Context context(runtime);           // Creates a context
js::Value result = context.eval(code);  // Evaluate JS code
js::Module& mod = context.add_module("Name");  // Add a module
js::Value global = context.get_global();       // Get global object

// Add global variables/constants
context.add_variable("varName", value);   // Add variable
context.add_constant("CONST_NAME", value); // Add constant
```

### Module
```cpp
// Add function
mod.function<&cpp_function>("jsFunctionName");

// Add class
mod.add_class<CPPClass>("JSClassName")
    .constructor<>()                    // Default constructor
    .constructor<Arg1, Arg2>()          // Constructor with arguments
    .function<&CPPClass::method>("methodName");
```

### Value
```cpp
// Type checking
value.is_undefined() / is_null() / is_function() / is_error() / is_array()

// Type conversion
int32_t i = value.to_int32();
double d = value.to_float64();
std::string s = value.to_string();
bool b = value.to_bool();

// Property access
js::Value prop = value["propertyName"];
js::Value elem = value[0];  // Array index

// Function call
js::Value result = value.call({arg1, arg2});
```

## Project Structure

```
quickjs_wrapper/
├── include/quickjs/     # Public headers
│   ├── quickjs.hpp      # Main header
│   ├── runtime.hpp
│   ├── context.hpp
│   ├── value.hpp
│   ├── module.hpp
│   └── ...
├── src/                 # Implementation
├── examples/            # Example project
│   ├── main.cpp
│   └── xmake.lua
└── xmake.lua           # Build config
```

## Acknowledgments

- [QuickJS](https://bellard.org/quickjs/) by Fabrice Bellard
- Inspired by [quickjspp](https://github.com/ftk/quickjspp)