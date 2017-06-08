INSTALL := install
CFLAGS := -fPIC -O2 -g -Wall -Wextra
FILTER_DIR=/usr/local/share/diamond/filters
BINDIR=/usr/local/bin

MATLAB_ROOT_DIR=/opt/matlab
MATLAB_LIBDIR=${MATLAB_ROOT_DIR}/bin/glnx86
MATLAB_EXE_PATH=${MATLAB_ROOT_DIR}/bin/matlab

all: filter-code/fil_matlab_exec

# filter code
filter-code/fil_matlab_exec: filter-code/fil_matlab_exec.c filter-code/matlab-compat.h
	gcc $(CFLAGS) -o $@ filter-code/fil_matlab_exec.c -DMATLAB_EXE_PATH=\"${MATLAB_EXE_PATH}\" -Wl,-rpath=${MATLAB_LIBDIR} $$(pkg-config opendiamond glib-2.0 libarchive --cflags --libs) -ldl


# clean
clean:
	$(RM) -r filter-code/fil_matlab_exec

install: all
	$(INSTALL) filter-code/fil_matlab_exec $(FILTER_DIR)
	$(INSTALL) diamond-bundle-matlab $(BINDIR)


.DUMMY: all clean install
