AERA
====

These are instructions to build and run AERA.

Prerequisites
=============

* Required: On Windows, a Visual Studio installation with C++ build tools
* Required: CMake
* Required: Git
* Required: The AERA code repository from https://github.com/IIIM-IS/AERA

Following are the detailed steps for each platform to install the prerequisites.

## Windows
Install either Visual Studio Community 2022 or Visual Studio Build Tools 2022 from
https://visualstudio.microsoft.com/downloads/ .

In the installer, enable the "Desktop development with C++" workload.

Install CMake from https://cmake.org/download/ and make sure `cmake` is available in `PATH`.

Optional: install `ninja` and add it to `PATH`. The scripted build will use it automatically when available.

To install Git, download and install GitHub for Desktop from https://desktop.github.com .

To get the AERA code repository, launch GitHub for Desktop and sign in to GitHub. In the File menu, 
click "Clone a Repository". Click the URL tab and enter `https://github.com/IIIM-IS/AERA` . 
It should be a recursive clone (which is the default).

Build
=====
The Windows build is now CMake-first.

The easiest path is to run:

    powershell -ExecutionPolicy Bypass -File tests\smoke_hello_world.ps1

That script configures, builds and smoke-tests the project through CMake. It will:

* locate the local MSVC installation
* load the 32-bit compiler environment required by the current codebase
* use `Ninja` when available, otherwise `NMake Makefiles`
* run `ctest` after the build

## `WITH_DETAIL_OID`

To work with the AERA Visualizer, enable `WITH_DETAIL_OID` manually in [base.h](C:/Users/pille/Documents/GitHub/AERA/submodules/CoreLibrary/CoreLibrary/base.h) so that the line is:

    #define WITH_DETAIL_OID // Enable get_detail_oid() in every object.

## Compile

If you want to run CMake manually instead of using the script, load the MSVC environment first and configure a Release build.

Example with `NMake`:

    cmake -S . -B build\smoke -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DAERA_ENABLE_PROTOBUF=OFF
    cmake --build build\smoke
    ctest --test-dir build\smoke --output-on-failure

Run
===

After a successful build, the smoke test runs `AERA.exe` against a generated settings file. The output should contain:

    0s:50ms:0us: hello world 1

The executable is generated under `build\smoke\bin\AERA.exe`.

# settings.xml

The `settings.xml` file specifies the program to run, AERA meta parameters, and other options for running AERA.
The repository version lives at [settings.xml](C:/Users/pille/Documents/GitHub/AERA/AERA/settings.xml).

This has many parameters which are documented at the bottom of the file. Following are some highlights.

## settings.xml source_file_name

The `source_file_name` parameter specifies the seed program to run. For example, a program to use with the AERA Visualizer
is "../AERA/replicode_v1.2/ball.external.replicode".

## settings.xml keep_invalidated_objects

To work with the AERA Visualizer, in the Debug section under Objects, set `keep_invalidated_objects` to "yes". (This is needed
so that the decompiled objects file contains all the objects that were created during the run, including temporary objects.)
