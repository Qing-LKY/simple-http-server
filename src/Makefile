OBJ := ds.o main.o
OBJ += opt_utils.o opt_cmd.o opt_file.o opt_common.o 
OBJ += sig.o
OBJ += worker.o

INCLUDE := $(addprefix ../, $(INCLUDE))
TARGET := ../$(TARGET)
CFLAGS += $(patsubst %, -I%, $(INCLUDE))

all: $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

$(OBJ):%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

.phony: clean

clean:
	$(RM) $(OBJ)