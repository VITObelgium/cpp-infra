# C++ infrastructure library
This library contains the things we use in almost every C++ software project.
Think of:
- logging
- string operations
- datetime operations
- dictionary types (Point, Coordinate, Cell, Color, Size, Line, ...)
- color mapping
- csv parsing
- xml parsing
- process launching
- database access
- gdal usage

This library tries to avoid reinventing the wheel over and over again when creating C++ projects. Most classes are wrappers around third party libraries (avoid reinventing the wheel remember) but guard us against third party API breakage and allows us to create the most convenient interface for our needs.

## Building
#### Requirements
- A C++17 compiler
- fmt library (https://github.com/fmtlib/fmt)
- date library (https://github.com/HowardHinnant/date)

The library is built using CMake:
```
cmake -G Ninja "/path/to/cpp-infra"
```

#### Build options
The following CMake options are available for toggling functionality:
- `INFRA_LOGGING` Enable logging functionality (requires spdlog)
- `INFRA_GDAL` Enable gdal wrapper (gdal without memory leaks) (required gdal)
- `INFRA_EMBED_GDAL_DATA` Embed the required gdal data files in the binary for easy deployment (avoid GDAL_HOME issues)
- `INFRA_XML` Enable xml reader (requires pugixml)
- `INFRA_NUMERIC` Enable the numeric utility classes
- `INFRA_CHARSET` Enable the character set utility classes (character set detection) (requires ICU)
- `INFRA_PROCESS` Enable the process utility classes (launching processes) (requires reproc)
- `INFRA_DATABASE` Build the database utility classes  (query execution) (requires sqlpp11)
- `INFRA_DATABASE_SQLITE` Sqlite support for the database utility classes (requires sqlpp11-connector-sqlite3)
- `INFRA_DATABASE_POSTGRES` Postgres support for the database utility classes (requires sqlpp11-connector-postgresql)
- `INFRA_ENABLE_TESTS` Build the unit tests (requires doctest)
- `INFRA_ENABLE_TEST_UTILS` Build the unit test utilities
- `INFRA_ENABLE_BENCHMARKS` Build the micro benchmarks
- `INFRA_ENABLE_DOCUMENTATION` Build the documentation

## Using infra in your project
directly add the source directory in your cmake project
```
# first set the enabled components in the cache
set(INFRA_LOGGING ON CACHE BOOL "" FORCE)
set(INFRA_GDAL ON CACHE BOOL "" FORCE)
add_subdirectory(infra)
```

or use the installed cmake module
```
find_package(Infra CONFIG REQUIRED)
target_link_libraries(mytarget PRIVATE Infra::infra)
```
