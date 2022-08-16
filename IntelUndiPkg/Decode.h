/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef DECODE_H_
#define DECODE_H_

/* Interface functions declarations */

/** This is the main SW UNDI API entry using the newer NII protocol.

   The parameter passed in is a 64 bit flat model virtual address of the cdb.  We then
   jump into the service routine pointed to by the Api_Table[OpCode].

   @param[in]   Cdb    Pointer to the command descriptor block.

   @retval      None
**/
EFI_STATUS
EFIAPI
IntelgbeUndiApiEntry (
  IN UINT64 Cdb
  );

#endif /* DECODE_H_ */
