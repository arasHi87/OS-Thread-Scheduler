
GIT_HOOKS := .git/hooks/applied
CC := gcc
CFLAGS += -std=gnu99 -g -Wall
CLIB += -ljson-c
OBJ := simulator.o os2021_thread_api.o function_libary.o parse_json.o

all: $(GIT_HOOKS) 

$(GIT_HOOKS):
	@.githooks/install-git-hooks
	@echo

simulator:$(OBJ)
	$(CC) $(CFLAGS) -o simulator $(OBJ) $(CLIB)

simulator.o: os2021_thread_api.h
os2021_thread_api.o: os2021_thread_api.h function_libary.h
parse_json.o:  os2021_thread_api.h parse_json.h 
function_libary.o: function_libary.h os2021_thread_api.h

.PHONY: clean
clean:
	-rm $(OBJ) simulator