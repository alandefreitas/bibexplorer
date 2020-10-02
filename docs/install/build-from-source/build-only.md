---
layout: default
title: Build Only
nav_order: 2
has_children: false
parent: Build from source
grand_parent: Install
has_toc: false
---
# Build Only

This will build everything in `./build` directory:

```bash
mkdir build
cmake -version
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O2"
cmake --build . -j 2 --config Release
```

On windows, replace `-O2` with `/O2`. You can replace `2` in `-j 2` with the number of cores in your computer.





<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
