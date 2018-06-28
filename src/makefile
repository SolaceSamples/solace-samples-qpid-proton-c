# general path references
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))
current_path := $(patsubst %/,%,$(dir $(mkfile_path)))

# build variables
CC=gcc
LIBS=-lqpid-proton
CFLAGS=-I. 
APP_NAMES=send receive producer dte_consumer dte_solconsumer
BINDIR=$(current_path)/bin
ODIR=$(current_path)/obj
_OBJ= $(patsubst %,%.o,$(APP_NAMES))
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
BIN = $(patsubst %,$(BINDIR)/%,$(APP_NAMES))
EXAMPLE_DEPENDCIES=$(ODIR)/util.o

## Targets ##

.PHONY: all

all: $(APP_NAMES)

.PHONY: build

build: $(APP_NAMES)

# general rule for .c compile to .o
$(ODIR)/%.o: $(current_path)/%.c
	mkdir $(ODIR) -p
	$(CC) -c -o $@ $< $(CFLAGS)

# rule template for <application> where $(1) is an <application>
define SAMPLE_RULE

.PHONY: $(1)

$(1): $(patsubst %,$$(BINDIR)/%,$(1))

$(patsubst %,$$(BINDIR)/%,$(1)): $(patsubst %,$$(ODIR)/%.o,$(1)) $$(EXAMPLE_DEPENDCIES) 
	mkdir -p $$(BINDIR)
	$$(CC) -o $$@ $$^ $$(CFLAGS) $$(LIBS)

endef

# create all <application> rules for each $APP in $APP_NAMES
$(foreach APP,$(APP_NAMES), $(eval $(call SAMPLE_RULE, $(APP)) ) )

# clean target
.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
	rm -rf $(ODIR) $(BINDIR)

# help target
.PHONY: help

help:
	@echo "make tagets:"
	@echo "    all: default target and makes all applications from list: $(APP_NAMES)"
	@echo "    build: see target all"
	@echo "    help: displays this message"
	@echo "    <application>: makes <application> from application list: $(APP_NAMES)"

## end Targets ##
