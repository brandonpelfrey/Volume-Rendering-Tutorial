CXX := g++
RELEASE := 1

OPT := -Wall -fopenmp
ifeq ($(RELEASE),1)
OPT += -O3 -ffast-math
else
OPT += -g
endif

NAME := VolumeRenderer
DEBUG := 0
BINDIR := bin
SRCDIR := src
SRC := $(shell find $(SRCDIR) -name "*.cpp")
HEADER := $(shell find $(SRCDIR) -name "*.h")
INC	:= $(addprefix -I , $(shell find $(SRCDIR) -name ".svn" -prune -o -type d))
ARCH := $(shell uname)
SUM	= $(shell find . -name "*.h" -print0 -o -name "*.cpp" -print0 | xargs -0 wc -l | grep total )

OPT += $(shell Magick++-config --cppflags --cxxflags --ldflags --libs)

ifeq ($(ARCH),Darwin)
	LIB := -fopenmp
endif

ifeq ($(ARCH),Linux)
	LIB := -fopenmp
endif

all:
	@make --no-print-directory depend
	@make --no-print-directory $(BINDIR)/$(NAME) #DEP=1
	@echo "\nLine sum: ${SUM}"

$(BINDIR)/$(NAME) : $(SRC:.cpp=.o)
	@$(CXX) -o $@ $(SRC:.cpp=.o) $(LIB) $(OPT)
	@echo "Linking $@..."

run: all
	cd $(BINDIR); 	./$(NAME)

depend : $(SRC:.cpp=.d)
%.d : %.cpp
	@echo "$@ $(dir $@)$$(gcc $(INC) -MM -MG $<)" > $@

%.o : %.cpp
	@$(CXX) $(OPT) $(INC) -o $@ -c $<
	@echo "Compiling $< ..."

.PHONY: clean
clean:
	rm -rf $(shell find . -name "*.o" -o -name "*.d")
	rm -rf $(BINDIR)/$(NAME)
	rm -rf $(BINDIR)/*.bmp
	rm -rf GPATH GRTAGS GSYMS GTAGS HTML

ifeq ($(DEP), 1)
-include $(SRC:.cpp=.d)
endif
