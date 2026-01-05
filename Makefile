ARMGNU=aarch64-elf


COPS = -Wall -Wextra -nostdlib -nostartfiles -ffreestanding -mstrict-align -Iinclude -g
COPS_DEBUG = $(COPS) -DDEBUG
COPS_TEST = $(COPS) -DTEST_MODE
ASMOPS = -Iinclude

BUILD_DIR = build
SRC_DIR = src
TEST_DIR = tests
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

# Compile C files for debug build (with DEBUG flag)
$(BUILD_DIR)/debug/%_c.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@$(ARMGNU)-gcc $(COPS_DEBUG) -MMD -c $< -o $@ >/dev/null

# Compile assembly files for debug build
$(BUILD_DIR)/debug/%_s.o: $(SRC_DIR)/%.S
	@$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@ >/dev/null

# Compile test C files quietly (with TEST_MODE flag)
$(BUILD_DIR)/tests/%_c.o: $(TEST_DIR)/%.c
	@mkdir -p $(@D)
	@$(ARMGNU)-gcc $(COPS_TEST) -MMD -c $< -o $@ >/dev/null

# Compile src C files for test build (with TEST_MODE flag)
$(BUILD_DIR)/test_src/%_c.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@$(ARMGNU)-gcc $(COPS_TEST) -MMD -c $< -o $@ >/dev/null

# Compile assembly files for test build
$(BUILD_DIR)/test_src/%_s.o: $(SRC_DIR)/%.S
	@$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@ >/dev/null

# Source files
C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
TEST_C_FILES = $(wildcard $(TEST_DIR)/*.c)

# Object files for normal kernel
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

# Object files for debug kernel (compiled with DEBUG flag)
DEBUG_OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/debug/%_c.o)
DEBUG_OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/debug/%_s.o)

# Object files for test kernel (includes test files, compiled with TEST_MODE)
TEST_OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/test_src/%_c.o)
TEST_OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/test_src/%_s.o)
TEST_OBJ_FILES += $(TEST_C_FILES:$(TEST_DIR)/%.c=$(BUILD_DIR)/tests/%_c.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
DEP_FILES += $(TEST_OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

# Link quietly, only show warnings/errors
kernel8.img: check-toolchain $(SRC_DIR)/linker.ld $(OBJ_FILES)
	@$(ARMGNU)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES) >/dev/null
	@$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary kernel8.img

# Build debug kernel with stack traces enabled
kernel8-debug.img: check-toolchain $(SRC_DIR)/linker.ld $(DEBUG_OBJ_FILES)
	@$(ARMGNU)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8-debug.elf $(DEBUG_OBJ_FILES) >/dev/null
	@$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8-debug.elf -O binary kernel8-debug.img

# Build test kernel with test files included
kernel8-test.img: check-toolchain $(SRC_DIR)/linker.ld $(TEST_OBJ_FILES)
	@$(ARMGNU)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8-test.elf $(TEST_OBJ_FILES) >/dev/null
	@$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8-test.elf -O binary kernel8-test.img

# Create boot.img with config.txt quietly
$(BOOT_IMG): $(CONFIG_TXT)
	@dd if=/dev/zero of=$(BOOT_IMG) bs=1M count=64 status=none
	@mformat -i $(BOOT_IMG) ::/
	@mcopy -i $(BOOT_IMG) $(CONFIG_TXT) ::/

.PHONY: check-toolchain
check-toolchain:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/debug
	@mkdir -p $(BUILD_DIR)/tests
	@mkdir -p $(BUILD_DIR)/test_src
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

.PHONY: run-debug
run-debug: kernel8-debug.img $(BOOT_IMG)
	@command -v qemu-system-aarch64 >/dev/null 2>&1 || { echo "qemu-system-aarch64 not found in PATH"; exit 1; }
	@echo "Running kernel in DEBUG mode (with stack traces)..."
	@qemu-system-aarch64 -m 1024 -no-reboot -M raspi3b -serial stdio \
		-kernel "$(CURDIR)/kernel8-debug.img" \
		-drive file="$(CURDIR)/$(BOOT_IMG)",format=raw,if=sd,media=disk

.PHONY: debug
debug: kernel8.img $(BOOT_IMG)
	@command -v qemu-system-aarch64 >/dev/null 2>&1 || { echo "qemu-system-aarch64 not found in PATH"; exit 1; }
	@qemu-system-aarch64 -m 1024 -no-reboot -M raspi3b -serial stdio \
		-kernel "$(CURDIR)/kernel8.img" \
		-drive file="$(CURDIR)/$(BOOT_IMG)",format=raw,if=sd,media=disk \
		-d guest_errors,unimp,int

# Build and run tests in QEMU
.PHONY: test
test: kernel8-test.img $(BOOT_IMG)
	@command -v qemu-system-aarch64 >/dev/null 2>&1 || { echo "qemu-system-aarch64 not found in PATH"; exit 1; }
	@echo "Running PIOS tests..."
	@qemu-system-aarch64 -m 1024 -no-reboot -M raspi3b -serial stdio \
		-kernel "$(CURDIR)/kernel8-test.img" \
		-drive file="$(CURDIR)/$(BOOT_IMG)",format=raw,if=sd,media=disk

# Build and run tests in QEMU with debug output
.PHONY: test-debug
test-debug: kernel8-test.img $(BOOT_IMG)
	@command -v qemu-system-aarch64 >/dev/null 2>&1 || { echo "qemu-system-aarch64 not found in PATH"; exit 1; }
	@echo "Running PIOS tests (debug mode)..."
	@qemu-system-aarch64 -m 1024 -no-reboot -M raspi3b -serial stdio \
		-kernel "$(CURDIR)/kernel8-test.img" \
		-drive file="$(CURDIR)/$(BOOT_IMG)",format=raw,if=sd,media=disk \
		-d guest_errors,unimp,int

# Just build tests without running
.PHONY: build-test
build-test: kernel8-test.img
	@echo "Test kernel built: kernel8-test.img"

# Just build debug kernel without running
.PHONY: build-debug
build-debug: kernel8-debug.img
	@echo "Debug kernel built: kernel8-debug.img (with stack traces enabled)"
