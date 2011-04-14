
# check for one particular file of the sources 
AC_INIT(ocaml-gstreamer, 0.1.0, savonet-users@lists.sourceforge.net)

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
if test "$CAMLIDL" = no ; then
        AC_MSG_ERROR(Cannot find camlidl.)
fi

AC_PROG_CC()
AC_CHECK_TOOL([AR],[ar],no)
AC_SUBST(AR)
PKG_PROG_PKG_CONFIG()
#PKG_CHECK_MODULES(gstreamer, gstreamer-0.10, , [AC_MSG_ERROR(gstreamer not found.)])

# PKG_CHECK_MODULES loses when you need --libs-only-[lL]
if ! $PKG_CONFIG --exists gstreamer-0.10; then
        AC_MSG_ERROR([gstreamer not found])
fi
gstreamer_CFLAGS=`$PKG_CONFIG --cflags gstreamer-0.10`
gstreamer_LIBS=`$PKG_CONFIG --libs-only-l gstreamer-0.10`
gstreamer_LDFLAGS=`$PKG_CONFIG --libs-only-L gstreamer-0.10`

if ! $PKG_CONFIG --exists gstreamer-app-0.10; then
        AC_MSG_ERROR([gstreamer-app not found])
fi
gstreamer_CFLAGS=`$PKG_CONFIG --cflags gstreamer-app-0.10`
gstreamer_LIBS=`$PKG_CONFIG --libs-only-l gstreamer-app-0.10`
gstreamer_LDFLAGS=`$PKG_CONFIG --libs-only-L gstreamer-app-0.10`

# substitutions to perform
AC_SUBST(VERSION)
AC_SUBST(INC)
AC_SUBST(requires)
AC_SUBST([gstreamer_CFLAGS])
AC_SUBST([gstreamer_LIBS])
AC_SUBST([gstreamer_LDFLAGS])

# Finally create the Makefile and samples
AC_CONFIG_FILES([Makefile],[chmod a-w Makefile])
AC_CONFIG_FILES([src/META])
AC_CONFIG_FILES([src/Makefile])
AC_OUTPUT
