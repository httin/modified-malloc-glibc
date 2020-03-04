 #!/bin/bash

GLIBC_VERSION=2.22
GLIBC_DIR=/usr/local/lib/glibc-test
GCC_DIR=/usr/lib/gcc/x86_64-linux-gnu/7/include

##############################################
### Run some software that need new glibc ###
##############################################
gcc -nostdinc -pthread -I${GCC_DIR} "$@" \
	-L${GLIBC_DIR}/lib -I${GLIBC_DIR}/include \
	-Wl,-rpath="${GLIBC_DIR}/lib" \
	-Wl,--dynamic-linker="${GLIBC_DIR}/lib/ld-linux-x86-64.so.2" \
	-D TIN_MALLOC \
	-std=c11 my_malloc.c


ldd ./a.out
#
#	-Xlinker -I "${GLIBC_DIR}/lib/libc.so.6" \
