
# ROOT Stuff
ROOTC = $(shell root-config --cflags)
ROOTLIB = $(shell root-config --libs)

COPTS=$(ROOTC) -Iinc -c -g -fPIC
LOPTS=$(ROOTLIB)

INCPATH=inc
SRCPATH=src
BINPATH=bin
OBJPATH=obj
DEPPATH=dep

# set VPATH to look in source path
VPATH=$(SRCPATH):$(INCPATH):$(DEPPATH):$(OBJPATH)

CPPFILES := $(foreach dir, $(SRCPATH)/, $(notdir $(wildcard $(SRCPATH)/*.cpp)))
CXXFILES := $(foreach dir, $(SRCPATH)/, $(notdir $(wildcard $(SRCPATH)/*.cxx)))
INCFILES := $(foreach dir, $(INCPATH)/, $(notdir $(wildcard $(INCPATH)/*.h)))
DEPFILES := $(CPPFILES:.cpp=.d)
DEPFILES2 := $(CPPFILES:.cpp=.d)
DEPFILES := $(addprefix $(DEPPATH)/,$(DEPFILES))
OBJFILES := $(CPPFILES:.cpp=.o)
OBJFILES := $(addprefix $(OBJPATH)/,$(OBJFILES))

BINOBJFILES := $(CXXFILES:.cxx=.o)
BINOBJFILES := $(addprefix $(OBJPATH)/,$(BINOBJFILES))
BINDEPFILES := $(CXXFILES:.cxx=.d)
BINDEPFILES := $(addprefix $(DEPPATH)/,$(BINDEPFILES))
DEPFILES := $(DEPFILES) $(BINDEPFILES)
BINFILES := $(CXXFILES:.cxx=)
BINFILES := $(addprefix $(BINPATH)/,$(BINFILES))

GCC=g++

target: $(BINFILES) $(OBJFILES) $(BINOBJFILES) 

-include $(DEPFILES)

# Link binary executables
$(BINPATH)/%: $(OBJPATH)/%.o $(OBJFILES)
	@echo " "
	@echo "--------->>>>>> Linking" $*
	$(GCC) $(LOPTS) $(OBJFILES) $< -o $@
	@echo "----->>> Done."


# Build binary Source
$(OBJPATH)/%.o: %.cxx
	@echo " "
	@echo "--------->>>>>> Compiling" $*.o
	$(GCC) -MD $(COPTS) $< -o $@
	@mv $(OBJPATH)/*.d $(DEPPATH)/
	@echo "----->>> Done."

# Build Generic Object Files
$(OBJPATH)/%.o: %.cpp
	@echo " "
	@echo "--------->>>>>> Compiling" $*.o
	$(GCC) -MD $(COPTS) $< -o $@
	@mv $(OBJPATH)/*.d $(DEPPATH)/
	@echo "----->>> Done."

clean:
	@echo " "
	@echo "--------->>>>>> Cleaning"
	rm -f $(OBJFILES) $(BINPATH)/* $(DEPFILES) *~ */*~ */*.o
	@echo "----->>> Done."

