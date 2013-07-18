SRCDIR = src
BUILDDIR = build
RWDIR = $(HOME)/src/rwtools
SRC := $(wildcard $(SRCDIR)/*.cpp)
OBJ := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC))
DEP := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.d,$(SRC))
INC = -I$(RWDIR)/include -I/usr/local/include/bullet
CFLAGS = $(INC) -Wall -Wextra -g #-pg -O2
LINK = $(RWDIR)/lib/librwtools.a\
  -lGL -lglfw -lGLEW -lpthread -lreadline -llua\
  -lBulletCollision -lBulletDynamics -lLinearMath
TARGET = gta

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LINK) -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: $(SRCDIR)/%.cpp
	$(CXX) -MM -MT '$(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$<)' $(CFLAGS) $< > $@

clean:
	rm -f build/* $(TARGET)

-include $(DEP)

