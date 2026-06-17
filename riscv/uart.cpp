#include "uart.h"

namespace {

constexpr uintptr_t UART0_BASE = 0x10000000UL;

constexpr uintptr_t UART_RHR = UART0_BASE + 0x00; // Registro de recepción (read)
constexpr uintptr_t UART_THR = UART0_BASE + 0x00; // Registro de transmisión (write)
constexpr uintptr_t UART_LSR = UART0_BASE + 0x05; // Registro de estado de línea

constexpr uint8_t LSR_DATA_READY = 0x01;
constexpr uint8_t LSR_THR_EMPTY = 0x20;

inline uint8_t mmio_read8(uintptr_t addr) {
  return *reinterpret_cast<volatile uint8_t*>(addr);
}

inline void mmio_write8(uintptr_t addr, uint8_t value) {
  *reinterpret_cast<volatile uint8_t*>(addr) = value;
}

} // fin del namespace

namespace uart {

void putc(char c) {
  if (c == '\n') {
    putc('\r');
  }

  while ((mmio_read8(UART_LSR) & LSR_THR_EMPTY) == 0) {
  }
  mmio_write8(UART_THR, static_cast<uint8_t>(c));
}

void write(const char* s, size_t len) {
  for (size_t i = 0; i < len; i++) {
    putc(s[i]);
  }
}

void puts(const char* s) {
  if (!s) return;
  for (size_t i = 0; s[i] != '\0'; i++) {
    putc(s[i]);
  }
}

int getc_nonblock() {
  if ((mmio_read8(UART_LSR) & LSR_DATA_READY) == 0) {
    return -1;
  }
  return mmio_read8(UART_RHR);
}

void write_u32_dec(uint32_t value) {
  char buf[10];
  size_t n = 0;

  if (value == 0) {
    putc('0');
    return;
  }

  while (value != 0) {
    uint32_t digit = value % 10;
    buf[n++] = static_cast<char>('0' + digit);
    value /= 10;
  }

  for (size_t i = 0; i < n; i++) {
    putc(buf[n - 1 - i]);
  }
}

void write_u32_hex(uint32_t value, uint32_t digits) {
  static constexpr char kHex[] = "0123456789ABCDEF";
  if (digits == 0) return;
  if (digits > 8) digits = 8;
  for (int i = static_cast<int>(digits) - 1; i >= 0; i--) {
    uint32_t nibble = (value >> (i * 4)) & 0xF;
    putc(kHex[nibble]);
  }
}

} // fin del namespace uart

