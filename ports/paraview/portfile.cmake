include(vcpkg_common_functions)
set(VCPKG_BUILD_TYPE release)

vcpkg_from_git(
    URL https://gitlab.kitware.com/paraview/paraview.git
    OUT_SOURCE_PATH SOURCE_PATH
    RECURSE_SUBMODULES
    REF v5.5.2
    HEAD_REF master
    PATCHES
        ${CMAKE_CURRENT_LIST_DIR}/no-opengl-probetool.patch
        ${CMAKE_CURRENT_LIST_DIR}/gl2ps-linker-error.patch
)

vcpkg_replace_string(${SOURCE_PATH}/VTK/ThirdParty/hdf5/module.cmake "\${HDF5_LIBRARIES}" "${CURRENT_INSTALLED_DIR}/lib/libhdf5.a")
vcpkg_replace_string(${SOURCE_PATH}/VTK/ThirdParty/hdf5/module.cmake "\${HDF5_HL_LIBRARIES}" "${CURRENT_INSTALLED_DIR}/lib/libhdf5_hl.a")

TEST_FEATURE("mpi" WITH_MPI)
TEST_FEATURE("osmesa" WITH_OSMESA)

if (WITH_OSMESA)
    set (ADDITIONAL_OPTIONS
        -DVTK_USE_X=OFF
        -DVTK_USE_TK=OFF
        -DVTK_RENDERING_BACKEND=None
        -DVTK_RENDERING_BACKEND_DEFAULT=None
        -DVTK_DEFAULT_RENDER_WINDOW_OFFSCREEN=ON
        -DVTK_OPENGL_HAS_OSMESA=ON
        -DVTK_DEFAULT_RENDER_WINDOW_HEADLESS=ON
        -DVTK_Group_ParaViewRendering=OFF
        -DOPENGL_INCLUDE_DIR=IGNORE
        -DOPENGL_xmesa_INCLUDE_DIR=IGNORE
        -DOPENGL_opengl_LIBRARY=/usr/lib/x86_64-linux-gnu/libGL.so
        -DOPENGL_gl_LIBRARY=/usr/lib/x86_64-linux-gnu/libGL.so
        -DOSMESA_INCLUDE_DIR=${CURRENT_INSTALLED_DIR}/include
        -DOSMESA_LIBRARY=${CURRENT_INSTALLED_DIR}/lib/libOSMesa.so
    )

    set(VCPKG_LINKER_FLAGS "-lGL -L/usr/lib/x86_64-linux-gnu")
endif ()

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DBUILD_SHARED_LIBS=OFF
        -DPARAVIEW_BUILD_QT_GUI=OFF
        -DPARAVIEW_USE_MPI=${WITH_MPI}
        -DPARAVIEW_USE_VTKM=OFF
        -DPARAVIEW_ENABLE_XDMF2=OFF
        -DPARAVIEW_ENABLE_COMMANDLINE_TOOLS=ON
        -DVTK_Group_MPI=${WITH_MPI}
        -DVTK_Group_Imaging=ON
        -DVTK_Group_Views=ON
        -DVTK_USE_SYSTEM_EXPAT=ON
        -DVTK_USE_SYSTEM_FREETYPE=ON
        -DVTK_USE_SYSTEM_JPEG=ON
        -DVTK_USE_SYSTEM_HDF5=ON
        -DVTK_MODULE_vtkhdf5_IS_SHARED=OFF
        -DVTK_USE_SYSTEM_JSONCPP=ON
        -DVTK_USE_SYSTEM_LIBPROJ4=ON
        -DVTK_USE_SYSTEM_LIBXML2=ON
        -DVTK_USE_SYSTEM_LZ4=ON
        -DVTK_USE_SYSTEM_OGGTHEORA=ON
        -DVTK_USE_SYSTEM_PNG=ON
        -DVTK_USE_SYSTEM_NETCDF=ON
        -DVTK_USE_SYSTEM_TIFF=ON
        -DVTK_USE_SYSTEM_ZLIB=ON
        -DVTK_USE_SYSTEM_LZMA=ON
        -DVTK_MODULE_vtklzma_IS_SHARED=OFF
        -DVTK_FORBID_DOWNLOADS=ON
        ${ADDITIONAL_OPTIONS}
)

vcpkg_install_cmake()
vcpkg_fixup_cmake_targets()

file(RENAME ${CURRENT_PACKAGES_DIR}/bin ${CURRENT_PACKAGES_DIR}/tools)

file(INSTALL ${SOURCE_PATH}/Copyright.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
