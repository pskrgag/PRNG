.PHONY: dir dir_test

BUILD_DIR = ../build
BUILD_DIR_TEST = $(BUILD_DIR)/test/

all: dir
	$(MAKE) -C src/ BUILD_DIR=$(BUILD_DIR)

clean:
	$(MAKE) -C src/ BUILD_DIR=$(BUILD_DIR) clean
	$(MAKE) -C test/ BUILD_DIR=$(BUILD_DIR_TEST) clean

test: dir_test all
	$(MAKE) -C test/ BUILD_DIR=$(BUILD_DIR_TEST) LIBRARY_DIR=$(BUILD_DIR)

dir:
	mkdir -p $(BUILD_DIR)

dir_test: dir
	mkdir -p $(BUILD_DIR_TEST)
