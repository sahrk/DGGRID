TARGET = dggrid
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.depends)
include ../../MakeIncludes
DGLIBDIR = ../../lib/dglib
SHPLIBDIR = ../../lib/shapelib
INCDIRS = -I$(DGLIBDIR)/include -I$(SHPLIBDIR)/include \
   $(ADD_INC_DIRS)
CCFLAGS  = ${INCDIRS} ${ADD_CPP_FLAGS} 
LDLIBS = ${ADD_LD_LIBS}

.PHONY: clean all

all: $(TARGET)

${TARGET}: $(SRC) $(OBJS) $(DEPS) $(LDLIBS)
	${CPPCOMP} $(CCFLAGS) $(EXPORT) -o ${TARGET} ${LDLIBS} ${OBJS}

%.o: %.cpp
	$(CPPCOMP) -c $< $(CCFLAGS) -o $@

link:
	${CPPCOMP} $(CCFLAGS) $(EXPORT) -o ${TARGET} ${OBJS} ${LDLIBS}

%.depends: %.cpp
	$(CPPCOMP) -M $(CCFLAGS) $< > $@

$(LOCAL_LIBS):
	cd $(SHPLIBDIR)/lib; ${MAKE}
	cd $(DGLIBDIR)/lib; ${MAKE}

clean:
	rm -rf $(OBJS) $(DEPS) ${TARGET} core ti_files ii_files

-include $(DEPS)

