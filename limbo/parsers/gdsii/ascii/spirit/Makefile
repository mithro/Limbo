#==========================================================================
#                         Directories and names 
# ==========================================================================

PARSER_PREFIX = Gds
LIB_PREFIX = gds
DEBUG_PREFIX = $(shell echo $(PARSER_PREFIX) | tr a-z A-Z)# lower case to upper case 
LIMBO_ROOT_DIR = $(realpath ../../../../..)

#==========================================================================
#                         Compilation Flags
# ==========================================================================

# include environmental configurations 
include $(LIMBO_ROOT_DIR)/Include.mk

# ==========================================================================
#                         Standard Setting
# ==========================================================================

.PHONY: install
install:
	mkdir -p $(PREFIX)/include/limbo/parsers/$(LIB_PREFIX)ii/ascii/spirit
	cp $(wildcard *.h) $(PREFIX)/include/limbo/parsers/$(LIB_PREFIX)ii/ascii/spirit
