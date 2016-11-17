# OPAQ - Operational prediction of air quality

OPAQ is a runtime framework for operational prediction of air quality, both forecast as well as mapping.
The package uses the GNU autotools built system, when checking out, please first run the
autogen.sh script to add the build structure. After that just ./configure; make; make install.

## Build environment
#### Required components:
- a c++14 compiler
- cmake
- ninja-build is recommended

## Prerequisites
The third party library dependencies can be compiled by calling the bootstrap script.

### On linux
call bootstrap.sh and select the desired configuration and toolchain. Use the default toolchain if you
want to do a normal compilation.

### On windows
call bootstrap.bat, it will build both debug and release versions of the prerequisites

This will build all third party libraries required by OPAQ
The libraries will be located in build/local

#### Third party libraries
- Spdlog logging facilities
- Eigen3 matrix library
- HDF5 hdf5 utilities
- Spdlog logging library
- Fmt string formatting library
- Sqlite database library
- GoogleTest unit testing library
- boost infrastructure library
- TinyXml xml library
- date header for easy date manipulation
- GoogleBenchmark: micro benchmark library
- sqllp11: type safe sql library
- zlib compression library

## Compilation & debugging
OPAQ uses cmake which should make the compilation process fairly straight forward.
Use the build scripts to perform the cmake configuration and initial build.

### On linux
call build.sh and select the desired configuration
build/opaq_[config] will be the build directory, use it for incremental builds after you make changes.

### On windows
call build.bat, it will generate a visual studio solution and perform an initial build
build/opaq will contain the visual studio solution, use it for development

Make sure to call the bootstrap script first.

## Contact
Bino Maiheu, (c) VITO 2013-2016
bino.maiheu@vito.be






