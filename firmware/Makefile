PROJECT       := qt_stm32_control
BUILD_DIR     := build

TOOL_DIR      ?= C:/arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi

# STM32CubeProgrammer install directory. The installer offers two locations;
# override on the command line if yours differs, e.g.
#   make flash CUBE_DIR="C:/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin"
CUBE_DIR      ?= C:/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin
CROSS_COMPILE ?= $(TOOL_DIR)/bin/arm-none-eabi-
PROGRAMMER    ?= $(CUBE_DIR)/STM32_Programmer_CLI.exe

CC            := "$(CROSS_COMPILE)gcc"
OBJCOPY       := "$(CROSS_COMPILE)objcopy"
OBJDUMP       := "$(CROSS_COMPILE)objdump"
SIZE          := "$(CROSS_COMPILE)size"

LDSCRIPT      := rom_0x08000000.lds
CPU_FLAGS     := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CPPFLAGS      := -DSTM32F411xE -I.
CFLAGS        := $(CPU_FLAGS) -std=gnu99 -O2 -g3 -Wall -Wextra \
                 -ffunction-sections -fdata-sections -fno-common \
                 -funsigned-char -fno-strict-aliasing -MMD -MP
LDFLAGS       := $(CPU_FLAGS) --specs=nano.specs --specs=nosys.specs \
                 -nostartfiles -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map \
                 -Wl,--cref -T$(LDSCRIPT)

C_SOURCES     := main.c device.c clock.c system_stm32f4xx.c exception.c \
                 UART2_Controller.c LED_Controller.c Motor_Controller.c \
                 US_Controller.c Timebase.c Handler.c state_manager.c protocol.c
ASM_SOURCES   := crt0.s
OBJECTS       := $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o)) \
                 $(addprefix $(BUILD_DIR)/,$(ASM_SOURCES:.s=.o))
DEPS          := $(OBJECTS:.o=.d)

.PHONY: all clean flash run size

all: $(BUILD_DIR)/$(PROJECT).bin

$(BUILD_DIR):
	@powershell.exe -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$@' | Out-Null"

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CPU_FLAGS) -x assembler-with-cpp -c $< -o $@

$(BUILD_DIR)/$(PROJECT).elf: $(OBJECTS) $(LDSCRIPT)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf
	$(OBJCOPY) -O binary $< $@
	$(OBJDUMP) -x -D -S $< > $(BUILD_DIR)/$(PROJECT).dmp

size: $(BUILD_DIR)/$(PROJECT).elf
	$(SIZE) $<

flash: $(BUILD_DIR)/$(PROJECT).elf
	"$(PROGRAMMER)" -c port=SWD -w "$<" -v -rst -q

# Bare-metal firmware cannot run on the host. Flash it and reset the target MCU.
run: flash

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)
