# StingrayKit

StingrayKit is a C++ toolkit with some useful stuff.

## Features

- thread-safety signals with support for synchronous and asynchronous handlers
- asynchronous task executors and timers
- RAII threads with cancellation tokens
- polymorphic collections inspired by IEnumerable from C#
- observable and transactional collections
- ranges and transformers
- nice logger and stuff for easy creation of to-stringable objects
- helpers for creation of comparable objects
- more...

## How-To build

You will need GCC, Cmake, make or ninja and POSIX-compatible operating system (Linux preferred and tested).

And GoogleTest to build tests.

Configure and build libstingraykit:

```sh
cmake -DPLATFORM_POSIX:BOOL=TRUE
make
```

Build and run hello world sample:
```sh
make stingraykit-hello-world
./build/stingraykit-hello-world
```

Configure, build and run tests:

```sh
cmake -DPLATFORM_POSIX:BOOL=TRUE -DGOOGLETEST_ROOT:STRING=/path/to/googletest
make stingraykit-test
./build/stingraykit-test
```
