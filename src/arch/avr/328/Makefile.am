include ../../../../Makefile.flags
OPT_SCRS=

SUBDIRS=dev
libexec_PROGRAMS=boot.o
noinst_LTLIBRARIES = lib328.la
lib328_la_SOURCES=328-io.c 328-timer.c $(OPT_SCRS)
lib328_la_LIBADD=dev/libdev.la

boot.o:
	$(CC) $(CCFLAGS) -c -o boot.o $(srcdir)/start.S
