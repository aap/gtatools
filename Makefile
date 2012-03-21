OBJDIR = obj
_OBJ = gta.o directory.o world.o camera.o pipeline.o gl.o primitives.o \
drawable.o objects.o texman.o water.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
RWDIR=$(HOME)/rwtools
CC = g++
CFLAGS = -Wall -g -L$(RWDIR) -Wl,-Bstatic -lrwtools \
-Wl,-Bdynamic -lGL -lGLU -lglut -lGLEW
BIN=gta

build: $(OBJRW) $(OBJ) 
	$(CC) -o $(BIN) $(OBJ) $(OBJRW) $(CFLAGS)

$(OBJ) $(OBJDIR)/:
	cd src && make

$(OBJRW):
	cd $(OBJDIRRW)/.. && make

clean:
	rm $(OBJDIR)/*

install:
	cp $(BIN) $(HOME)/bin
