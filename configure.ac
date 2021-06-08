AC_INIT([productviewer], [0.0], [pt@darapsa.co.id])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_TYPE_SIZE_T
AC_CHECK_FUNCS([strrchr memset])
AM_PROG_AR
LT_INIT
AC_CANONICAL_HOST
case $host_vendor in
	*apple* ) apple=true;;
	*) apple=false;;
esac
AM_CONDITIONAL([APPLE], [test x$apple = xtrue])
case $host_cpu in
	*wasm* ) wasm=true;;
	*) wasm=false;;
esac
AM_CONDITIONAL([WASM], [test x$wasm = xtrue])
case $host in
	*arm*apple-darwin1* ) ios=true;;
	*) ios=false;;
esac
AM_CONDITIONAL([IOS], [test x$ios = xtrue])
AC_OUTPUT
