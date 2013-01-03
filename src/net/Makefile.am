SUBDIRS=core

bermudaosdir=@libdir@/bermudaos
bermudaos_LTLIBRARIES=libnet.la
libnet_la_SOURCES=tokenbucket.c
libnet_la_LIBADD=core/libnetcore.la

CLEANFILES=*.lst
LOCAL_LIB=../../include
include ../../Makefile.flags
