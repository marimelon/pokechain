CC := gcc
CFLAGS	:= -Wall -Wextra
LDLIBS	:= -lncursesw

PROGRAM := pokechain
SOURCES := $(wildcard *.c)
OBJS := $(SOURCES:.c=.o)
DEPENDS := $(SOURCES:.c=.depend)

.PHONY: all
all: $(PROGRAM)
$(PROGRAM): $(OBJS) 
	$(CC) -o $(PROGRAM) $(CFLAGS) $^ $(LDLIBS)


%.depend: %.c
	@echo generating $@
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $< | sed "s/\($*\)\.o[ :]*/\1.o $@ : /g" > $@; [ -s $@ ] || rm -f $@'


ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEPENDS)
endif

.PHONY : clean
clean:
	rm -f $(PROGRAM)
	rm -fr $(OBJS)
	rm -fr $(DEPENDS)