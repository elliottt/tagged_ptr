
test_suite(
    name = "tests",
    tests = [
        "unique_ptr_test",
        "readme_example",
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

cc_test(
    name = "readme_example",
    srcs = ["readme_example.cc"],
    copts = ["-std=c++17"],
    deps = [
        "//tagged",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)
