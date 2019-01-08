if (MSVC AND PYTHON2 AND NOT "${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
    # we are building for 32bit python 2, make sure the correct interpreter is used
    set (PYTHON_EXECUTABLE c:/Python27_32bit/python.exe)
else ()
    find_package(PythonInterp REQUIRED)
endif ()

add_test(NAME pygdxtest
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/testgdx.py
)

if (MSVC)
    set_tests_properties(pygdxtest PROPERTIES ENVIRONMENT PYTHONPATH=${LIBRARY_OUTPUT_PATH}/$<CONFIG>)
else ()
    set_tests_properties(pygdxtest PROPERTIES ENVIRONMENT PYTHONPATH=${LIBRARY_OUTPUT_PATH})
endif ()

if (OpenCL_FOUND)
    add_test(NAME pygdxgputest
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/testgdxgpu.py
    )

    if (MSVC)
        set_tests_properties(pygdxgputest PROPERTIES ENVIRONMENT PYTHONPATH=${LIBRARY_OUTPUT_PATH}/$<CONFIG>)
    else ()
        set_tests_properties(pygdxgputest PROPERTIES ENVIRONMENT PYTHONPATH=${LIBRARY_OUTPUT_PATH})
    endif ()
endif ()