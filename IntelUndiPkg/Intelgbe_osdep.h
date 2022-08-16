/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef INTELGBE_OSDEP_H_
#define INTELGBE_OSDEP_H_

#include <Uefi.h>
#include <Base.h>
//#include <Library\BaseLib.h>
//#include <Library\BaseMemoryLib.h>

//#pragma warning(disable : 4244)
//#pragma warning(disable : 4206)

#ifndef EFI_SPECIFICATION_VERSION
#define EFI_SPECIFICATION_VERSION 0x00020000
#endif /* EFI_SPECIFICATION_VERSION */

#ifndef TIANO_RELEASE_VERSION
#define TIANO_RELEASE_VERSION     0x00080005
#endif /* TIANO_RELEASE_VERSION */

#ifdef EFI32

// Remove truncation warning in type cast when some 64 bit variables are converted to 32-bit pointers
#pragma warning(disable : 4305)
#endif /* EFI32 */

//#define STATIC static

#define CHAR            CHAR8
#define memcmp          CompareMem
#define memcpy          CopyMem
#define strlen          AsciiStrLen
#define NalMemoryCopy   CopyMem

#define int32_t  INT32;
#define uint32_t UINT32;
#define int16_t  INT16;
#define uint16_t UINT16;

#define __le64 UINT64
#define u64 UINT64
#define s64 INT64
#define __le32 UINT32
#define u32 UINT32
#define s32 INT32
#define __le16 UINT16
#define u16 UINT16
#define s16 INT16
#define u8  UINT8
#define s8  INT8
#define bool BOOLEAN

#define true 1
#define false 0

struct intelgbe_hw;

/** This function calls the MemIo callback to read a dword from the device's
   address space

   Since UNDI3.0 uses the TmpMemIo function (instead of the callback routine)
   which also takes the UniqueId parameter (as in UNDI3.1 spec) we don't have
   to make undi3.0 a special case

   @param[in]   Hw     Pointer to hardware instance.
   @param[in]   Port   Which Port to read from.

   @return   Results - The data read from the port.
**/
UINT32
INTELGBEInDword (
  struct intelgbe_hw *Hw,
  UINT32           Port
  );

/** This function calls the MemIo callback to write a word from the device's
   address space

   Since UNDI3.0 uses the TmpMemIo function (instead of the callback routine)
   which also takes the UniqueId parameter (as in UNDI3.1 spec) we don't have
   to make undi3.0 a special case

   @param[in]   Hw   Pointer to hardware instance.
   @param[in]   Port   Which port to write to.
   @param[in]   Data   Data to write to Port.

   @return   Word written
**/
VOID
INTELGBEOutDword (
  IN struct intelgbe_hw *Hw,
  IN UINT32           Port,
  IN UINT32           Data
  );

/** Delay a specified number of microseconds.

   @param[in]   Hw   Pointer to hardware instance.
   @param[in]   usecs   Number of microseconds to delay

   @return   Execution of code delayed
**/
VOID
USecDelay (
  struct intelgbe_hw *Hw,
  UINTN            Usecs
  );

/** Wrapper macro for shared code usec_delay statement
   with USecDelay function

   @param[in]   x   Time to wait in microseconds

   @return   USecDelay called
**/
#define usec_delay(x)     USecDelay (hw, x)

/** Wrapper macro for shared code msec_delay statement
   with USecDelay function

   @param[in]   x   Time to wait in milliseconds

   @return   USecDelay called
**/
#define msec_delay(x)     USecDelay (hw, x * 1000)

/** Wrapper macro for shared code usec_delay_irq statement
   with USecDelay function

   @param[in]   x   Time to wait in microseconds

   @return   USecDelay called
**/
#define usec_delay_irq(x) USecDelay (hw, x)

/** Wrapper macro for shared code msec_delay_irq statement
   with USecDelay function

   @param[in]   x   Time to wait in milliseconds

   @return   USecDelay called
**/
#define msec_delay_irq(x) USecDelay (hw, x * 1000)

/** Shared code uses memset(), this macro wraps SetMem to fullfill this need

   @param[in]    Buffer         Buffer to set its contents
   @param[in]    BufferLength   Length of the buffer
   @param[in]    Value          Value to set buffer contents to

   @return   Buffer contents set to Value
**/
#define memset(Buffer, Value, BufferLength) SetMem (Buffer, BufferLength, Value)

#define CMD_MEM_WRT_INVALIDATE EFI_PCI_COMMAND_MEMORY_WRITE_AND_INVALIDATE

typedef BOOLEAN boolean_t;


#if (1)
#define DEBUGFUNC(F)
#define DEBUGOUT(s) Aprint (s);
#define DEBUGOUT1(s, a) Aprint (s, a);
#define DEBUGOUT2(s, a, b) Aprint (s, a, b);
#define DEBUGOUT3(s, a, b, c) Aprint (s, a, b, c);
#define DEBUGOUT7(s, a, b, c, d, e, f, g) Aprint (s, a, b, c, d, e, f, g);
#else /* NOT (0) */

/** Macro wrapper for shared code, blank here

   @param[in]   F    String to display

   @retval  None
**/
#define DEBUGFUNC(F)

/** Macro wrapper for shared code DEBUGOUT statement
   with UNREFERENCED_XPARAMETER (resulting in DEBUGOUT being unused)

   @param[in]   s    String to display

   @retval   None
**/
#define DEBUGOUT(s) \
  do { \
  } while (0)

/** Macro wrapper for shared code DEBUGOUT1 statement
   with UNREFERENCED_1PARAMETER (resulting in DEBUGOUT being unused)

   @param[in]   s    String to display
   @param[in]   a    Value to include in string

   @retval   None
**/
#define DEBUGOUT1(s, a) \
  do { \
    UNREFERENCED_1PARAMETER (a); \
  } while (0)

/** Macro wrapper for shared code DEBUGOUT2 statement
   with UNREFERENCED_2PARAMETER (resulting in DEBUGOUT being unused)

   @param[in]   s    String to display
   @param[in]   a    Value to include in string
   @param[in]   b    Value to include in string

   @retval   None
**/
#define DEBUGOUT2(s, a, b) \
  do { \
    UNREFERENCED_2PARAMETER (a, b); \
  } while (0)

/** Macro wrapper for shared code DEBUGOUT3 statement
   with UNREFERENCED_3PARAMETER (resulting in DEBUGOUT being unused)

   @param[in]   s    String to display
   @param[in]   a    Value to include in string
   @param[in]   b    Value to include in string
   @param[in]   c    Value to include in string

   @retval   None
**/
#define DEBUGOUT3(s, a, b, c) \
  do { \
    UNREFERENCED_3PARAMETER (a, b, c); \
  } while (0)

/** Macro wrapper for shared code DEBUGOUT7 statement,
   blank here

   @param[in]   s    String to display
   @param[in]   a    Value to include in string
   @param[in]   b    Value to include in string
   @param[in]   c    Value to include in string
   @param[in]   d    Value to include in string
   @param[in]   e    Value to include in string
   @param[in]   f    Value to include in string
   @param[in]   g    Value to include in string

   @retval   None
**/
#define DEBUGOUT7(s, a, b, c, d, e, f, g)
#endif /* (0) */

/** INTELGBE_WRITE_REG wrapper macro for shared code

   @param[in]   a        Pointer to hardware instance.
   @param[in]   Reg      Which port to write to.
   @param[in]   Value    Data to write to Port.

   @return   INTELGBEOutDword called
**/
#define INTELGBE_WRITE_REG(a, Reg, Value) \
  INTELGBEOutDword (a, (UINT32) (Reg), Value)

/** INTELGBE_READ_REG wrapper macro for shared code

   @param[in]   a        Pointer to hardware instance.
   @param[in]   Reg      Which port to read from.

   @return   INTELGBEInDword called
**/
#define INTELGBE_READ_REG(a, Reg)  \
  INTELGBEInDword (a, (UINT32) (Reg)) \

/** INTELGBE_READ_REG_ARRAY wrapper macro for shared code

   @param[in]   a        Pointer to hardware instance.
   @param[in]   Reg      Which port to read from.
   @param[in]   Offset   Offset from Reg.

   @return   INTELGBEInDword called
**/
#define INTELGBE_READ_REG_ARRAY(a, Reg, Offset)  \
  INTELGBEInDword (a, (UINT32) (Reg + ((Offset) << 2)))

#endif /* INTELGBE_OSDEP_H_ */
