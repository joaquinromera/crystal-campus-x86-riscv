CROSS = riscv64-unknown-elf

CC  = $(CROSS)-gcc
CXX = $(CROSS)-g++
AS  = $(CROSS)-as
LD  = $(CROSS)-ld
OBJCOPY = $(CROSS)-objcopy
QEMU ?= qemu-system-riscv64

ARCH_OPTS = -march=rv64imac -mabi=lp64

COMMONFLAGS = -Wall -O2 -ffreestanding -nostdlib -nostartfiles \
  -fno-stack-protector -fno-exceptions -fno-rtti -fno-threadsafe-statics \
  -fno-unwind-tables -fno-asynchronous-unwind-tables -mcmodel=medany $(ARCH_OPTS)

CXXFLAGS = $(COMMONFLAGS)
CFLAGS = $(COMMONFLAGS)
ASFLAGS = $(ARCH_OPTS)
# NO incluir -march / -mabi acá:
LDFLAGS = -T linker.ld -nostdlib

all: build/kernel.elf build/kernel.bin

build:
	mkdir -p build

OBJS = \
  build/machine.o \
  build/entry.o \
  build/trap.o \
  build/uart.o \
  build/screen.o \
  build/state.o \
  build/sched.o \
  build/game.o \
  build/mmu.o \
  build/mem.o \
  build/kernel.o \
  build/user_miner_bin.o \
  build/user_idle_bin.o

build/entry.o: entry.S | build
	$(AS) $(ASFLAGS) $< -o $@

build/machine.o: machine.S | build
	$(AS) $(ASFLAGS) $< -o $@

build/trap.o: trap.S | build
	$(AS) $(ASFLAGS) $< -o $@

build/%.o: %.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/user_entry.o: user/entry.S | build
	$(AS) $(ASFLAGS) $< -o $@

build/user_miner.o: user/miner.c | build
	$(CC) -Wall -O2 -ffreestanding -nostdlib -nostartfiles -fno-stack-protector \
	  -mcmodel=medany $(ARCH_OPTS) -I. -c $< -o $@

build/user_idle.o: user/idle.c | build
	$(CC) -Wall -O2 -ffreestanding -nostdlib -nostartfiles -fno-stack-protector \
	  -mcmodel=medany $(ARCH_OPTS) -I. -c $< -o $@

build/user_miner.elf: build/user_entry.o build/user_miner.o user/linker.ld
	$(LD) -T user/linker.ld -nostdlib build/user_entry.o build/user_miner.o -o $@

build/user_idle.elf: build/user_entry.o build/user_idle.o user/linker.ld
	$(LD) -T user/linker.ld -nostdlib build/user_entry.o build/user_idle.o -o $@

build/user_miner.bin: build/user_miner.elf
	$(OBJCOPY) -O binary $< $@

build/user_idle.bin: build/user_idle.elf
	$(OBJCOPY) -O binary $< $@

build/user_miner_bin.o: build/user_miner.bin
	$(LD) -r -b binary $< -o $@

build/user_idle_bin.o: build/user_idle.bin
	$(LD) -r -b binary $< -o $@

build/kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

build/kernel.bin: build/kernel.elf
	$(OBJCOPY) -O binary $< $@

run: all
	$(QEMU) -machine virt -nographic -monitor none -serial stdio -bios none -kernel build/kernel.elf

clean:
	rm -rf build
