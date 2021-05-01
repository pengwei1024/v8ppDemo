# v8ppDemo
v8pp demo on osx

本仓库是 v8pp 在 mac os 的使用案例，编译的 libv8_monolith.a 也只能在 mac 上使用。
因为 v8pp 只支持 V8 7.x 及以下版本，本仓库依赖的V8版本是: 7.6.303.31.

编译分支:
```
git checkout -b 7.6 -t branch-heads/7.6
```

编译脚本：
```
is_debug = false
target_cpu = "x64"
use_custom_libcxx=false
is_component_build = false
is_clang = true
use_lld = false
v8_static_library = true
v8_monolithic = true
v8_use_external_startup_data = false
v8_enable_testtrue_features = false
v8_enable_i18n_support = false
treat_warnings_as_errors = false
symbol_level = 1
```
