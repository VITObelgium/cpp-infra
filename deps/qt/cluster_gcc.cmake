message(STATUS "Patching QT for static runtime linking in: " ${QT_SRC_ROOT})

file(READ ${QT_SRC_ROOT}/qt/qtbase/mkspecs/common/g++-base.conf conf_file)
string(REPLACE "QMAKE_CC                = gcc" "QMAKE_CC                = x86_64-unknown-linux-gnu-gcc" conf_file "${conf_file}")
string(REPLACE "QMAKE_CXX               = g++" "QMAKE_CXX               = x86_64-unknown-linux-gnu-g++" conf_file "${conf_file}")
file(WRITE ${QT_SRC_ROOT}/qt/qtbase/mkspecs/common/g++-base.conf ${conf_file})
