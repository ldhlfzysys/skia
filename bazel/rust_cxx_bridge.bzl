"""
This file specifies a macro that generates a C++ .h and .cc file
based on a rust file. It uses the cxx crate [1], specifically the
cxxbridge executable. This is based off of the example provided in [2].


[1] https://cxx.rs/build/bazel.html
[2] https://github.com/dtolnay/cxx/blob/e7576dc938f60512edfd1f7525e649b959b9dee6/tools/bazel/rust_cxx_bridge.bzl
"""

load("@bazel_skylib//rules:run_binary.bzl", "run_binary")
load("@rules_cc//cc:defs.bzl", "cc_library")
load(
    "//bazel:skia_rules.bzl",
    "skia_filegroup",
)

def rust_cxx_bridge(name, src, deps = [], visibility = []):
    """Creates rules for CXX C++/Rust bridging.

    Takes a Rust source file to generate binding code from a section that is marked with `#[cxx::bridge]`.

    Args:
      name: Name of the CXX bridge rule, used for sub-rules.
      src: Source file that contains CXX bridge definitions.
      deps: Rules this rule depends on.
      visibility: Visibility of the generated sub-rules.
    """
    out_h = "gen/%s.h" % src
    out_cc = "gen/%s.cc" % src
    run_binary(
        name = "%s/generated" % name,
        srcs = [src],
        outs = [out_h, out_cc],
        args = [
            "$(location %s)" % src,
            "-o",
            "$(location %s)" % out_h,
            "-o",
            "$(location %s)" % out_cc,
        ],
        tool = "@cxxbridge_cmd//:cxxbridge",
    )

    skia_filegroup(
        name = "%s/filegroup" % name,
        srcs = [out_cc],
        visibility = visibility,
    )

    cc_library(
        name = name,
        srcs = [out_cc],
        hdrs = [out_h],
        deps = deps + ["%s/include" % name],
        visibility = visibility,
    )

    cc_library(
        name = "%s/include" % name,
        hdrs = [out_h],
        visibility = visibility,
    )
