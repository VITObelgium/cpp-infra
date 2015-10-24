include ../config.make

DEPEND    = ../../../depend
TOP_LEVEL = ..

TINYXML  :=  $(DEPEND)/TinyXML-2.6.2/tinyxml.o \
	     $(DEPEND)/TinyXML-2.6.2/tinyxmlparser.o \
             $(DEPEND)/TinyXML-2.6.2/tinyxmlerror.o \
             $(DEPEND)/TinyXML-2.6.2/tinystr.o

# sources for test programs/tools
TEST1_SRC = test1.cpp
TEST1_OBJ = $(TEST1_SRC:.cpp=.o)

# library dependencies
LIBS       = -L$(TOP_LEVEL)/src -lfeedforwardnet -lm

CPPFLAGS  +=  -I$(DEPEND)/TinyXML-2.6.2 \
              -I$(DEPEND)/Eigen-3.2.0 \
              -I$(TOP_LEVEL)/src

# compilation rules
.cpp.o:
	$(CPP) $(CPPFLAGS) -c $<

all: test1
test1: $(TOP_LEVEL)/src/libfeedforwardnet.a $(TEST1_OBJ) $(TINYXML)
	$(LD) -static $(LDFLAGS) $(TEST1_OBJ) $(TINYXML) $(LIBS) -o $@

clean:
	@rm -f $(TEST1_OBJ)
	@rm -f test1

# dependencies
$(TINYXML):
	@cd $(DEPEND)/TinyXML-2.6.2; make ; cd -

$(TOP_LEVEL)/src/libfeedforwardnet.a:
	@cd $(TOP_LEVEL)/src; make; cd -
