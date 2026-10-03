#!/usr/bin/env python3
"""Doxygen INPUT_FILTER, only used for the docs build (source is untouched).

Breathe 4.36 asserts on the C++11 form `friend Foo;` (no class keyword),
e.g. in DgZ3RF.h / DgZ7RF.h. Rewrite it to `friend class Foo;`.
"""
import re
import sys

with open(sys.argv[1], encoding="utf-8", errors="replace") as f:
    sys.stdout.write(re.sub(r"^(\s*friend)\s+(\w+)\s*;", r"\1 class \2;",
                            f.read(), flags=re.M))
