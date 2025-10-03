RPI_VERSION ?= 3
ARMGNU ?= aarch64-elf

COPS = -DRPI_VERSION=$(RPI_VERSION) -Wall -nostdlib -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only
ASMOPS = -Iinclude

BUILD_DIR = build
SRC_DIR = src

C_FILES := $(shell find $(SRC_DIR) -name "*.c")
CPP_FILES := $(shell find $(SRC_DIR) -name "*.cpp")
ASM_FILES := $(shell find $(SRC_DIR) -name "*.S")

C_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%_c.o,$(C_FILES))
CPP_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%_cpp.o,$(CPP_FILES))
ASM_OBJS := $(patsubst $(SRC_DIR)/%.S,$(BUILD_DIR)/%_s.o,$(ASM_FILES))

OBJ_FILES := $(C_OBJS) $(CPP_OBJS) $(ASM_OBJS)
DEP_FILES := $(OBJ_FILES:.o=.d)

all: kernel8.img

# Pattern rules
$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_cpp.o: $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(ARMGNU)-g++ $(COPS) -fno-exceptions -fno-rtti -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

# Link and generate final image
kernel8.img: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	@echo "Building kernel8.img for RPI $(RPI_VERSION)"
	$(ARMGNU)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary kernel8.img

clean:
	rm -rf $(BUILD_DIR) *.img

-include $(DEP_FILES)

armstub/build/armstub_s.o: armstub/src/armstub.S
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

armstub: armstub/build/armstub_s.o
	$(ARMGNU)-ld --section-start=.text=0 -o armstub/build/armstub.elf armstub/build/armstub_s.o
	$(ARMGNU)-objcopy armstub/build/armstub.elf -O binary armstub-new.bin

