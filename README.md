# Malloc-glibc
In this project, I do some research about the virtual memory, process address space, the malloc GNU C library.
Then modify the malloc source code inside glibc to add some feature allowing me to track memory leak. 
## Getting Started
The *my_malloc.c* file is a small example to allocate & deallocate memory to simulate the actual running program with malloc & free.
We can use *Makefile* (for static linking) or *use-glibc.sh* (for dynamic linking) to compile and link to our specific glibc.
### Prerequisites
You can choose a [version of glibc](https://ftp.gnu.org/gnu/libc/) & learn how to install them as a second library for your system.
Here I choose the library glibc-2.22
```
git clone git://sourceware.org/git/glibc.git
cd glibc
git checkout glibc-2.22
```
### Install glibc
Place glibc source code at */usr/local/src/glibc-2.22/* 
```
cd /usr/local/src
mkdir glibc-build -p && cd glibc-build
```
Config the install directory as /usr/local/lib/glibc-install/, if you choose the install directory as /usr, the new glibc will override the glibc of your system and that's not a good idea!
```
../glibc-2.22/configure --prefix=/usr/local/lib/glibc-install/ --enable-add-ons --with-tls --disable-werror
```
Build the source code
```
make -j $(nproc)
```
Make install to the specific directory
```
make install -j $(nproc)
```
Now you have the secondary library in your system and you can build an application with linking to your new glibc.

## Exploiting the Glibc Malloc
First of all, we do some research about data structure inside the glibc malloc, such as [Arena, Heap and Chunk](https://sourceware.org/glibc/wiki/MallocInternals). And [the malloc source code](https://elixir.bootlin.com/glibc/glibc-2.22/source/malloc/malloc.c#L2895), it's actually a wrapper function of [internal malloc](https://elixir.bootlin.com/glibc/glibc-2.22/source/malloc/malloc.c#L3312)

The main arena and thread arena (with single heap segment) look like:
![](https://docs.google.com/drawings/d/1367fdYcRTvkyfZ_s27yg6oJp5KYsVAuYqPf8szbRNc0/pub?w=960&h=720)

A thread arena (with multiple heap segments) looks like:
![](https://docs.google.com/drawings/d/150bTi0uScQlnABDImLYS8rWyL82mmfpMxzRbx-45UKw/pub?w=960&h=720)

