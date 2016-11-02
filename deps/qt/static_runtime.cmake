message(STATUS "Patching QT for static runtime linking in: " ${QT_SRC_ROOT})

file(READ ${QT_SRC_ROOT}/qt/qtbase/mkspecs/common/msvc-desktop.conf conf_file)
string(REPLACE "-MD" "-MT" conf_file "${conf_file}")
file(WRITE ${QT_SRC_ROOT}/qt/qtbase/mkspecs/common/msvc-desktop.conf ${conf_file})
