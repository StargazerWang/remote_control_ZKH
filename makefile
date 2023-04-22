
# 配置编译环境
BUILD := build
SRC_C = 
SRC_S = 
INC =

# 选择项目
PROJECT ?= SRC-4526
PROJECT_DIR = ./vendor/827x_ble_remote

# 版本管理
# TAG_COMMIT := $(shell git rev-list --abbrev-commit --tags --max-count=1)
# TAG := $(shell git describe --abbrev=0 --tags ${TAG_COMMIT} 2>/dev/null || true)
TAG := $(shell git describe)
COMMIT := $(shell git rev-parse --short HEAD)
DATE := $(shell git log -1 --format=%cd --date=format:"%Y%m%d")

VERSION ?= $(TAG)
# 主版本号
VERSION_MAJOR := $(word 2,$(subst -, ,$(subst ., ,$(VERSION))))
# 此版本号
VERSION_MINOR := $(word 3,$(subst -, ,$(subst ., ,$(VERSION))))
# build版本: finds the most recent tag that is reachable from a commit.
VERSION_REVISION := $(word 2,$(subst -, ,$(shell git describe)))

$(info $(VERSION_MAJOR))
$(info $(VERSION_MINOR))
$(info $(VERSION_REVISION))


ifeq ($(VERSION),)
    VERSION := $(PROJECT)-$(COMMIT)-$(DATA)
endif

ifneq ($(shell git status --porcelain),)
    VERSION := $(VERSION)-dirty
endif

# 项目特定配置
-include $(PROJECT_DIR)/flags.mk
# 芯片类型
FLAG_MCU_TYPE ?= -DCHIP_TYPE=CHIP_TYPE_8278
# 休眠时RAM保存范围
#MCU_STARTUP_FLAGS := -DMCU_STARTUP_8278
FLAG_MCU_STARTUP ?= -DMCU_STARTUP_8271_RET_32K
# MCU_STARTUP_FLAGS := -DMCU_STARTUP_8278_RET_32K
# MCU_STARTUP_FLAGS := -DMCU_STARTUP_8271_RET_16K
# MCU_STARTUP_FLAGS := -DMCU_STARTUP_8271_RET_32K
# MCU_STARTUP_FLAGS := -DMCU_STARTUP_8271
# 项目特定
FLAG_PROJECT ?= -D__PROJECT_8278_BLE_REMOTE__=1
FLAG_PROJECT += -DVERSION=$(VERSION)
# 链接脚本
PROJECT_LINK ?= boot.link
# LD_FLAG  := boot/boot_32k_retn_8278.link
# LD_FLAG  := boot/boot_32k_retn_8271.link
#   项目源码
SRC_PROJECT ?= $(wildcard $(PROJECT_DIR)/*.c)
#   启动代码
SRC_STARTUP ?= boot/8271/cstartup_8271_RET_32K.S
OBJ_STARTUP ?= $(addprefix $(BUILD)/, $(notdir $(SRC_STARTUP:.S=.o)))
OUT_ELF ?= $(PROJECT)
OUT_BIN ?= $(PROJECT)

# 静态链接库
LIBS ?= -llt_8278 -lfirmware_encrypt

# 项目源码目录
INC  =
SRC_C  = 
INC += -I$(PROJECT_DIR)
SRC_C += $(SRC_PROJECT)

# 头文件
INC += -I"."
INC += -I"./drivers/8278"
INC += -Ivendor/common
INC += -Icommon
INC += -Icommon/config
INC += -Iapplication/app
INC += -Iapplication/audio
INC += -Iapplication/keyboard
INC += -Iapplication/print
INC += -Iapplication/usbstd
INC += -Idrivers/8278
INC += -Idrivers/8278/flash

# 源代码
SRC_C += $(wildcard vendor/common/*.c)
SRC_C += $(wildcard application/app/*.c)
SRC_C += $(wildcard application/audio/*.c)
SRC_C += $(wildcard application/keyboard/*.c)
SRC_C += $(wildcard application/print/*.c)
SRC_C += $(wildcard application/usbstd/*.c)
SRC_C += $(wildcard common/*.c)
SRC_C += $(wildcard drivers/8278/*.c)
SRC_DIR = $(dir $(SRC_C))

# OBJS文件
OBJS = $(addprefix $(BUILD)/, $(notdir $(SRC_C:.c=.o)))

# 编译器标志
CFLAGS = 
CFLAGS += $(INC)
CFLAGS += $(FLAG_MCU_TYPE)
CFLAGS += $(FLAG_MCU_STARTUP)
CFLAGS += $(FLAG_PROJECT)
CFLAGS += -ffunction-sections -fdata-sections -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions

# 设置项目的配置文件
# $(warning PROJECT_DIR = $(PROJECT_DIR))
# $(warning BUILD = $(BUILD))
# $(warning OBJ_STARTUP = $(OBJ_STARTUP))
# $(warning $(shell if [ ! -d "$(PROJECT_DIR)" ]; then echo "1"; else echo ""; fi;))
$(if $(shell if [ ! -d "$(PROJECT_DIR)" ]; then echo "1"; else echo ""; fi;), $(error "project not exist"))
$(shell if [ ! -d "$(BUILD)" ]; then mkdir $(BUILD); fi;)
# $(shell if [ ! -f "$(BUILD)/app_config.h" ]; then cp $(PROJECT_DIR)/app_config.h $(BUILD)/app_config.h; fi;)

# all target
all: $(BUILD)/$(PROJECT).elf secondary-outputs

$(BUILD)/$(PROJECT).elf: $(OBJS) $(BUILD)/div_mod.o $(OBJ_STARTUP) 
	@echo 'Building target: $@'
	@tc32-elf-ld --gc-sections -L"./proj_lib" -L"./application/audio" -T $(PROJECT_LINK) -o"$(BUILD)/$(PROJECT).elf" $(OBJS) $(BUILD)/div_mod.o $(OBJ_STARTUP) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	@$(MAKE) --no-print-directory post-build	

-include $(BUILD)/*.d
vpath %.c $(dir $(SRC_C))
$(BUILD)/%.o: %.c | $(PROJECT_DIR)/app_config.h
	@echo "build $<"
	@tc32-elf-gcc $(CFLAGS) -c -MMD -o"$@" "$<"

$(BUILD)/div_mod.o:./div_mod.S
	@echo 'Building file: $<'
	@tc32-elf-gcc $(CFLAGS) -c -o"$@" "$<"
	@echo 'Finished building: $<'

$(OBJ_STARTUP):$(SRC_STARTUP)
	@echo 'Building file: $<'
	@tc32-elf-gcc $(CFLAGS) -c -o"$@" "$<"
	@echo 'Finished building: $<'

post-build:
	@"./tl_check_fw.sh" $(BUILD)/$(VERSION) $(BUILD)/$(PROJECT)
	@echo ' '

secondary-outputs: $(BUILD)/$(PROJECT).lst  sizedummy

$(BUILD)/$(PROJECT).lst: $(BUILD)/$(PROJECT).elf
	@echo 'Invoking: TC32 Create Extended Listing'
	@tc32-elf-objdump -x -D -l -S $(BUILD)/$(PROJECT).elf  >"$(BUILD)/$(PROJECT).lst"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy:  $(BUILD)/$(PROJECT).elf
	@echo 'Invoking: Print Size'
	tc32-elf-size -t $(BUILD)/$(PROJECT).elf
	@echo 'Finished building: $@'
	@echo ' '

.PHONY: clean reset erase flash
clean:
	@rm -rf $(BUILD)/*

# bdt download tools
# $(BDT_TOOL) 需要在系统设置BDT_TOOL路径

# 检查是否是在wsl环境
kernel_release = $(shell uname -r)
#   :wsl中运行
ifeq ($(findstring microsoft,$(kernel_release)),microsoft)
	is_wsl_host:=1
#   :非wsl中运行
else 
	is_wsl_host:=0
endif


reset:
ifeq ($(is_wsl_host),1)
	@$(BDT_TOOL) 1 8278 rst -f
else
	@$(BDT_TOOL) 8278 rst -f
endif

erase:
ifeq ($(is_wsl_host),1)
# active
	@$(BDT_TOOL) 1 8278 ac
# erase
	@$(BDT_TOOL) 1 8278 wf 0 -s 256k -e
else 
# active
	@$(BDT_TOOL) 8278 activate
# erase
	@$(BDT_TOOL) 8278 wf 0 -s 256k -e
endif

flash:
ifeq ($(is_wsl_host),1)
# active
	@$(BDT_TOOL) 1 8278 ac
# download
	@$(BDT_TOOL) 1 8278 wf 0 -i "$(BUILD)/$(VERSION).bin" 
# reset
	@$(BDT_TOOL) 1 8278 rst -f
else
# active
	@$(BDT_TOOL) 8278 activate
# download
	@$(BDT_TOOL) 8278 wf 0 -i "$(BUILD)/$(VERSION).bin" 
# reset
	@$(BDT_TOOL) 8278 rst -f
endif
