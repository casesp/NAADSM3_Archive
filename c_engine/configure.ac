dnl Process this file with autoconf to produce a configure script.
AC_INIT(naadsm, 3.2.19b)
AC_CONFIG_SRCDIR(src/main.c)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(config.h)

dnl Adds command-line options.

 AC_DEFINE(DEBUG,0,[Define to 1 to provide an option for debug output.])

AC_ARG_ENABLE([debug],
  AC_HELP_STRING([--enable-debug], [provide an option for debug output]),
  [ac_cv_enable_debug=${enable_debug}],
  AC_CACHE_CHECK([whether enable-debug was specified], ac_cv_enable_debug,
    ac_cv_enable_debug=yes)
) # end of AC_ARG_ENABLE debug
case "X${ac_cv_enable_debug}" in
  Xyes|X )
    AC_DEFINE(DEBUG,1,[Define to 1 to provide an option for debug output.]) ;;
  * )
    ;;
  esac

ac_cv_enable_silent_mode=no

AC_ARG_ENABLE([silent_mode],
  AC_HELP_STRING([--enable-silent-mode], [provide for disabling the normal standard output result data]),
  [ac_cv_enable_silent_mode=${enable_silent_mode}],
  AC_CACHE_CHECK([whether enable-silent-mode was specified], ac_cv_enable_silent_mode,
    ac_cv_enable_silent_mode=yes)
) # end of AC_ARG_ENABLE silent_mode  
case "X${ac_cv_enable_silent_mode}" in
  Xyes|X )
{
    AC_DEFINE(SILENT_MODE,1,[Define to 1 to remove standard output result data, other than SQL statements.])} ;;
  * )
    ;;
  esac


ac_cv_enable_sc_guilib=no

AC_ARG_ENABLE([sc_guilib],
  AC_HELP_STRING([--enable-sc-guilib], [provide for MySQL database SQL output statements]),
  [ac_cv_enable_sc_guilib=${enable_sc_guilib}],
  AC_CACHE_CHECK([whether enable-sc-guilib was specified], ac_cv_enable_sc_guilib,
    ac_cv_enable_sc_guilib=yes)
) # end of AC_ARG_ENABLE sc_guilib  
case "X${ac_cv_enable_sc_guilib}" in
  Xyes|X )
{
    AC_DEFINE(USE_SC_GUILIB,1,[Define to 1 to provide an option for SQL output.])
    AC_DEFINE(SILENT_MODE,1,[Define to 1 to remove standard output result data, other than SQL statements.])} ;;
  * )
    ;;
  esac

AM_CONDITIONAL(USE_SC_GUILIB, test "X${ac_cv_enable_sc_guilib}" != Xno)

dnl Checks for programs.
AC_PROG_CC
AC_C_CONST

AM_PROG_LEX
AC_PROG_YACC
case "X${YACC}" in
  Xyacc )
  {
    AC_DEFINE( USE_PLAIN_YACC, 1, [Set this to 1 if no byacc or bison are available])
  };;
  esac

dnl Checks for libraries.

PKG_CHECK_MODULES(GLIB,glib >= 2.6,,try_glib_20=yes)
if test "X${try_glib_20}" = Xyes
then
  PKG_CHECK_MODULES(GLIB,glib-2.0 >= 2.6,,AC_MSG_ERROR(cannot continue without glib))
fi
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

AM_PATH_GSL(1.4,[have_gsl=true],AC_MSG_ERROR(cannot continue without GSL))
AC_SUBST(GSL_CFLAGS)
AC_SUBST(GSL_LIBS)

AM_PATH_SPRNG(,AC_MSG_ERROR(cannot continue without SPRNG))
AC_SUBST(SPRNG_CFLAGS)
AC_SUBST(SPRNG_LIBS)

AC_CHECK_LIB(expat,XML_Parse,,AC_MSG_ERROR(cannot continue without expat))

AC_CHECK_LIB(m,pow,,AC_MSG_ERROR(cannot continue without math library))

AC_CHECK_LIB(rtree,RTreeNewIndex,,AC_MSG_ERROR(cannot continue without R-tree library))

AC_CHECK_LIB(shp,SHPCreate,,AC_MSG_ERROR(cannot continue without Shapefile library))

AC_CHECK_LIB(gpcl,gpc_polygon_clip,,AC_MSG_ERROR(cannot continue without General Polygon Clipping (GPC) library))

AC_CHECK_LIB(gd,gdImageCreate,,AC_MSG_ERROR(cannot continue without GD graphics drawing library))

AC_CHECK_LIB(proj,pj_init,,AC_MSG_ERROR(cannot continue without PROJ.4 Cartographic Projections library))

AM_PATH_MPI()
AC_SUBST(MPI_CFLAGS)
AC_SUBST(MPI_LIBS)
AM_CONDITIONAL(HAVE_MPI, test "X${ac_cv_with_mpi}" != Xno)

case "X${ac_cv_with_mpi}" in
  Xyes|X )
    AC_DEFINE(USE_MPI,1,[Define to 1 to enable MPI properly in header files included from the operating system.]) ;;
  * )
    ;;
  esac

PKG_CHECK_MODULES(SCEW,scew >= 0.3.1,,AC_MSG_ERROR(cannot continue without SCEW))
AC_SUBST(SCEW_CFLAGS)
AC_SUBST(SCEW_LIBS)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h math.h unistd.h limits.h regex.h ctype.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_CHECK_FUNCS(getstr getdelim getline strtoi)

AC_OUTPUT(Makefile \
  wml/Makefile \
  wml/test/Makefile \
  herd/Makefile \
  herd/test/Makefile \
  prob_dist/Makefile \
  prob_dist/test/Makefile \
  rel_chart/Makefile \
  rel_chart/test/Makefile \
  gis/Makefile \
  gis/test/Makefile \
  reporting/Makefile \
  reporting/test/Makefile \
  src/Makefile \
  models/Makefile \
  models/test/Makefile \
  models/test/model.disease/Makefile \
  models/test/model.direct-contact-spread/Makefile \
  models/test/model.indirect-contact-spread/Makefile \
  models/test/model.airborne-spread/Makefile \
  models/test/model.detection/Makefile \
  models/test/model.control-measures/Makefile \
  models/test/model.ring-destruction/Makefile \
  models/test/model.ring-vaccination/Makefile \
  models/test/model.zones/Makefile \
  models/test/model.economic/Makefile \
  smherds2xml/Makefile \
  sm3herds2xml/Makefile \
  xml2sm3herds/Makefile \
  filters/Makefile \
  dll/Makefile \
  doc/Makefile \
  doc/diagrams/Makefile)
