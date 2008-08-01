dnl Process this file with autoconf to produce a configure script.

AC_INIT(MATLABFind, 1.0.2)
AC_CONFIG_SRCDIR(src)
AM_INIT_AUTOMAKE(foreign)
AM_CONFIG_HEADER(config.h)

AC_PROG_CC_C99

pkg_modules="gtk+-2.0 >= 2.8.0 \
	libglade-2.0 >= 2.5.1"

PKG_CHECK_MODULES(GLADE, [$pkg_modules])
PKG_CHECK_MODULES(OPENDIAMOND, [opendiamond])

AC_SUBST(AM_CPPFLAGS, "-Werror-implicit-function-declaration")

# XXX should work on more than 1 arch
AC_SUBST(AM_CFLAGS, "-m32")


AC_CONFIG_FILES([
Makefile
src/Makefile
src/matlab.sf_conf
data/Makefile
data/glade/Makefile
data/filter/Makefile
])

AC_OUTPUT
