#pragma once

#include <stddef.h>
#include <stdint.h>


/// Tipos primitivos de largura explícita, no estilo do Rust. Os tipos nativos do C++ variam de
/// tamanho entre plataformas e compiladores (ex.: `long`), então o engine usa só estes.

using i8 = int8_t;

using i16 = int16_t;

using i32 = int32_t;

using i64 = int64_t;

using u8 = uint8_t;

using u16 = uint16_t;

using u32 = uint32_t;

using u64 = uint64_t;


/// Inteiros do tamanho de um ponteiro. Use `isize` para tamanhos e índices.

using isize = ptrdiff_t;

using usize = size_t;

using f32 = float;

using f64 = double;

static_assert(sizeof(i8) == 1 && sizeof(u8) == 1, "i8/u8 devem ter 1 byte");
static_assert(sizeof(i16) == 2 && sizeof(u16) == 2, "i16/u16 devem ter 2 bytes");
static_assert(sizeof(i32) == 4 && sizeof(u32) == 4, "i32/u32 devem ter 4 bytes");
static_assert(sizeof(i64) == 8 && sizeof(u64) == 8, "i64/u64 devem ter 8 bytes");
static_assert(sizeof(isize) == sizeof(void*), "isize deve ter o tamanho de um ponteiro");
static_assert(sizeof(usize) == sizeof(void*), "usize deve ter o tamanho de um ponteiro");
static_assert(sizeof(f32) == 4, "f32 deve ter 4 bytes");
static_assert(sizeof(f64) == 8, "f64 deve ter 8 bytes");
