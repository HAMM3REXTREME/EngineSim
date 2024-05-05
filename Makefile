# Compiler
CXX := clang++


ARCH := $(shell uname -m)

# Compiler flags
CXXFLAGS := -O2

# SFML flags
SFMLFLAGS := -lsfml-graphics -lsfml-window -lsfml-system

# FMOD flags
FMODFLAGS := -Llib/fmod/$(ARCH) -lfmodL -lfmodstudioL

# Directories
SRCDIR := src
OBJDIR := obj
LIBDIR := lib
INCDIR := include

# Source files
SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

# Executable
EXE := engine-sim

# Targets
all: $(EXE)

$(EXE): $(OBJS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ -L$(LIBDIR) $(SFMLFLAGS) $(FMODFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)


clean:
	rm -rf $(OBJDIR) $(EXE)