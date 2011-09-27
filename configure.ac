
# check for one particular file of the sources 
AC_INIT([ocaml-gstreamer],[0.1.0],[savonet-users@lists.sourceforge.net])

VERSION=$PACKAGE_VERSION
AC_MSG_RESULT([configuring $PACKAGE_STRING])

AC_BASE_CHECKS()

if test "$CAMLIDL" = no ; then
        AC_MSG_ERROR(Cannot find camlidl.)
fi

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
