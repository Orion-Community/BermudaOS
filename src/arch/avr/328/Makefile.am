include ../../../../Makefile.flags
OPT_SCRS=

SUBDIRS=dev
libexec_PROGRAMS=boot.elf
nodist_boot_elf_SOURCES=boot.c
dist_boot_elf_SOURCES=start.S
noinst_LTLIBRARIES = lib328.la
lib328_la_SOURCES=328-io.c 328-timer.c $(OPT_SCRS)
lib328_la_LIBADD=dev/libdev.la

boot.elf:
	$(CC) $(CCFLAGS) -c -o boot.elf $(srcdir)/start.S
