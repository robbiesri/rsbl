# How to Build

## Build Environment

We currently rely on Python3 and CMake to manage the build environment. We also need `git` to be available,
which means using an externally installed version of Git for Windows. Perhaps in the future, we can fetch and
configure a local version of `git`.

We will use setup scripts to download Python and CMake, and set up `git` subtrees defined in `subtrees.toml`.

The core build environment can be setup by running the `setup_build_env.sh` script

```sh
$ scripts/setup_build_env.sh
```

### Subtrees

The definitions for what subtrees are downloaded are contained in `subtrees.toml`. The TOML should already
be populated as part of this repo, and the subtrees will be pulled down as part of this repo.

If you need to update the already existing subtrees, update `subtrees.toml` and then run:

```sh
$ build_env/python/python.exe scripts/setup_git_subtrees.py --update
```

### Visual Studio

If you want to use Visual Studio as the generator, it's pretty simple with `-G`. Make sure you mark it with `-A x64` to
get 64-bit builds, which are required!

```shell
> .\build_env\cmake\cmake-4.1.2-windows-x86_64\bin\cmake.exe -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 17 2022" -A x64 -S C:\funsrc\rsbl -B C:\funsrc\rsbl\cmake-test
```

### Ninja + MSVC

If you want to use Ninja + MSVC, it's a bit trickier. You **need** to open the
`x64 Native Tools Command Prompt for VS 2022`, and run `cmake` from that command prompt. There's really no other way to
give `cmake` enough visibility into the MSVC toolchain. Too bad there isn't a clearer way to do this, since the Visual
Studio path is so simple.

```shell
> .\build_env\cmake\cmake-4.1.2-windows-x86_64\bin\cmake.exe -DCMAKE_BUILD_TYPE=Debug "-DCMAKE_MAKE_PROGRAM=C:/funsrc/rsbl/build_env/ninja/ninja.exe" -G Ninja -S C:\funsrc\rsbl -B C:\funsrc\rsbl\cmake-test-ninja
> .\build_env\cmake\cmake-4.1.2-windows-x86_64\bin\cmake.exe --build C:\funsrc\rsbl\cmake-test-ninja --target rsbl-core -j 10
```

Claude suggested this setup, by invoking `vcvars64.bat`. This is a great idea, and I'm leaving both paths here as
examples.

```shell
"C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" && cd "C:\funsrc\rsbl" && mkdir -p cmake-test-ninja && cd
    cmake-test-ninja && cmake -G Ninja -DCMAKE_MAKE_PROGRAM="C:/funsrc/rsbl/build_env/ninja/ninja.exe" .. && cmake --build . --target rsbl-result-test
```

The first command sets up the build, and the second command actually builds the `rsbl-core` library target.