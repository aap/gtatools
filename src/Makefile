OBJDIR = ../obj
_OBJ = lua.o gta.o directory.o pipeline.o gl.o world.o camera.o primitives.o \
drawable.o objects.o texman.o water.o timecycle.o sky.o ifp.o col.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
HEADER = gta.h camera.h directory.h gl.h pipeline.h quat.h world.h \
primitives.h drawable.h objects.h texman.h water.h timecycle.h sky.h lua.h \
ifp.h col.h
CC = g++
CFLAGS = -Wall -g
INC = -I$(HOME)/rwtools/src

gtatools: $(OBJ)

$(OBJDIR)/%.o : %.cpp $(HEADER)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
