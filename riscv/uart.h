#pragma once

#include <stddef.h>
#include <stdint.h>

namespace uart {

void putc(char c);
void write(const char* s, size_t len);
void puts(const char* s);

// Devuelve [0..255] cuando hay un byte disponible, o -1 cuando no hay nada para leer
int getc_nonblock();

// Funciones auxiliares mínimas (sin libc)
void write_u32_dec(uint32_t value);
void write_u32_hex(uint32_t value, uint32_t digits);

} // fin del namespace uart

