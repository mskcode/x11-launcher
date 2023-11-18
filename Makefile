#
# Makefile
#

CC = g++
CC_FLAGS = -std=c++20 -Wall -g -O0 -I/usr/include/X11

LD = g++
LD_FLAGS = -g -lX11

EXE_NAME = x11-launcher

SRC_DIR = ./src
OUT_DIR = ./build

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(addprefix $(OUT_DIR)/,$(notdir $(SRCS:%.cpp=%.o)))
EXE = $(OUT_DIR)/$(EXE_NAME)

all: $(EXE)

$(EXE): $(OBJS)
	@echo "Linking executable..."
	$(LD) $(LD_FLAGS) $^ -o "$@"

$(OBJS): $(SRCS)
	@echo "Compiling sources..."
	@mkdir -p build
	@echo "CC: $< --> $@"
	$(CC) $(CC_FLAGS) "$<" -c -o "$@"

.PHONY: clean
clean:
	@echo Removing $(OUT_DIR)...
	@rm -fr $(OUT_DIR)

.PHONY: watch
watch:
	while true; do $(MAKE) -q || $(MAKE); sleep 2; done
