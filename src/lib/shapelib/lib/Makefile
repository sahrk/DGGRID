LIB = shapelib.a 
SRCS = $(wildcard *.c) 
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.depends)
include ../../../MakeIncludes
INCDIRS = -I../include $(ADD_INC_DIRS)
CCFLAGS = ${INCDIRS} ${ADD_CC_FLAGS} 

.PHONY: clean all

all: $(LIB)

$(LIB): $(SRC) $(OBJS) $(DEPS)
	ar ruv $(LIB) $(OBJS)
	ranlib $(LIB)

%.o: %.c 
	$(CCOMP) -c $< $(CCFLAGS)

%.depends: %.c
	$(CCOMP) -M $(CCFLAGS) $< > $@

clean:
	rm -rf $(OBJS) $(LIB) $(DEPS) ii_files ti_files make.out

-include $(DEPS)
