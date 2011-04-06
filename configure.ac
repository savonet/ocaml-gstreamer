
# check for one particular file of the sources 
AC_INIT(ocaml-bjack, 0.1.3, savonet-users@lists.sourceforge.net)

VERSION=$PACKAGE_VERSION
AC_MSG_RESULT([configuring $PACKAGE_STRING])

# Add prefix to compilation variables
# if passed
if test "x$prefix" != "xNONE"; then
	CFLAGS="$CFLAGS -I$prefix/include"
	LDFLAGS="$LDFLAGS -L$prefix/lib"
	CPPFLAGS="$CPPFLAGS -I$prefix/include"
fi

AC_CHECK_OCAML_COMPILERS()

AC_PROG_CC()
AC_CHECK_TOOL([AR],[ar],no)
PKG_PROG_PKG_CONFIG()
#PKG_CHECK_MODULES(jack, jack, , [AC_MSG_ERROR(jack not found.)])

# PKG_CHECK_MODULES loses when you need --libs-only-[lL]
if ! $PKG_CONFIG --exists jack; then
        AC_MSG_ERROR([jack not found])
fi
jack_CFLAGS=`$PKG_CONFIG --cflags jack`
AC_SUBST([jack_CFLAGS])
jack_LIBS=`$PKG_CONFIG --libs-only-l jack`
AC_SUBST([jack_LIBS])
jack_LDFLAGS=`$PKG_CONFIG --libs-only-L jack`
AC_SUBST([jack_LDFLAGS])

# Check for samplerate
PKG_CHECK_MODULES(samplerate, samplerate >= 0.0.15)

LIBS="$LIBS -lsamplerate"

# substitutions to perform
AC_SUBST(VERSION)
AC_SUBST(LIBS)

# Finally create the Makefile and samples
AC_CONFIG_FILES([Makefile],[chmod a-w Makefile])
AC_CONFIG_FILES([src/META])
AC_CONFIG_FILES([src/Makefile])
AC_OUTPUT
