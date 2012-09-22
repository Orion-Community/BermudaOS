OPT_SCRS=
if THREADS
MAYBE_EVENTS=events
OPT_SCRS+= sched.c thread.c
endif

SUBDIRS=$(MAYBE_EVENTS)
lib_LTLIBRARIES=libsys.la
libsys_la_SOURCES=mem.c virt_timer.c $(OPT_SCRS)
libsys_la_LIBADD=events/libEvents.la
include ../../Makefile.flags
