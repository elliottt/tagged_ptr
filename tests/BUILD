
test_suite(
    name = "tests",
    tests = [
        "unique_ptr_test",
    ],
)

cc_test(
    name = "unique_ptr_test",
    srcs = ["unique_ptr_test.cc"],
    copts = ["-std=c++17"],
    deps = [
        "//tagged",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)
