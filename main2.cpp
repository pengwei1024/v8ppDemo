#include <iostream>
#include "v8.h"
#include "V8Binding.h"
#include "libplatform/libplatform.h"

v8::SnapshotCreator *creator;

void nativeMethod(const v8::FunctionCallbackInfo<v8::Value> &args) {
    printf("call nativeMethod\n");
    v8::Local<v8::Object> object = args.Holder();
    printf("InternalFieldCount=%d\n", object->InternalFieldCount());
    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(object->GetInternalField(0));
    printf("InternalFieldCount=%d ptr=%p\n", object->InternalFieldCount(),wrap->Value());
    if (!wrap.IsEmpty()) {
        char * s = (char *)wrap->Value();
        printf("InternalField = %s\n", s);
    }
    args.GetReturnValue().Set(
            v8::String::NewFromUtf8(args.GetIsolate(), v8::V8::GetVersion())
                    .ToLocalChecked());
}


int main(int argc, char *argv[]) {
    // Initialize V8.
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(__FILE__);
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    printf("version=%s\n", v8::V8::GetVersion());
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator =
            v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    std::vector<intptr_t> external_references;
    external_references.push_back((intptr_t)nativeMethod);
    char* abc = "hello";
    external_references.push_back((intptr_t)abc);
    v8::SnapshotCreator create(external_references.data(), nullptr);
    v8binding::creator = &create;
//    auto isolate_ = v8::Isolate::Allocate();
//    v8::Isolate::Initialize(isolate_, create_params);
//    isolate_->Enter();
//    v8::Isolate* isolate_ = v8::Isolate::New(create_params);
    auto isolate_ = create.GetIsolate();
//    v8::Isolate::Initialize(isolate_, create_params);
//    creator = &create;

    {
        v8::Isolate::Scope isolate_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);
        // Create a new context.
        v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate_);
        v8::Local<v8::ObjectTemplate> swanObjTpl = v8::ObjectTemplate::New(isolate_);
        swanObjTpl->SetInternalFieldCount(1);
        global->Set(v8::String::NewFromUtf8(isolate_, "_na").ToLocalChecked(), swanObjTpl);
        v8::Local<v8::FunctionTemplate> fun = v8::FunctionTemplate::New(isolate_, &nativeMethod);
        swanObjTpl->Set(isolate_, "nativeMethod", fun);
        v8::Local<v8::Context> context = v8::Context::New(isolate_, nullptr, global);
        v8::Local<v8::Object> wrapper = swanObjTpl->NewInstance(context).ToLocalChecked();
        if (swanObjTpl->NewInstance(context).ToLocal(&wrapper)) {
            wrapper->SetInternalField(0, v8::External::New(isolate_, abc));
        }
        context->Global()->Set(context, v8::String::NewFromUtf8(isolate_, "_na").ToLocalChecked(), wrapper)
        .IsJust();
        create.AddContext(context);
        create.SetDefaultContext(context);

        // Enter the context for compiling and running the hello world script.
        v8::Context::Scope context_scope(context);
        {
            v8::Local<v8::String> source =
                    v8::String::NewFromUtf8(isolate_, "_na.nativeMethod() + ' - hello' + 123").ToLocalChecked();

            // Compile the source code.
            v8::Local<v8::Script> script =
                    v8::Script::Compile(context, source).ToLocalChecked();

            // Run the script to get the result.
            v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

            // Convert the result to an UTF8 string and print it.
            v8::String::Utf8Value utf8(isolate_, result);
            printf("result = %s\n", *utf8);
        }
    }
    auto data = create.CreateBlob(v8::SnapshotCreator::FunctionCodeHandling::kClear);
    printf("CreateBlob data=%d\n", data.raw_size);
}