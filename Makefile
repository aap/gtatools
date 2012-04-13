OBJDIR = obj
_OBJ = gta.o directory.o world.o camera.o pipeline.o gl.o primitives.o \
drawable.o objects.o texman.o water.o timecycle.o sky.o lua.o ifp.o col.o \
renderer.o objman.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
RWDIR=$(HOME)/rwtools
CC = g++
CFLAGS = -Wall -g -L$(RWDIR) -Wl,-Bstatic -lrwtools \
-Wl,-Bdynamic -lGL -lGLU -lglut -lGLEW -lpthread -llua -lreadline
BIN=gta

build: $(OBJRW) $(OBJ) 
	$(CC) -o $(BIN) $(OBJ) $(CFLAGS)
#	$(CC) -o colread src/colread.cpp obj/col.o $(CFLAGS) -I../rwtools/src

$(OBJ) $(OBJDIR)/:
	cd src && make

$(OBJRW):
	cd $(OBJDIRRW)/.. && make

clean:
	rm $(OBJDIR)/*

install:
	cp $(BIN) $(HOME)/bin
