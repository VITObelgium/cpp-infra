option(INFRA_GDAL "Enable gdal wrapper for reading gis data" ON)
option(INFRA_LOGGING "Enable logging functionality" ON)
option(INFRA_EMBED_GDAL_DATA "Embed the required gdal data files in the binary" ON)
option(INFRA_LICENSE_MANAGER "Enable the license management functionality" OFF)

find_package(fmt CONFIG REQUIRED)

add_library(infra
    include/infra/algo.h
    include/infra/cell.h
    include/infra/generator.h
    #include/infra/line.h
    include/infra/point.h
    include/infra/exception.h
    include/infra/filesystem.h filesystem.cpp
    include/infra/color.h color.cpp
    include/infra/colormap.h colormap.cpp
    include/infra/geometadata.h geometadata.cpp
)

target_include_directories(infra
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_link_libraries(infra PRIVATE fmt::fmt)

if (CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(infra PRIVATE stdc++fs)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    find_library(CPP_EXP_LIB c++experimental HINTS /usr/local/opt/llvm/lib)
    target_link_libraries(infra PRIVATE ${CPP_EXP_LIB})
endif ()

if (INFRA_GDAL)
    find_package(Gdal REQUIRED)
    find_package(Gsl REQUIRED)

    target_sources(infra PRIVATE
        include/infra/gdal.h gdal.cpp
        include/infra/internal/gdalinternal.h
    )

    if (INFRA_EMBED_GDAL_DATA)
        target_sources(infra PRIVATE
            embedgdaldata.h
            embedgdaldata.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/gdal_data.h
            ${CMAKE_CURRENT_BINARY_DIR}/gdal_data.cpp
            embedgdaldata.cmake
        )

        target_compile_definitions(infra PRIVATE -DEMBED_GDAL_DATA)

        add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gdal_data.h ${CMAKE_CURRENT_BINARY_DIR}/gdal_data.cpp
                           COMMAND ${CMAKE_COMMAND} ARGS -DGDAL_DATA_PATH=${GDAL_DATA_PATH} -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/gdal_data -P ${CMAKE_CURRENT_SOURCE_DIR}/embedgdaldata.cmake
                           DEPENDS ${GDAL_DATA_PATH}/gcs.csv ${CMAKE_CURRENT_SOURCE_DIR}/embedgdaldata.cmake
                           COMMENT "Generate embedded gdal data"
        )
    endif ()

    target_link_libraries(infra PUBLIC Gdal::Gdal Gsl::Gsl)
endif ()

if (INFRA_LOGGING)
    find_package(spdlog CONFIG REQUIRED)

    target_sources(infra PRIVATE
        include/infra/log.h log.cpp
    )

    target_link_libraries(infra PRIVATE spdlog::spdlog)
endif ()

if (INFRA_LICENSE_MANAGER)
    find_package(Rlm CONFIG REQUIRED)

    target_sources(infra PRIVATE
        include/infra/licensemanager.h licensemanager.cpp
    )
endif ()

add_subdirectory(test)
