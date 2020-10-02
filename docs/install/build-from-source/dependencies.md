---
layout: default
title: Dependencies
nav_order: 1
has_children: false
parent: Build from source
grand_parent: Install
has_toc: false
---
# Dependencies

This section lists the dependencies you need before installing BibExplorer from source:

* C++17
* CMake 3.14 or higher
* OpenGL
* Curl


Instructions: Linux/Ubuntu/GCC
    
Check your GCC version

```bash
g++ --version
```

The output should be something like

```console
g++-8 (Ubuntu 8.4.0-1ubuntu1~18.04) 8.4.0
```

If you see a version before GCC-8, update it with

```bash
sudo apt update
sudo apt install gcc-8
sudo apt install g++-8
```

To update to any other version, like GCC-9 or GCC-10:

```bash
sudo apt install build-essential
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt install g++-10
```

Once you installed a newer version of GCC, you can link it to `update-alternatives`. For instance, if you have GCC-7 and GCC-10, you can link them with:

```bash
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 7
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 7
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 10
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 10
```

You can now use `update-alternatives` to set you default `gcc` and `g++`:

```bash
update-alternatives --config g++
update-alternatives --config gcc
```

Check your CMake version:

```bash
cmake --version
```

If it's older than CMake 3.14, update it with

```bash
sudo apt upgrade cmake
```

or download the most recent version from [cmake.org](https://cmake.org/).

[Later](#build-the-examples) when running CMake, make sure you are using GCC-8 or higher by appending the following options:

```bash
-DCMAKE_C_COMPILER=/usr/bin/gcc-8 -DCMAKE_CXX_COMPILER=/usr/bin/g++-8
```

Install OpenGL / GLFW3:

```bash
sudo apt-get install libglfw3-dev
```

Install curl:

```bash
sudo apt install curl
sudo apt-get install libcurl4-gnutls-dev
```

Install OpenSSL:

```bash
sudo apt-get install libssl-dev
```




Instructions: Mac Os/Clang

Check your Clang version:

```bash
clang --version
```

The output should be something like

```console
Apple clang version 11.0.0 (clang-1100.0.33.8)
```

If you see a version before Clang 11, update XCode in the App Store or update clang with homebrew. 

Check your CMake version:

```bash
cmake --version
```

If it's older than CMake 3.14, update it with

```bash
sudo brew upgrade cmake
```

or download the most recent version from [cmake.org](https://cmake.org/).

If the last command fails because you don't have [Homebrew](https://brew.sh) on your computer, you can install it with

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
```

or you can follow the instructions in [https://brew.sh](https://brew.sh).

Download GLFW3 from https://www.glfw.org

Install OpenSSL:

```bash
brew install openssl
```




Instructions: Windows/MSVC
    
* Make sure you have a recent version of [Visual Studio](https://visualstudio.microsoft.com)
* Install [Git](https://git-scm.com/download/win)
* Install [CMake](https://cmake.org/download/)
* Install [GLFW3](https://www.glfw.org)

Install [VCPKG](https://github.com/microsoft/vcpkg):

```bash
git clone https://github.com/microsoft/vcpkg
> .\vcpkg\bootstrap-vcpkg.bat
```

Install [libcurl](https://curl.haxx.se/download.html):

```bash
vcpkg install curl
```



Some other small dependencies, like header-only libraries, if not found, will be download at compile-time by the build script. You can see these dependencies in [source/CMakeLists.txt](https://github.com/alandefreitas/bibexplorer/blob/master/source/CMakeLists.txt).




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
