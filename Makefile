# Makefile for SAM C embedded projects
#
# Automatic build directory creation from:
# http://ismail.badawi.io/blog/2017/03/28/automatic-directory-creation-in-make/

# Build sources and outputs
SRCS := main.c board.c
SRCS += deps/asf/sam0/utils/cmsis/samc20/source/gcc/startup_samc20.c
OUT := main.out
LINKER_SCRIPT := deps/asf/sam0/utils/linker_scripts/samc20/gcc/samc20e16a_flash.ld
BUILD_DIR := build

CPU := cortex-m0plus
DEVICE := SAMC20E16A
STACK_SIZE := 2048

PREFIX := arm-none-eabi-
CC := $(PREFIX)gcc
SIZE := $(PREFIX)size
GDB := $(PREFIX)gdb

CFLAGS := -Wall -std=gnu99 -mcpu=$(CPU) -mthumb -ffunction-sections -g
CFLAGS += -Wstack-usage=$(STACK_SIZE)
CFLAGS += -I. -Ideps/asf/sam0/utils/cmsis/samc20/include
CFLAGS += -Ideps/asf/thirdparty/CMSIS/Include
CFLAGS += -D__$(DEVICE)__ -DDONT_USE_CMSIS_INIT -Dprintf=iprintf
CFLAGS += -Os

LDFLAGS := -mcpu=$(CPU) -mthumb -Wl,--gc-sections
LDFLAGS += -Wl,--defsym=STACK_SIZE=$(STACK_SIZE)

OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))
OUT := $(BUILD_DIR)/$(OUT)

.PHONY: all clean debug deploy

.PRECIOUS: $(BUILD_DIR)/. $(BUILD_DIR)%/.

all: deps/asf/. $(OUT)

clean:
	rm -rf build/*

deps/.:
	mkdir -p $@

deps/asf-standalone.zip: deps/.
	wget -nc -O $@ https://www.microchip.com/mymicrochip/filehandler.aspx?ddocname=en1000272 || true

deps/asf/.: deps/asf-standalone.zip
	rm -rf deps/*/
	unzip -q $< */sam0/utils/cmsis/samc20/* */sam0/utils/linker_scripts/samc20/* */thirdparty/CMSIS/Include/* -d deps/
	mv deps/*/ deps/asf

debug:
	$(GDB) -ex "target remote localhost:2331" $(OUT)

deploy: $(OUT)
	$(GDB) -batch -x load.gdb $<

$(OUT): $(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@ -T$(LINKER_SCRIPT)
	@$(SIZE) $^ $@

$(BUILD_DIR)/.:
	mkdir -p $@

$(BUILD_DIR)%/.:
	mkdir -p $@

.SECONDEXPANSION:

$(BUILD_DIR)/%.o: %.c | $$(@D)/.
	echo $^ $|
	$(CC) $(CFLAGS) -c -o $@ $<
