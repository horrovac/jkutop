AM_CFLAGS = -Wall
bin_PROGRAMS = jkutop
jkutop_SOURCES = jkutop.c readproc.c printing.c
man9_MANS = jkutop.man jkups.man

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	[ -e jkups ] || $(LN_S) jkutop jkups
