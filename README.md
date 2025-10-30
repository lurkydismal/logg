<!-- :toc: macro -->
<!-- :toc-title: -->
<!-- :toclevels: 99 -->

# logg <!-- omit from toc -->

> A small, header-only C++23 logging helper.

## Table of Contents <!-- omit from toc -->

* [General Information](#general-information)
* [Technologies Used](#technologies-used)
* [Features](#features)
* [Setup](#setup)
* [Usage](#usage)
* [Project Status](#project-status)
* [Room for Improvement](#room-for-improvement)
* [License](#license)

## General Information

* It prints `INFO`, `WARNING`, `ERROR` lines with the calling thread id, filename, line number and function name.
* Provides leveled console output with source location and colored formatting. Designed to be drop-in for projects that already use `stdfunc` utilities.

## Technologies Used

<!--
GNU bash, version 5.3.3(1)-release (x86_64-pc-linux-gnu)
Copyright (C) 2025 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>

This is free software; you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
-->
* GNU bash 5.3.3
<!--
clang version 21.1.4
Target: x86_64-pc-linux-gnu
Thread model: posix

Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
See https://llvm.org/LICENSE.txt for license information.
-->
* clang 21.1.4
* gtest - 1.17.0-1
* [stdfunc](https://github.com/lurkydismal/stdfunc)

## Features

`header_frequency.sh` helps pick candidates for a precompiled header by counting `#include` occurrences.

The build system must `source` or `eval` the module's `config.sh`. The template uses the following variables:

```bash
#!/bin/bash
export FILES_TO_INCLUDE='include/*.hpp'
export FILES_TO_COMPILE='src/*.cpp'
```

* `INFO`, `WARNING`, `ERROR` printers using `std::format` style.
* `ERROR` is available as a macro wrapper that prints source info.
* `DEBUG` macros:
  * `logg$debug(fmt, ...)` prints debug messages when `DEBUG` is defined.
  * `logg$variable(x)` prints variable name and value when `DEBUG` is defined.
  * Both are no-ops when `DEBUG` is not defined.
* Thread id shown in hex for quick correlation in multithreaded applications.

## Setup

1. Place `logg.hpp` in your include path or add it as module to [build](https://github.com/lurkydismal/build) system.

## Usage

```cpp
#include "logg.hpp"

int main() {
    logg::info( "Starting application version {}", 1 );
    logg::warning( "Low memory: {} MB", 42 );
    logg$error( "Fatal error: {}", "out-of-memory" );

#if defined(DEBUG)

    int value = 7;

    logg$variable( value );                // prints `value = '7'`
    logg$debug( "Step {} OK", 1 );         // debug message with source location

#endif

    return ( 0 );
}
```

## Project Status

Project is: _in progress_.

## Room for Improvement

* Make no-op macros use provided arguments to avoid *unused* warnings.

## License

This project is open source and available under the
[GNU Affero General Public License v3.0](LICENSE).
