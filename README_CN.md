# QuickJS Wrapper

[ENGLISH](README_EN.md)

一个现代化的 C++ 封装库，用于 [QuickJS](https://bellard.org/quickjs/) - 一个小巧且可嵌入的 JavaScript 引擎。

## 特性

- **现代 C++17 API** - 简洁直观的接口，使用 RAII 内存管理
- **类型安全** - C++ 与 JavaScript 之间的自动类型转换
- **模块系统** - 将 C++ 函数和类暴露给 JavaScript 作为模块
- **类绑定** - 轻松绑定 C++ 类，支持构造函数、方法和属性
- **异常处理** - 支持 C++ 异常
- **静态库** - 易于集成到你的项目中

## 环境要求

- 支持 C++17 的编译器
- [xmake](https://xmake.io/) 构建系统
- QuickJS 库（由 xmake 自动获取）

## 许可证 / License

本项目基于 **Apache License 2.0** 协议开源。

This project is open sourced under the **Apache License 2.0**.

详细内容请查看 [LICENSE](LICENSE) 文件。

## 快速开始

### 构建库

```bash
xmake
```

这会将静态库构建到 `lib/$(arch)-$(mode)/` 目录。

### 在项目中使用

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
        print(m.add(10, 20));  // 输出: 30
    )", "<script>", js::JSEvalOptions::TYPE_MODULE);

    return 0;
}
```

### 使用 xmake 链接

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

查看 `examples/xmake.lua` 获取完整示例。

## 示例

### 类绑定

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
        print(calc.get_result());  // 输出: 10
    )", "<script>", js::JSEvalOptions::TYPE_MODULE);

    return 0;
}
```

### 异常处理

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

### 全局变量

```cpp
// 添加变量到全局对象
context.add_variable("myVar", 42);
context.add_variable("myString", "Hello from C++");
context.add_variable("myDouble", 3.14);

// 添加常量到全局对象
context.add_constant("PI", 3.14159);
context.add_constant("APP_NAME", "MyApp");

// 在 JavaScript 中使用
context.eval(R"(
    print(myVar);        // 输出: 42
    print(myString);     // 输出: Hello from C++
    print(PI);           // 输出: 3.14159
)");
```

## API 概览

### Runtime（运行时）
```cpp
js::Runtime runtime;  // 创建一个新的 QuickJS 运行时
```

### Context（上下文）
```cpp
js::Context context(runtime);           // 创建上下文
js::Value result = context.eval(code);  // 执行 JS 代码
js::Module& mod = context.add_module("Name");  // 添加模块
js::Value global = context.get_global();       // 获取全局对象

// 添加全局变量/常量
context.add_variable("varName", value);    // 添加变量
context.add_constant("CONST_NAME", value); // 添加常量
```

### Module（模块）
```cpp
// 添加函数
mod.function<&cpp_function>("jsFunctionName");

// 添加类
mod.add_class<CPPClass>("JSClassName")
    .constructor<>()                    // 默认构造函数
    .constructor<Arg1, Arg2>()          // 带参数的构造函数
    .function<&CPPClass::method>("methodName");
```

### Value（值）
```cpp
// 类型检查
value.is_undefined() / is_null() / is_function() / is_error() / is_array()

// 类型转换
int32_t i = value.to_int32();
double d = value.to_float64();
std::string s = value.to_string();
bool b = value.to_bool();

// 属性访问
js::Value prop = value["propertyName"];
js::Value elem = value[0];  // 数组索引

// 函数调用
js::Value result = value.call({arg1, arg2});
```

## 项目结构

```
quickjs_wrapper/
├── include/quickjs/     # 公共头文件
│   ├── quickjs.hpp      # 主头文件
│   ├── runtime.hpp
│   ├── context.hpp
│   ├── value.hpp
│   ├── module.hpp
│   └── ...
├── src/                 # 实现文件
├── examples/            # 示例项目
│   ├── main.cpp
│   └── xmake.lua
└── xmake.lua           # 构建配置
```

## 致谢

- [QuickJS](https://bellard.org/quickjs/) by Fabrice Bellard
- 受到 [quickjspp](https://github.com/ftk/quickjspp) 的启发
