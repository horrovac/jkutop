AC_PREREQ(2.5)
AC_INIT([jkutop], [0.9], [faruk.kujundzic@jku.at])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC

AC_CHECK_LIB( [ncurses], [initscr], , [AC_MSG_FAILURE([Missing required library ncurses.])] )
AC_SUBST([EXTRA_MODULES])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
