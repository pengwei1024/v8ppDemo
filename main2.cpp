#include <iostream>
#include "v8.h"
#include "V8Binding.h"
#include "libplatform/libplatform.h"
#include "SnapshotUtil.h"

#define MAKE_SNAPSHOT

struct PersistentWrapper {
    v8::Persistent<v8::Object> persistent;
    char* msg{};
};

void nativeMethod(const v8::FunctionCallbackInfo<v8::Value> &args) {
    printf("call nativeMethod\n");
    v8::HandleScope handle_scope(args.GetIsolate());
    v8::Local<v8::Object> object = args.Holder();
    printf("InternalFieldCount=%d\n", object->InternalFieldCount());
    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(object->GetInternalField(0));
    printf("InternalFieldCount=%d ptr=%p\n", object->InternalFieldCount(), wrap->Value());
    if (!wrap.IsEmpty()) {
        auto* wrapper = static_cast<PersistentWrapper *>(wrap->Value());
        printf("InternalField = %s\n", wrapper->msg);
    }
    args.GetReturnValue().Set(
            v8::String::NewFromUtf8(args.GetIsolate(), v8::V8::GetVersion())
                    .ToLocalChecked());
}



void WeakCallback(const v8::WeakCallbackInfo<PersistentWrapper> & data) {
    printf("call WeakCallback");
    PersistentWrapper* wrapper = data.GetParameter();
    wrapper->persistent.Reset();
    delete wrapper;
}


int main(int argc, char *argv[]) {
    // Initialize V8.
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(__FILE__);
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    printf("version=%s\n", v8::V8::GetVersion());
#ifndef MAKE_SNAPSHOT
    v8::StartupData initData = {nullptr, 0};
    SnapshotUtil::readFile(initData);
#endif
    std::vector<intptr_t> external_references;
    external_references.push_back((intptr_t) nativeMethod);
    auto* persistentWrapper = new PersistentWrapper();
    persistentWrapper->msg = "abc";
    external_references.push_back((intptr_t) persistentWrapper);
#ifndef MAKE_SNAPSHOT
    v8::SnapshotCreator create(external_references.data(), &initData);
#else
    auto *create = new v8::SnapshotCreator(external_references.data());
#endif
    auto isolate_ = create->GetIsolate();
    {
        v8::Isolate::Scope isolate_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);
        // Create a new context.
        v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate_);
        v8::Local<v8::Context> context = v8::Context::New(isolate_, nullptr, global);
#ifdef MAKE_SNAPSHOT
        v8::Local<v8::ObjectTemplate> swanObjTpl = v8::ObjectTemplate::New(isolate_);
        swanObjTpl->SetInternalFieldCount(1);
        global->Set(v8::String::NewFromUtf8(isolate_, "_na").ToLocalChecked(), swanObjTpl);
        v8::Local<v8::FunctionTemplate> fun = v8::FunctionTemplate::New(isolate_, &nativeMethod);
        swanObjTpl->Set(isolate_, "nativeMethod", fun);
        v8::Local<v8::Object> wrapper = swanObjTpl->NewInstance(context).ToLocalChecked();
        if (swanObjTpl->NewInstance(context).ToLocal(&wrapper)) {
            wrapper->SetInternalField(0, v8::External::New(isolate_, persistentWrapper));
        }

        persistentWrapper->persistent.Reset(isolate_, wrapper);
        persistentWrapper->persistent.SetWeak(persistentWrapper, WeakCallback, v8::WeakCallbackType::kInternalFields);
        create->AddData(wrapper);
        context->Global()->Set(context, v8::String::NewFromUtf8(isolate_, "_na")
                .ToLocalChecked(), wrapper).IsJust();
        create->AddContext(context);
        create->SetDefaultContext(context);
#endif

        // Enter the context for compiling and running the hello world script.
        v8::Context::Scope context_scope(context);
        {
            v8::Local<v8::String> source =
                    v8::String::NewFromUtf8(isolate_, "_na.nativeMethod() + ' - hello' + 123").ToLocalChecked();
//                    v8::String::NewFromUtf8(isolate_, "' - hello' + 123").ToLocalChecked();

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
#ifdef MAKE_SNAPSHOT
    auto data = create->CreateBlob(v8::SnapshotCreator::FunctionCodeHandling::kClear);
    printf("CreateBlob data=%d\n", data.raw_size);
    SnapshotUtil::writeFile(data);
#endif
//    isolate_->Exit();
//    isolate_->Dispose();
    delete create;
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
}