include ../config.make

DEPEND    = ../../../depend
TOP_LEVEL = ..

TINYXML  :=  $(DEPEND)/TinyXML-2.6.2/tinyxml.o \
             $(DEPEND)/TinyXML-2.6.2/tinyxmlparser.o \
             $(DEPEND)/TinyXML-2.6.2/tinyxmlerror.o \
             $(DEPEND)/TinyXML-2.6.2/tinystr.o

# sources for test programs/tools
VDM_SRC = vandermonde.cpp
VDM_OBJ = $(VDM_SRC:.cpp=.o)

DKTEST_SRC = detr_krige_test.cpp
DKTEST_OBJ = $(DKTEST_SRC:.cpp=.o)

# library dependencies
LIBS       = -L$(TOP_LEVEL)/src -ldetrkrige -lm

CPPFLAGS  +=  -I$(DEPEND)/TinyXML-2.6.2 \
              -I$(DEPEND)/Eigen-3.2.0 \
              -I$(TOP_LEVEL)/src

# compilation rules
.cpp.o:
	$(CPP) $(CPPFLAGS) -c $<

all: vandermonde detr_krige_test
vandermonde: $(VDM_OBJ)
	$(LD) -static $(LDFLAGS) $(VDM_OBJ) $(LIBS) -o $@

detr_krige_test: $(TOP_LEVEL)/src/libdetrkrige.a $(DKTEST_OBJ) $(TINYXML)
	$(LD) -static $(LDFLAGS) $(DKTEST_OBJ) $(TINYXML) $(LIBS) -o $@

clean:
	@rm -f $(VDM_OBJ)
	@rm -f vandermonde
	@rm -f $(DKTEST_OBJ)
	@rm -f detr_krige_test

# dependencies
$(TINYXML):
	@cd $(DEPEND)/TinyXML-2.6.2; make ; cd -

$(TOP_LEVEL)/src/libdetrkrige.a:
	@cd $(TOP_LEVEL)/src; make; cd -
