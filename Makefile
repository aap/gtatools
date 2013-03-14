SRCDIR = src
BUILDDIR = build
RWDIR = $(HOME)/rwtools
SRC := $(wildcard $(SRCDIR)/*.cpp)
OBJ := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC))
DEP := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.d,$(SRC))
INC = -I$(RWDIR)/include
CFLAGS = $(INC) -Wall -Wextra -g# -O3 -pg
LINK = $(RWDIR)/lib/librwtools.a\
  -lGL -lglfw -lGLEW -lpthread -lreadline -llua    #lua5.1
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

