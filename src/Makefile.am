## Process this file with automake to produce Makefile.in

gladedir = $(datadir)/matlabfind/glade
filterdir = $(datadir)/matlabfind/filter

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DMATLABFIND_GLADEDIR=\""$(gladedir)"\" \
	-DMATLABFIND_FILTERDIR=\""$(filterdir)"\" \
	@glade_CFLAGS@

bin_PROGRAMS = matlabfind

matlabfind_SOURCES = \
	main.c matlabfind.h \
	define.c define.h \
	investigate.cpp investigate.h \
	util.c util.h \
	quick_tar.c quick_tar.h \
	diamond_interface.c diamond_interface.h

matlabfind_LDADD = @glade_LIBS@

matlabfind_LDFLAGS = -export-dynamic
