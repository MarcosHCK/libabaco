/* Copyright 2021-2025 MarcosHCK
 * This file is part of libabaco.
 *
 * libabaco is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libabaco is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libabaco.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __LIBABACO_BYTECODE__
#define __LIBABACO_BYTECODE__ 1
#include <stdint.h>

#define PACKED __attribute__ ((packed, aligned (1)))

typedef struct _BHeader BHeader;
typedef struct _BSection BSection;
typedef struct _BNote BNote;
typedef union  _BOpcode BOpcode;

struct _BHeader
{
  union
  {
    uint8_t magic [4];
    uint32_t umagic;
  };

  uint32_t checksum;
  uint32_t sectn;
  uint32_t size;
} PACKED;

struct _BSection
{
  uint16_t name;
  uint8_t type;
  uint8_t flags;
  uint32_t size;
} PACKED;

#define B_SECTION_ALIGN (8)
#define B_HEADER_MAGIC "ABC"

#define b_header_check_magic(header) \
  (G_GNUC_EXTENSION ({ \
    BHeader* __header = (header); \
    const gchar* __magic = (B_HEADER_MAGIC); \
    __header->magic [0] == __magic [0] && \
    __header->magic [1] == __magic [1] && \
    __header->magic [2] == __magic [2] && \
    __header->magic [3] == __magic [3]; \
  }))

typedef enum
{
  B_SECTION_TYPE_BITS = 0,
  B_SECTION_TYPE_STACK,
  B_SECTION_TYPE_STRTAB,
  B_SECTION_TYPE_NOTES,
} BSectionType;

typedef enum
{
  B_SECTION_READ = (1 << 0),
  B_SECTION_WRITE = (1 << 1),
  B_SECTION_EXECUTE = (1 << 2),
  B_SECTION_VIRTUAL = (1 << 3),

  B_SECTION_DATA = B_SECTION_READ,
  B_SECTION_CODE = B_SECTION_READ | B_SECTION_EXECUTE,
  B_SECTION_BSS = B_SECTION_READ | B_SECTION_WRITE | B_SECTION_VIRTUAL,
} BSectionFlags;

struct _BNote
{
  uint16_t key;
  uint16_t value;
} PACKED;

#define B_NOTE_NAMESPACE_SYMBOLS "symbols::"
#define B_NOTE_NAMESPACE_DEBUG "debug::"

/* Bytecode structure heavily based on Lua bytecode */
/*    0   1   2   3   4   5    6   7   8   9  10  11  12  13   14  15  16  17  18  19  20  21  22   23  24  25  26  27  28  29  30  31   */
/* +------------------------+--------------------------------+------------------------------------+------------------------------------+ */
/* | Intruction opcode      | Operand A (always a registry)  | Operand B (always a registry)      | Operand C (always a registry)      | */
/* +------------------------+--------------------------------+------------------------------------+------------------------------------+ */
/* | Intruction opcode      | Operand A (always a registry)  | Operand Bx (absolute offset)                                            | */
/* +------------------------+--------------------------------+-------------------------------------------------------------------------+ */
/* | Intruction opcode      | Operand A (always a registry)  | Operand sBx (relative offset)                                           | */
/* +------------------------+--------------------------------+-------------------------------------------------------------------------+ */

/* As in Lua specifications (thanks guys!)    */
/* - R(X) means Xth register (any of A, B, C) */
/* - K(X) means Xth constant (Bx)             */
/* - F(X) means Xth function (Bx)             */

/* NOP                                                      */
/* MOVE   A B     R(A) := R(B)                              */
/* LOADK  A Bx    R(A) := K(Bx)                             */
/* LOADF  A Bx    R(A) := F(Bx)                             */
/* CALL   A B C   R(A) := R(A)(R(B), R(B+1), ..., R(B+C-1)) */
/* RETURN A       return R(A)                               */

union _BOpcode
{
  uint8_t code : 6;

  struct _BOpcode_ABC
  {
    uint32_t code : 6;
    uint32_t a : 8;
    uint32_t b : 9;
    uint32_t c : 9;
  } PACKED abc;

  struct _BOpcode_ABx
  {
    uint32_t code : 6;
    uint32_t a : 8;
    uint32_t bx : 18;
  } PACKED abx;

  struct _BOpcode_AsBx
  {
    uint32_t code : 6;
    uint32_t a : 8;
    uint32_t sign : 1;
    uint32_t sbx : 17;
  } PACKED asbx;
} PACKED;

typedef enum
{
  B_OPCODE_NOP = 0,
  B_OPCODE_MOVE,
  B_OPCODE_LOADK,
  B_OPCODE_LOADF,
  B_OPCODE_CALL,
  B_OPCODE_RETURN,
  B_OPCODE_MAXOPCODE,
} BOpcodeCode;

#if __cplusplus
extern "C" {
#endif // __cplusplus

#if !defined(MP_EXTERN)
#if defined(_MSC_VER)
#define MP_EXTERN __declspec(dllexport) extern
#elif __GNUC__ >= 4
#define MP_EXTERN __attribute__((visibility("default"))) extern
#else
#define MP_EXTERN extern
#endif
#endif

MP_EXTERN uint32_t _bytecode_checksum (const uint8_t* code, uint32_t size);

#if __cplusplus
}
#endif // __cplusplus

#undef MP_EXTERN
#endif // __LIBABACO_BYTECODE__
