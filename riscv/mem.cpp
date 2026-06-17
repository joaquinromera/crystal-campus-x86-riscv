#include <stddef.h>
#include <stdint.h>

extern "C" void* memset(void* dst, int c, size_t n) {
  uint8_t* p = reinterpret_cast<uint8_t*>(dst);
  for (size_t i = 0; i < n; i++) {
    p[i] = static_cast<uint8_t>(c);
  }
  return dst;
}

extern "C" void* memcpy(void* dst, const void* src, size_t n) {
  uint8_t* d = reinterpret_cast<uint8_t*>(dst);
  const uint8_t* s = reinterpret_cast<const uint8_t*>(src);
  for (size_t i = 0; i < n; i++) {
    d[i] = s[i];
  }
  return dst;
}

