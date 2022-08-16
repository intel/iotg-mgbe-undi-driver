/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Intelgbe.h"

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
  IN struct intelgbe_hw *Hw,
  IN UINT32           Port
  )
{
  UINT32           Results;
  GIG_DRIVER_DATA *Adapter;
  Adapter = Hw->back;

  if (IsSurpriseRemoval (Adapter)) {
    return INVALID_STATUS_REGISTER_VALUE;
  }

  MemoryFence ();
  Results = (*(UINT32 *)(Hw->hw_addr + Port));
  MemoryFence ();

  return Results;
}

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
  )
{
  UINT32           Value;
  GIG_DRIVER_DATA *Adapter;

  Adapter = Hw->back;
  Value = Data;

  if (IsSurpriseRemoval (Adapter)) {
    return;
  }

  MemoryFence ();
  *((UINT32 *)(Hw->hw_addr + Port)) = Value;
  MemoryFence ();

  return;
}

/** Delay a specified number of microseconds.

   @param[in]   Hw   Pointer to hardware instance.
   @param[in]   usecs   Number of microseconds to delay

   @return   Execution of code delayed
**/
VOID
USecDelay (
  struct intelgbe_hw *Hw,
  UINTN            Usecs
  )
{
  DelayInMicroseconds (Hw->back, Usecs);
}
