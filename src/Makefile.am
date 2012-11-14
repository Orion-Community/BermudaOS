SUBDIRS=core
ACLOCAL_AMFLAGS=-I m4

lib_LTLIBRARIES=libnet.la
libnet_la_SOURCES=tokenbucket.c
libnet_la_LIBADD=core/libnetcore.la

CLEANFILES=*.lst
include ../Makefile.flags