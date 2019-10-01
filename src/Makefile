include ./MakeIncludes
DGGRID_DIR := ./apps/dggrid
APPEX_DIR := ./apps/appex
DG_DIR := ./lib/dglib
SHP_DIR := ./lib/shapelib
PROJ4_DIR := ./lib/proj4lib
LIB_DIRS := $(DG_DIR)/lib $(SHP_DIR)/lib $(PROJ4_DIR)/lib

.PHONY: all $(DGGRID_DIR) $(LIB_DIRS)

all: $(DGGRID_DIR) $(APPEX_DIR)

$(DGGRID_DIR) $(APPEX_DIR) $(LIB_DIRS):
	$(MAKE) --directory=$@

$(DGGRID_DIR) $(APPEX_DIR): $(LIB_DIRS)

clean:
	cd $(DGGRID_DIR); $(MAKE) clean
	cd $(APPEX_DIR); $(MAKE) clean
	cd $(SHP_DIR)/lib; $(MAKE) clean
	cd $(PROJ4_DIR)/lib; $(MAKE) clean
	cd $(DG_DIR)/lib; $(MAKE) clean
