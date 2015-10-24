include ../config.make

TOP_LEVEL := ..
DEPEND    := ../../../depend

# use -msse2 to surely enable Eigen to vectorize the code 
# (on x86_64 should be by default)
CPPFLAGS   += -I$(DEPEND)/TinyXML-2.6.2 \
              -I$(DEPEND)/Eigen-3.2.0

TINYXML    :=  $(DEPEND)/TinyXML-2.6.2/tinyxml.o \
	       $(DEPEND)/TinyXML-2.6.2/tinyxmlparser.o \
               $(DEPEND)/TinyXML-2.6.2/tinyxmlerror.o \
               $(DEPEND)/TinyXML-2.6.2/tinystr.o

SRC = DetrendedKrigingInterpolator.cpp \
      TrendModel.cpp \
      MultiLinearTrendModel.cpp \
      PolyTrendModel.cpp \
      SpatialCorrelator.cpp

OBJ = $(SRC:.cpp=.o)

# compilation rules
.cpp.o:
	$(CPP) $(CPPFLAGS) -c $<

all: libdetrkrige.a

libdetrkrige.a: $(OBJ)
	$(AR) $@ $(OBJ)

clean:
	@rm -f *.o
	@rm -f libdetrkrige.a

