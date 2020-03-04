TARGET = my_malloc
SRC = $(TARGET).c
OBJ = $(TARGET).o
DEPS = $(patsubst %, %.d, $(TARGET))

CC = gcc
CFLAGS = -g
LDFLAGS = -nostdlib -nostartfiles -static -pthread
GLIBCDIR = /usr/local/lib/glibc-test/lib
INCDIR = /usr/local/lib/glibc-test/include
STARTFILES = $(GLIBCDIR)/crt1.o $(GLIBCDIR)/crti.o `gcc --print-file-name=crtbegin.o`
ENDFILES = `gcc --print-file-name=crtend.o` $(GLIBCDIR)/crtn.o
LIBGROUP = -Wl,--start-group $(GLIBCDIR)/libc.a -lgcc -lgcc_eh -Wl,--end-group

.PHONY: all clean 

all: $(TARGET)	

$(TARGET): $(OBJ)
	@echo "Statically linking..."
	$(CC) $(LDFLAGS) -o $@ $(STARTFILES) $^ $(LIBGROUP) $(ENDFILES) 

$(OBJ): $(SRC)
	@echo "Compiling $(OBJ)..."
	$(CC) $(CFLAGS) -c $^ -I `gcc --print-file-name=include` -I $(INCDIR)

clean: 
	@echo "Cleaning..."
	@rm -fv *.o *.~ .*.swp $(TARGET)
