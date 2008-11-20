INSTALL := install
CFLAGS := -fPIC -O2 -g -m32 -Wall -Wextra -Iquick-tar
SNAPFIND_LIBDIR=/opt/snapfind/lib

MATLAB_ROOT_DIR=/afs/cs.cmu.edu/local/matlab/i386_fc5/7.6/lib/matlab7
MATLAB_LIBDIR=${MATLAB_ROOT_DIR}/bin/glnx86
MATLAB_INCLUDEDIR=${MATLAB_ROOT_DIR}/extern/include
MATLAB_EXE_PATH=${MATLAB_ROOT_DIR}/bin/matlab

all: filter-code/fil_matlab_exec.so snapfind-plugin/matlab_search.so

# quick tar
quick-tar/quick_tar.o: quick-tar/quick_tar.c quick-tar/quick_tar.h
	gcc $(CFLAGS) -o $@ -c quick-tar/quick_tar.c

# filter code
filter-code/fil_matlab_exec.so: filter-code/fil_matlab_exec.c quick-tar/quick_tar.o
	gcc $(CFLAGS) $$(pkg-config opendiamond glib-2.0 --cflags) -I${MATLAB_INCLUDEDIR} -L${MATLAB_LIBDIR} -shared  -o $@ filter-code/fil_matlab_exec.c -DMATLAB_EXE_PATH=\"${MATLAB_EXE_PATH}\" -leng -lmx -lut -Xlinker -rpath -Xlinker ${MATLAB_LIBDIR} quick-tar/quick_tar.o


# snapfind plugin
snapfind-plugin/matlab_search.so: snapfind-plugin/matlab_search.h snapfind-plugin/matlab_search.cc quick-tar/quick_tar.o
	g++ $(CFLAGS) -I/opt/snapfind/include -shared -o $@ snapfind-plugin/matlab_search.cc $$(pkg-config --cflags --libs gtk+-2.0 opendiamond) quick-tar/quick_tar.o


# clean
clean:
	$(RM) -r filter-code/fil_matlab_exec.so quick-tar/*.o snapfind-plugin/*.so

install: all snapfind-plugin/matlab.sf_conf
	$(INSTALL) filter-code/*.so snapfind-plugin/*.so $(SNAPFIND_LIBDIR)
	$(INSTALL) -m 644 snapfind-plugin/*.sf_conf $(SNAPFIND_LIBDIR)


.DUMMY: all clean install
