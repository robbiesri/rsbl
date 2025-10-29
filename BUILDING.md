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
$ python_local/python.exe scripts/setup_git_subtrees.py --update
```