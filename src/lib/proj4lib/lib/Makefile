LIB = proj4lib.a 
SRCS = $(wildcard *.cpp) 
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.depends)
include ../../../MakeIncludes
INCDIRS = -I../include $(ADD_INC_DIRS) -I../../dglib/include
CCFLAGS = ${INCDIRS} ${ADD_CPP_FLAGS} 

.PHONY: clean all

all: $(LIB)

$(LIB): $(SRC) $(OBJS) $(DEPS)
	ar ruv $(LIB) $(OBJS)
	ranlib $(LIB)

%.o: %.cpp 
	$(CPPCOMP) -c $< $(CCFLAGS)

%.depends: %.cpp
	$(CPPCOMP) -M $(CCFLAGS) $< > $@

clean:
	rm -rf $(OBJS) $(LIB) $(DEPS) ii_files ti_files make.out

-include $(DEPS)
