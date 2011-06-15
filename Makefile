INSTALL := install
CFLAGS := -fPIC -O2 -g -m32 -Wall -Wextra
SNAPFIND_LIBDIR=/opt/snapfind/lib

MATLAB_ROOT_DIR=/opt/matlab
MATLAB_LIBDIR=${MATLAB_ROOT_DIR}/bin/glnx86
MATLAB_EXE_PATH=${MATLAB_ROOT_DIR}/bin/matlab

all: filter-code/fil_matlab_exec

# filter code
filter-code/fil_matlab_exec: filter-code/fil_matlab_exec.c filter-code/matlab-compat.h
	export PKG_CONFIG_PATH=/opt/diamond-filter-kit/lib/pkgconfig:$$PKG_CONFIG_PATH; gcc $(CFLAGS) $$(pkg-config opendiamond glib-2.0 --cflags --libs) -ldl -o $@ filter-code/fil_matlab_exec.c -DMATLAB_EXE_PATH=\"${MATLAB_EXE_PATH}\" -Wl,-rpath=${MATLAB_LIBDIR} -Wl,-Bstatic -I/opt/diamond-filter-kit/include -L/opt/diamond-filter-kit/lib $$(pkg-config libarchive --cflags --libs --static) -Wl,-Bdynamic


# clean
clean:
	$(RM) -r filter-code/fil_matlab_exec

install: all
	$(INSTALL) filter-code/fil_matlab_exec $(SNAPFIND_LIBDIR)


.DUMMY: all clean install
