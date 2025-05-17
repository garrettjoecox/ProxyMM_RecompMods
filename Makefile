BUILD_DIR := build
MOD_DIR := packages/$(MOD)

ifeq ($(MOD),)
$(error Please specify a mod name using the MOD variable)
endif

# Allow the user to specify the compiler and linker on macOS
# as Apple Clang does not support MIPS architecture
ifeq ($(shell uname),Darwin)
    CC      ?= clang
    LD      ?= ld.lld
else
    CC      := clang
    LD      := ld.lld
endif

TARGET  := $(BUILD_DIR)/$(MOD_DIR)/mod.elf

ifeq ($(OS),Windows_NT)
RECOMP_MOD_TOOL := RecompModTool.exe
else
RECOMP_MOD_TOOL := ./RecompModTool
endif

LDSCRIPT := mod.ld
CFLAGS   := -target mips -mips2 -mabi=32 -O2 -G0 -mno-abicalls -mno-odd-spreg -mno-check-zero-division \
			-fomit-frame-pointer -ffast-math -fno-unsafe-math-optimizations -fno-builtin-memset \
			-Wall -Wextra -Wno-incompatible-library-redeclaration -Wno-unused-parameter -Wno-unknown-pragmas -Wno-unused-variable \
			-Wno-missing-braces -Wno-unsupported-floating-point-opt -Werror=section
CPPFLAGS := -nostdinc -D_LANGUAGE_C -DMIPS -DF3DEX_GBI_2 -DF3DEX_GBI_PL -DGBI_DOWHILE -I include -I include/dummy_headers \
			-I mm-decomp/include -I mm-decomp/src -I mm-decomp/extracted/n64-us -I mm-decomp/include/libc
LDFLAGS  := -nostdlib -T $(LDSCRIPT) -Map $(BUILD_DIR)/$(MOD_DIR)/mod.map --unresolved-symbols=ignore-all --emit-relocs -e 0 --no-nmagic

C_SRCS := $(wildcard $(MOD_DIR)/src/*.c)
C_OBJS := $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.o))
C_DEPS := $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.d))

$(TARGET): $(C_OBJS) $(LDSCRIPT) | $(BUILD_DIR)
	$(LD) $(C_OBJS) $(LDFLAGS) -o $@
	$(MAKE) post-build

$(BUILD_DIR) $(BUILD_DIR)/$(MOD_DIR)/src:
ifeq ($(OS),Windows_NT)
	mkdir $(subst /,\,$@)
else
	mkdir -p $@
endif

$(C_OBJS): $(BUILD_DIR)/%.o : %.c | $(BUILD_DIR) $(BUILD_DIR)/$(MOD_DIR)/src
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -MMD -MF $(@:.o=.d) -c -o $@

no-post-build: $(TARGET)
	$(RECOMP_MOD_TOOL) $(MOD_DIR)/mod.toml $(BUILD_DIR)/$(MOD_DIR)
	
post-build: $(TARGET)
	$(RECOMP_MOD_TOOL) $(MOD_DIR)/mod.toml $(BUILD_DIR)/$(MOD_DIR) && mkdir -p ~/.config/Zelda64Recompiled/mods && cp $(BUILD_DIR)/$(MOD_DIR)/*.nrm ~/.config/Zelda64Recompiled/mods

clean:
	rm -rf $(BUILD_DIR)

-include $(C_DEPS)

.PHONY: clean
