INCLUDE = -I ../include
LINK    = -L $(LIBRARY_DIR) -lrand
CC := clang

CFLAGS := -Og -ggdb -Wall -Wextra $(INCLUDE)

ifeq ($(SANITIZE),y)
CFLAGS += -fsanitize=address -fsanitize=undefined
endif

TESTS := noise \
	 coder \
	 generator \
	 noise_extracted \
	 speed \

EXE = $(patsubst %, $(BUILD_DIR)/test_%, $(TESTS))

all: $(EXE)

$(BUILD_DIR)/test_%: test_%.c
	$(CC) $(CFLAGS) -o $@ $< $(LINK)

clean:
	rm -f $(BUILD_DIR)/*
