load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")
load(
  "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
  "feature",
  "flag_group",
  "flag_set",
  "tool_path",
  "with_feature_set",
)

all_link_actions = [
  ACTION_NAMES.cpp_link_executable,
  ACTION_NAMES.cpp_link_dynamic_library,
  ACTION_NAMES.cpp_link_nodeps_dynamic_library,
]

all_cpp_compile_actions = [
  ACTION_NAMES.assemble,
  ACTION_NAMES.preprocess_assemble,
  ACTION_NAMES.linkstamp_compile,
  ACTION_NAMES.cpp_compile,
  ACTION_NAMES.cpp_header_parsing,
  ACTION_NAMES.cpp_module_codegen,
  ACTION_NAMES.cpp_module_compile,
  ACTION_NAMES.clif_match,
  ACTION_NAMES.lto_backend,
]

all_compile_actions = all_cpp_compile_actions + [ACTION_NAMES.c_compile]

def _impl(ctx):
  tool_paths = [
    tool_path(
      name = "gcc",
      path = "/usr/bin/clang",
    ),
    tool_path(
      name = "ld",
      path = "/usr/bin/ld",
    ),
    tool_path(
      name = "ar",
      path = "/usr/bin/ar",
    ),
    tool_path(
      name = "cpp",
      path = "/bin/false",
    ),
    tool_path(
      name = "gcov",
      path = "/bin/false",
    ),
    tool_path(
      name = "nm",
      path = "/bin/false",
    ),
    tool_path(
      name = "objdump",
      path = "/bin/false",
    ),
    tool_path(
      name = "strip",
      path = "/bin/false",
    ),
  ]

  linker_flags = feature(
    name = "default_linker_flags",
    enabled = True,
    flag_sets = [
      flag_set(
        actions = all_link_actions,
        flag_groups = ([
          flag_group(
            flags = [
              "-std=c++17",
              "-lstdc++",
            ],
          ),
        ]),
      ),
    ],
  )

  compiler_flags = feature(
    name = "default_compile_flags",
    enabled = True,
    flag_sets = [
      flag_set(
        actions = all_compile_actions,
        flag_groups = [
          flag_group(
            flags = [
              "-Wall",
              "-no-canonical-prefixes",
            ],
          ),
        ],
      ),
      flag_set(
        actions = all_cpp_compile_actions,
        flag_groups = [
          flag_group(
            flags = [
              "-std=c++17",
            ],
          ),
        ],
      ),
      flag_set(
        actions = all_compile_actions,
        flag_groups = [flag_group(flags = ["-DNDEBUG", "-O3"])],
        with_features = [with_feature_set(features = ["opt"])],
      ),
      flag_set(
        actions = all_compile_actions,
        flag_groups = [flag_group(flags = ["-g"])],
        with_features = [with_feature_set(features = ["dbg"])],
      ),
      flag_set(
        actions = all_compile_actions,
        flag_groups = [flag_group(flags = ["-O1"])],
        with_features = [with_feature_set(features = ["fastbuild"])],
      ),
    ],
  )

  dbg_feature = feature(name = "dbg")
  opt_feature = feature(name = "opt")

  features = [
    linker_flags,
    compiler_flags,
    dbg_feature,
    opt_feature,
  ]

  return cc_common.create_cc_toolchain_config_info(
    ctx = ctx,
    features = features,
    cxx_builtin_include_directories = [
      "/usr/lib/clang/15.0.7/include/",
      "/usr/include",
    ],
    toolchain_identifier = "local",
    host_system_name = "local",
    target_system_name = "local",
    target_cpu = "k8",
    target_libc = "unknown",
    compiler = "clang",
    abi_version = "unknown",
    abi_libc_version = "unknown",
    tool_paths = tool_paths,
  )

cc_toolchain_config = rule(
  implementation = _impl,
  attrs = {},
  provides = [CcToolchainConfigInfo],
)
