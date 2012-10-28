OBJDIR = ../obj
_OBJ = animation.o jobqueue.o gta.o renderer.o gl.o lua.o directory.o \
pipeline.o world.o camera.o primitives.o drawable.o objects.o \
texman.o water.o timecycle.o sky.o animread.o col.o input.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
HEADER = gta.h camera.h directory.h gl.h pipeline.h quat.h world.h \
primitives.h drawable.h objects.h texman.h water.h timecycle.h sky.h lua.h \
animation.h col.h renderer.h jobqueue.h
CC = g++
CFLAGS = -Wall -Wextra -O3 -g #-Wold-style-cast
INC = -I$(HOME)/rwtools/src

gtatools: $(OBJ)

$(OBJDIR)/%.o : %.cpp $(HEADER)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
