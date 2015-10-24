include ../config.make

DEPEND    = ../../../depend
TOP_LEVEL = ..

TINYXML  :=  $(DEPEND)/TinyXML-2.6.2/tinyxml.o \
	     $(DEPEND)/TinyXML-2.6.2/tinyxmlparser.o \
             $(DEPEND)/TinyXML-2.6.2/tinyxmlerror.o \
             $(DEPEND)/TinyXML-2.6.2/tinystr.o

# sources for test programs/tools
SRC     = station.cpp network.cpp grid.cpp
OBJ     = $(SRC:.cpp=.o)

IDW_SRC = ipol_idw_test.cpp 
IDW_OBJ = $(IDW_SRC:.cpp=.o)

KRIGE_SRC = ipol_krige_test.cpp
KRIGE_OBJ = $(KRIGE_SRC:.cpp=.o)

# library dependencies
LIBS       = -L$(DEPEND)/libmap++ -lmap++ -lm

CPPFLAGS  +=  -I$(DEPEND)/TinyXML-2.6.2 \
              -I$(DEPEND)/Eigen-3.2.0 \
              -I$(DEPEND)/libmap++/include

# compilation rules
.cpp.o:
	$(CPP) $(CPPFLAGS) -c $<

all: ipol_idw_test ipol_krige_test
ipol_idw_test: $(OBJ) $(IDW_OBJ) $(TINYXML)
	$(LD) -static $(LDFLAGS) $(OBJ) $(IDW_OBJ) $(TINYXML) $(LIBS) -o $@

ipol_krige_test: $(OBJ) $(KRIGE_OBJ) $(TINYXML) $(DEPEND)/libmap++/libmap++.a
	$(LD) -static $(LDFLAGS) $(OBJ) $(KRIGE_OBJ) $(TINYXML) $(LIBS) -o $@

clean:
	@rm -f $(IDW_OBJ) $(KRIGE_OBJ) $(OBJ)
	@rm -f ipol_idw_test ipol_krige_test
	@rm -f core

# dependencies
$(TINYXML):
	@cd $(DEPEND)/TinyXML-2.6.2; make ; cd -

$(DEPEND)/libmap++/libmap++.a:
	@cd $(DEPEND)/libmap++; make; cd -
