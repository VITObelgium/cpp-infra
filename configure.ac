# $Id: configure.ac 2601 2015-09-23 15:11:23Z oconrad $
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(opaq, 0.5, BUG-REPORT-ADDRESS)
AC_CONFIG_SRCDIR([src/opaq_core/opaq.cpp])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign tar-ustar subdir-objects])

#start with empty compiler flags
${CFLAGS=""}
${CXXFLAGS=""}

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# Checks for libraries.
#AC_CHECK_LIB([fftw3],[fftw_execute],FFTWFOUND=1,)
AC_CHECK_LIB([gomp], [omp_get_num_threads], GOMPFOUND=1,,)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([float.h malloc.h memory.h stdlib.h string.h unistd.h])
AC_CHECK_HEADERS([wchar.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_STRUCT_TM

dnl Checks for platform characteristics.
AC_C_BIGENDIAN

dnl Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([floor memmove memset modf pow sqrt strchr strdup strerror strrchr strstr strtol])

dnl ==============================================
dnl Set path to Eigen and check the header files
dnl EIgen3 defaults to /usr/include/eigen3 on ubuntu
dnl the user can override with --with-Eigen=/path...
dnl ==============================================
EIGEN_PATH=/usr/include/eigen3
AC_ARG_WITH(Eigen,[  --with-Eigen[=dir]        add Eigen installation path (default /usr/include/eigen3)],,)
if test "$with_Eigen" != "no" -a "$with_Eigen" != "" ; then
  if test "$with_Eigen" = "yes" ; then
    EIGEN_PATH=/usr/include/eigen3
  else
    EIGEN_PATH=$with_Eigen
  fi

  AC_SUBST(EIGEN_PATH,$EIGEN_PATH)
fi

dnl Check for the Eigen header files
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS(${EIGEN_PATH}/Eigen/Dense,
     [AC_MSG_RESULT(Eigen found in ${EIGEN_PATH}...)],
     [AC_MSG_ERROR(Eigen not found, please set with --with-Eigen...)])
AC_LANG_POP([C++])


dnl ===============================================
dnl checking for LOG4CXX library
dnl
dnl TODO clean this up !
dnl ===============================================
LOG4CXX_PATH="/usr"
LOG4CXX_INCLUDE="/usr/include"
LOG4CXX_LIB_PATH="/usr/lib"
AC_ARG_WITH(log4cxx,[  --with-log4cxx[=dir]        add Log4cxx installation path (default /usr)],,)
if test "$with_log4cxx" != "no" -a "$with_log4cxx" != "" ; then
  if test "$with_log4cxx" = "yes" ; then
    LOG4CXX_PATH=/usr/include/eigen3
  else
    # we have the full path
    EIGEN_PATH=$with_Eigen
  fi

  AC_SUBST(EIGEN_PATH,$EIGEN_PATH)
fi

AC_CHECK_LIB(log4cxx, main, [LOG4CXX="-L${LOG4CXX_LIB_PATH} -llog4cxx"],,["-L${LOG4CXX_LIB_PATH}"])
if test -z "${LOG4CXX}"; then
      AC_ERROR("... log4cxx not found!")
fi

AC_SUBST(LOG4CXX)
AC_SUBST(LOG4CXX_INCLUDE)

dnl check for HDF5 support
dnl See : http://www.gnu.org/software/autoconf-archive/ax_lib_hdf5.html

AX_LIB_HDF5()

# debug
AC_ARG_ENABLE(debug,
     [  --enable-debug	  Turn on debugging],
     [case "${enableval}" in
        yes) debug=true ;;
        no)  debug=false ;;
        *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
      esac],[debug=false])
      AM_CONDITIONAL(DEBUG, test x$debug = xtrue)
 
# openmp
AC_ARG_ENABLE(openmp,
     [  --disable-openmp	  Build without OpenMP support],
     [case "${enableval}" in
        yes) openmp=true ;;
        no)  openmp=false ;;
        *) AC_MSG_ERROR(bad value ${enableval} for --disable-openmp) ;;
      esac],[openmp=true])
      AM_CONDITIONAL(OPENMP, test x$openmp = xtrue)
  
  if test $openmp = "false" ; then
    AC_MSG_RESULT([        OpenMP support disabled]) 
      GOMPFLAGS=""
  else
    if test -n "$GOMPFOUND" ; then 
      AC_MSG_RESULT([        Building with OpenMP support using libgomp])
      GOMPFLAGS="-fopenmp -lgomp"
    else 
      AC_MSG_RESULT([        libgomp not found, OpenMP support disabled]) 
      GOMPFLAGS=""
    fi
  fi

AC_SUBST(GOMPFLAGS)


# write the makefiles
AC_OUTPUT(Makefile \
	src/Makefile \
	src/depend/Makefile\
	src/depend/TinyXML-2.6.2/Makefile\
	src/depend/gzstream-1.5/Makefile\
	src/opaq_core/Makefile\
	src/plugins/Makefile\
	src/plugins/data/Makefile\
	src/plugins/data/src/Makefile
	src/plugins/forecast/Makefile\
	src/plugins/forecast/src/Makefile\
	src/plugins/forecast/tools/Makefile\
	src/plugins/forecast/tools/ffsim/Makefile\
	run/Makefile
)

