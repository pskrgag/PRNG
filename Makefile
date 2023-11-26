.PHONY: dir dir_test

BUILD_DIR = $(shell pwd)/build
BUILD_DIR_TEST = $(BUILD_DIR)/test/

NUM_SAMPLES_TEST := 1000000

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

histogram_coder: test
	export LD_LIBRARY_PATH=$HOME/Documents/diplom/generator/build:$HOME/Documents/diplom/libakrypt-0.x/build
	$(BUILD_DIR_TEST)/test_coder /tmp/coder $(NUM_SAMPLES_TEST)
	python3 ./dist.py /tmp/coder

histogram_noise: test
	export LD_LIBRARY_PATH=$HOME/Documents/diplom/generator/build:$HOME/Documents/diplom/libakrypt-0.x/build
	$(BUILD_DIR_TEST)/test_memory /tmp/memory_noise $(NUM_SAMPLES_TEST) 10
	ls -1 /tmp/memory_noise* | xargs python3 ./dist.py

histogram_prng: test
	export LD_LIBRARY_PATH=$HOME/Documents/diplom/generator/build:$HOME/Documents/diplom/libakrypt-0.x/build
	$(BUILD_DIR_TEST)/test_generator /tmp/generator $(NUM_SAMPLES_TEST)
	python3 ./dist.py /tmp/generator
