# tcenv
ToolChain Env

# Development
Dependencies:
* `docker` or `podman`
* `just` (optional)
* `dive` (optional)

Project uses C++ modules, so it has strict toolchain requirements:
* CMake 3.26+
* Ninja 1.10+
* Clang 16+
* Libc++ 16+

Run `just` to see available commands:
```
Available recipes:
    build *ARGS              # build project. ARGS are passed to cmake, i.e. --target XXX
    build-image IMAGE='proj' # build container image
    configure *ARGS          # configure project. ARGS are passed to cmake, i.e. --fresh
    dive IMAGE='proj'        # inspect container image
    lf IMAGE='proj'          # browse files in container
    run STAGE *ARGS          # run 'tcenv' in specified stage. Example: just run 0 test
    shell IMAGE='proj'       # enter default shell of container
```

Or, read `justfile` to manually build images and project
