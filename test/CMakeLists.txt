if (MSVC AND PYTHON2 AND NOT "${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
    set (PYTHON_EXECUTABLE c:/Python27_32bit/python.exe)
else ()
    find_package(PythonInterp REQUIRED)
endif ()

macro(add_integration_test _NAME _TEST_FILE)
    add_test(NAME ${_NAME}
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${_TEST_FILE}
    )

    if (MSVC)
        set (ENV_VARS
            GDAL_DATA=${CMAKE_PREFIX_PATH}/data
            PYTHONPATH=${LIBRARY_OUTPUT_PATH}/$<CONFIG>
        )
    else ()
        set (ENV_VARS
            GDAL_DATA=${CMAKE_PREFIX_PATH}/share/gdal
            PYTHONPATH=${LIBRARY_OUTPUT_PATH}
        )
    endif ()

    set_tests_properties(${_NAME} PROPERTIES ENVIRONMENT "${ENV_VARS}")
endmacro()

add_integration_test(gdxintegrationtest testscenarios.py)
add_integration_test(clustertest testclusters.py)
add_integration_test(distancetest testdistance.py)
add_integration_test(reclasstest testreclass.py)
add_integration_test(rasterizetest testrasterize.py)
add_integration_test(majoritytest testmajority.py)
