INCLUDE := -I ../include


CFLAGS := -Og -ggdb -Wall -Wextra -fpic -MMD $(INCLUDE)
LFLAGS := -lakrypt -lm
CC := clang

ifeq ($(SANITIZE),y)
CFLAGS += -fsanitize=address -fsanitize=undefined
endif

CFLAGS_LIB = -shared

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRC))
DEP = $(OBJ:%.o=%.d)

LIBNAME = librand.so


all: $(OBJ) $(BUILD_DIR)/$(LIBNAME)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(LIBNAME): $(OBJ)
	export LD_LIBRARY_PATH=$(BUILD_DIR)
	$(CC) $(OBJ) -o $@ $(CFLAGS_LIB) $(LFLAGS)

-include $(DEP)

clean:
	rm $(OBJ) $(DEP) 2>/dev/null || true

