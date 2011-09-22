
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
PKG_CONFIG_CHECK_MODULE([gstreamer-0.10])
PKG_CONFIG_CHECK_MODULE([gstreamer-app-0.10])

# This module requires camlidl.
requires=camlidl

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
