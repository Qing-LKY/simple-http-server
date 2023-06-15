DEMO_BIN := say_something

all: $(DEMO_BIN)

$(DEMO_BIN):%: %.c
	$(CC) $< -o $@

.phony: clean

clean:
	$(RM) $(DEMO_BIN)