cc_library(
    name = "logger",
    hdrs = ["logger.hpp"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    deps = [":logger"],
    copts = ["-std=c++20"],
)
