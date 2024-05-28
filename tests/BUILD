
test_suite(
    name = "tests",
    tests = [
        "unique_ptr_test",
        "shared_ptr_test",
        "readme_example",
    ],
)

cc_test(
    name = "unique_ptr_test",
    srcs = ["unique_ptr_test.cc"],
    copts = [
        "-std=c++17",

        # Warnings disabled for testing bad behavior
        "-Wno-self-move",
    ],
    linkstatic = True,
    deps = [
        "//tagged",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

cc_test(
    name = "shared_ptr_test",
    srcs = ["shared_ptr_test.cc"],
    copts = [
        "-std=c++17",

        # Warnings disabled for testing bad behavior
        "-Wno-self-move",
    ],
    linkstatic = True,
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
    linkstatic = True,
    deps = [
        "//tagged",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)
