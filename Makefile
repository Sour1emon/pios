ARMGNU ?= ./arm-gnu-toolchain-15.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf


COPS = -Wall -Wextra -nostdlib -nostartfiles -ffreestanding -Iinclude
ASMOPS = -Iinclude

BUILD_DIR = build
SRC_DIR = src
BOOT_IMG = boot.img
CONFIG_TXT = config.txt

all: kernel8.img

clean:
	@rm -rf $(BUILD_DIR) *.img

# Compile C files quietly, only show warnings/errors
$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@ >/dev/null

# Compile assembly files quietly, only show warnings/errors
$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	@$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@ >/dev/null

C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

# Link quietly, only show warnings/errors
kernel8.img: check-toolchain $(SRC_DIR)/linker.ld $(OBJ_FILES)
	@$(ARMGNU)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES) >/dev/null
	@$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary kernel8.img

# Create boot.img with config.txt quietly
$(BOOT_IMG): $(CONFIG_TXT)
	@dd if=/dev/zero of=$(BOOT_IMG) bs=1M count=64 status=none
	@mformat -i $(BOOT_IMG) ::/
	@mcopy -i $(BOOT_IMG) $(CONFIG_TXT) ::/

.PHONY: check-toolchain
check-toolchain:
	@mkdir -p $(BUILD_DIR)
	@printf ".arch armv8-a\nmrs x0, mpidr_el1\n" > $(BUILD_DIR)/check.S
	-@$(ARMGNU)-gcc -c $(BUILD_DIR)/check.S -o $(BUILD_DIR)/check.o >/dev/null 2>&1 \
		|| (echo "Error: $(ARMGNU)-gcc does not support AArch64. Install an aarch64 toolchain and run 'make ARMGNU=aarch64-elf'"; exit 1)
	@rm -f $(BUILD_DIR)/check.S $(BUILD_DIR)/check.o

.PHONY: run
run: kernel8.img $(BOOT_IMG)
	@command -v qemu-system-aarch64 >/dev/null 2>&1 || { echo "qemu-system-aarch64 not found in PATH"; exit 1; }
	@qemu-system-aarch64 -m 1024 -no-reboot -M raspi3b -serial stdio \
		-kernel "$(CURDIR)/kernel8.img" \
		-drive file="$(CURDIR)/$(BOOT_IMG)",format=raw,if=sd,media=disk

.PHONY: debug
debug: kernel8.img $(BOOT_IMG)
	@command -v qemu-system-aarch64 >/dev/null 2>&1 || { echo "qemu-system-aarch64 not found in PATH"; exit 1; }
	@qemu-system-aarch64 -m 1024 -no-reboot -M raspi3b -serial stdio \
		-kernel "$(CURDIR)/kernel8.img" \
		-drive file="$(CURDIR)/$(BOOT_IMG)",format=raw,if=sd,media=disk \
		-d guest_errors,unimp,int
