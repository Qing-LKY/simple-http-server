OBJ := ds.o main.o
OBJ += opt_utils.o opt_cmd.o opt_file.o opt_common.o 
OBJ += sig.o worker.o
OBJ += sock.o
OBJ += http_handler.o http_parser.o http_utils.o

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