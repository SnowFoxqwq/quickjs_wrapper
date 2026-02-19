#include <quickjs/quickjs.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

// Test global functions
void test_void_function()
{
    std::cout << "[C++] test_void_function called" << std::endl;
}

int32_t test_int_function(int32_t a, int32_t b)
{
    return a + b;
}

double test_float_function(double a, double b)
{
    return a * b;
}

std::string test_string_function(const std::string& s)
{
    return "echo: " + s;
}

// Test rest parameter function
void test_rest_function(js::rest<std::string> args)
{
    std::cout << "[C++] test_rest_function called with " << args.size() << " args: ";
    for (const auto& arg : args)
    {
        std::cout << arg << " ";
    }
    std::cout << std::endl;
}

// Test class
class TestClass
{
public:
    int32_t int_member = 42;
    double double_member = 3.14159;
    std::string string_member = "hello from c++";

    TestClass()
    {
        std::cout << "[C++] TestClass default constructor called" << std::endl;
    }

    TestClass(std::vector<int> vec)
    {
        std::cout << "[C++] TestClass vector constructor called (size: " << vec.size() << ")" << std::endl;
        vec_member = vec;
    }

    void void_member_function()
    {
        std::cout << "[C++] TestClass::void_member_function called" << std::endl;
    }

    int32_t int_member_function(int32_t a)
    {
        return a * 2;
    }

    std::string string_member_function(const std::string& s)
    {
        return "TestClass says: " + s;
    }

    std::vector<int> vec_member{};
};

void print_test_result(const std::string& test_name, bool success)
{
    std::cout << "[" << (success ? "PASS" : "FAIL") << "] " << test_name << std::endl;
}

int main()
{
    js::Runtime runtime;
    js::Context context(runtime);
    print_test_result("Runtime/Context initialization", runtime.is_valid() && context.is_valid());

    try
    {
        js::Module& my_module = context.add_module("TestModule")
                                    .function<&test_void_function>("testVoidFunction")
                                    .function<&test_int_function>("testIntFunction")
                                    .function<&test_float_function>("testFloatFunction")
                                    .function<&test_string_function>("testStringFunction")
                                    .function<&test_rest_function>("testRestFunction");

        my_module.add_class<TestClass>("TestClass")
            .constructor<>()
            .constructor<std::vector<int>>("TestClassWithVector")
            .function<&TestClass::int_member>("intMember")
            .function<&TestClass::double_member>("doubleMember")
            .function<&TestClass::string_member>("stringMember")
            .function<&TestClass::vec_member>("vecMember")
            .function<&TestClass::void_member_function>("voidMemberFunction")
            .function<&TestClass::int_member_function>("intMemberFunction")
            .function<&TestClass::string_member_function>("stringMemberFunction");

        // Import module
        context.eval(R"(
            import * as test from 'TestModule';
            globalThis.test = test;
            test.testRestFunction("[JS] Module imported successfully");
        )",
                     "<module_import>", js::JSEvalOptions::TYPE_MODULE);
        print_test_result("Module import", true);

        // Test void function
        context.eval(R"(
            test.testVoidFunction();
        )");
        print_test_result("Void function call", true);

        // Test int function
        auto int_result = context.eval(R"(
            test.testIntFunction(10, 20);
        )");
        print_test_result("Int function call", static_cast<int32_t>(int_result) == 30);

        // Test float function
        auto float_result = context.eval(R"(
            test.testFloatFunction(2.5, 4.0);
        )");
        print_test_result("Float function call", static_cast<double>(float_result) == 10.0);

        // Test string function
        auto string_result = context.eval(R"(
            test.testStringFunction("hello js");
        )");
        print_test_result("String function call", static_cast<std::string>(string_result) == std::string("echo: hello js"));

        // Test rest function
        context.eval(R"(
            test.testRestFunction("a", "b", "c", "d");
            test.testRestFunction("single arg");
        )");
        print_test_result("Rest function call", true);

        // Test class default constructor
        context.eval(R"(
            let obj1 = new test.TestClass();
        )");
        print_test_result("Class default constructor", true);

        // Test class vector constructor
        context.eval(R"(
            let obj2 = new test.TestClassWithVector([1,2,3,4,5]);
        )");
        print_test_result("Class vector constructor", true);

        // Test member variable read
        context.eval(R"(
            let obj3 = new test.TestClass();
            let val = obj3.intMember;
        )");
        print_test_result("Class member variable read", true);

        // Test prototype has member functions
        auto proto_check = context.eval(R"(
            let proto = test.TestClass.prototype;
            let hasVoid = typeof proto.voidMemberFunction === 'function';
            let hasInt = typeof proto.intMemberFunction === 'function';
            let hasString = typeof proto.stringMemberFunction === 'function';
            hasVoid && hasInt && hasString;
        )");
        print_test_result("Prototype has member functions", static_cast<bool>(proto_check));

        // Test instance void member function
        bool all_member_tests_passed = true;
        (void)(all_member_tests_passed);

        try
        {
            context.eval(R"(
                let obj9 = new test.TestClass();
                obj9.voidMemberFunction();
            )");
            print_test_result("Instance void member function call", true);
        }
        catch (const js::Exception&)
        {
            print_test_result("Instance void member function call", false);
            all_member_tests_passed = false;
        }

        // Test int member function
        try
        {
            auto int_result = context.eval(R"(
                let obj10 = new test.TestClass();
                obj10.intMemberFunction(100);
            )");
            print_test_result("Instance int member function call", static_cast<int32_t>(int_result) == 200);
            if (static_cast<int32_t>(int_result) != 200) all_member_tests_passed = false;
        }
        catch (const js::Exception&)
        {
            print_test_result("Instance int member function call", false);
            all_member_tests_passed = false;
        }

        // Test string member function
        try
        {
            auto str_result = context.eval(R"(
                let obj11 = new test.TestClass();
                obj11.stringMemberFunction("js call");
            )");
            print_test_result("Instance string member function call", static_cast<std::string>(str_result) == "TestClass says: js call");
            if (static_cast<std::string>(str_result) != "TestClass says: js call") all_member_tests_passed = false;
        }
        catch (const js::Exception&)
        {
            print_test_result("Instance string member function call", false);
            all_member_tests_passed = false;
        }

        // Test JS callback function to C++
        context.eval(R"(
            globalThis.jsCallback = function(msg, num) {
                return num * 2;
            };
        )");

        // Get JS callback function and convert to std::function
        js::Value cb_val = context.eval("jsCallback");
        print_test_result("JS function is_function check", cb_val.is_function());

        std::function<int32_t(const std::string&, int32_t)> cpp_cb = cb_val;
        int32_t cb_result = cpp_cb("hello from c++", 123);
        print_test_result("JS callback call from C++", cb_result == 246);

        // Test global object access
        js::Value global = context.get_global();
        js::Value global_test = global["test"];
        print_test_result("Global object access", static_cast<bool>(global_test));

        // Test exception handling
        bool exception_caught = false;
        try
        {
            context.eval("this is invalid javascript code");
        }
        catch (const js::Exception& e)
        {
            exception_caught = true;
            js::Value exc = context.get_exception();
            js::Value stack = exc["stack"];
            if (static_cast<bool>(stack))
            {
                std::cout << "[JS] Expected exception caught: " << static_cast<std::string>(stack) << std::endl;
            }
            else
            {
                std::cout << "[JS] Expected exception caught: " << static_cast<std::string>(exc) << std::endl;
            }
            std::cout << "[ERROR]: " << e.what() << std::endl;
        }
        print_test_result("Exception handling", exception_caught);

        // Test type conversion
        auto type_convert_result = context.eval(R"(
            let boolVal = true;
            let nullVal = null;
            let undefinedVal = undefined;
            true;
        )");
        print_test_result("Type conversion (JS side)", static_cast<bool>(type_convert_result));

        // Test global variables
        context.add_variable("globalVar", 42)
            .add_variable("globalString", "Hello from C++")
            .add_constant("GLOBAL_CONST", 3.14159);

        auto global_var_result = context.eval(R"(
            globalVar + 8;
        )");
        print_test_result("Global variable access", static_cast<int32_t>(global_var_result) == 50);

        auto global_string_result = context.eval(R"(
            globalString + " World!";
        )");
        print_test_result("Global string variable", static_cast<std::string>(global_string_result) == "Hello from C++ World!");

        auto global_const_result = context.eval(R"(
            GLOBAL_CONST * 2;
        )");
        print_test_result("Global constant access", static_cast<double>(global_const_result) == 6.28318);

        // All tests passed
        std::cout << "\n===== All Tests Completed =====" << std::endl;
    }
    catch (const js::Exception& e)
    {
        std::cerr << "\n[ERROR] Uncaught JS exception: " << e.what() << std::endl;
        js::Value exc = context.get_exception();
        if (static_cast<bool>(exc))
        {
            std::cerr << "[ERROR] Exception details: " << static_cast<std::string>(exc) << std::endl;
            js::Value stack = exc["stack"];
            if (static_cast<bool>(stack))
            {
                std::cerr << "[ERROR] Stack trace: " << static_cast<std::string>(stack) << std::endl;
            }
        }
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "\n[ERROR] C++ exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
