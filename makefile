CR := gcc
EXE := server

FLAGS := -std=c99 -Wall
LIBS := -lwsock32 -lws2_32

FOOT = -o $@
HEAD = -c $<

OBJ := ./obj/

NEED := $(OBJ)server.o $(OBJ)hashlist.o

$(EXE): $(NEED)
	$(CR) $(FLAGS) $(NEED) $(FOOT) $(LIBS)

$(OBJ)%.o: %.c
	$(CR) $(FLAGS) $(HEAD) $(FOOT) $(LIBS)

clean:
	cd $(OBJ) & $(foreach obj,$(wildcard $(OBJ)*.o),del $(subst $(OBJ), ,$(obj));)