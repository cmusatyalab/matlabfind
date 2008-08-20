## Process this file with automake to produce Makefile.in

gladedir = $(datadir)/matlabfind/glade
filterdir = $(datadir)/matlabfind/filter

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DMATLABFIND_GLADEDIR=\""$(gladedir)"\" \
	-DMATLABFIND_FILTERDIR=\""$(filterdir)"\" \
	@OPENDIAMOND_CFLAGS@ @GLADE_CFLAGS@

bin_PROGRAMS = matlabfind

matlabfind_SOURCES = \
	main.c matlabfind.h \
	define.c define.h \
	investigate.c investigate.h \
	util.c util.h \
	quick_tar.c quick_tar.h \
	diamond_interface.c diamond_interface.h

matlabfind_LDADD = @OPENDIAMOND_LIBS@ @GLADE_LIBS@

matlabfind_LDFLAGS = -export-dynamic

pkglib_LTLIBRARIES = libmatlab_search.la 
pkglib_LIBRARIES = matlab.sf_conf

libmatlab_search_la_SOURCES = matlab_search.cc

libmatlab_search_la_CPPFLAGS = -I/opt/snapfind/include

EXTRA_DIST = fil_matlab_exec.c
