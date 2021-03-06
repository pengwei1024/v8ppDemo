#include <iostream>
#include <v8pp/module.hpp>
#include <v8pp/class.hpp>
#include "v8.h"
#include "libplatform/libplatform.h"

/**
 * 官方简单的 V8 例子，执行字符串想加，并输出结果
 * @param argv
 */

struct X
{
    X(int v, bool u) : var(v) {}
    int var;
    int get() const { return var; }
    void set(int x) { var = x; }
};

void say_hello(char* argv[]) {
    // Initialize V8.
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(__FILE__);
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    printf("version=%s\n", v8::V8::GetVersion());
    // Create a new Isolate and make it the current one.
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator =
            v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);
    {
        v8::Isolate::Scope isolate_scope(isolate);
        // Create a stack-allocated handle scope.
        v8::HandleScope handle_scope(isolate);

        v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
        v8pp::module myLib(isolate);
        myLib.set_const("PI", 3.1415);
//
        v8pp::class_<X> X_class(isolate);
        X_class
                // specify X constructor signature
                .ctor<int, bool>()
                        // bind variable
                .set("var", &X::var)
                        // bind function
                .set("fun", &X::set)
                        // bind read-only property
                .set("prop",  v8pp::property(&X::get));
        myLib.set("X", X_class);

        // Create a new context.
        v8::Local<v8::Context> context = v8::Context::New(isolate, nullptr, global);

        // Enter the context for compiling and running the hello world script.
        v8::Context::Scope context_scope(context);
        {
            v8::Maybe<bool> success = context->Global()->Set(context, v8::String::NewFromUtf8(isolate, "myLib")
                                                             .ToLocalChecked(),
                                                             myLib.new_instance());
            printf("context->Global()->Set() success=%d\n", success.FromJust());
            // Create a string containing the JavaScript source code.
            v8::Local<v8::String> source =
                    v8::String::NewFromUtf8(isolate, "var x = new myLib.X(1024, true);(typeof myLib.X) "
                                                     "+ myLib.PI + '#Hello' + ', World!'").ToLocalChecked();

            // Compile the source code.
            v8::Local<v8::Script> script =
                    v8::Script::Compile(context, source).ToLocalChecked();

            // Run the script to get the result.
            v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

            // Convert the result to an UTF8 string and print it.
            v8::String::Utf8Value utf8(isolate, result);
            printf("result = %s\n", *utf8);
        }
    }
    // Dispose the isolate and tear down V8.
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete create_params.array_buffer_allocator;
}

int main(int argc, char *argv[]) {
    std::cout << "Hello, World!" << std::endl;
    say_hello(argv);
    return 0;
}
