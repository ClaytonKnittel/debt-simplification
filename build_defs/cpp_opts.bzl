"""C++ compile/link options."""

COPTS = select({
  "//conditions:default": [
    "-Wno-c++17-extensions",
  ],
})
