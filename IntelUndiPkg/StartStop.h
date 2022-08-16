/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef START_STOP_H_
#define START_STOP_H_

#define EFI_DRIVER_STOP_PROTOCOL_GUID                \
  {                                                  \
    0x34d59603, 0x1428, 0x4429,                      \
    {                                                \
      0xa4, 0x14, 0xe6, 0xb3, 0xb5, 0xfd, 0x7d, 0xc1 \
    }                                                \
  }

typedef struct EFI_DRIVER_STOP_PROTOCOL_S  EFI_DRIVER_STOP_PROTOCOL;

/** Issues a call to stop the driver so diagnostic application can access the hardware.

   @param[in]   This       Pointer to the EFI_DRIVER_STOP_PROTOCOL instance.

   @retval   EFI_SUCCESS   Driver is stopped successfully
**/
typedef
EFI_STATUS
(EFIAPI *EFI_DRIVER_STOP_PROTOCOL_STOP_DRIVER) (
  IN EFI_DRIVER_STOP_PROTOCOL *This
  );

/** Issues a call to start the driver after diagnostic application has completed.

   @param[in]   This       Pointer to the EFI_DRIVER_STOP_PROTOCOL instance.

   @retval   EFI_SUCCESS   If driver has restarted successfully
**/
typedef
EFI_STATUS
(EFIAPI *EFI_DRIVER_STOP_PROTOCOL_START_DRIVER) (
  IN EFI_DRIVER_STOP_PROTOCOL *This
  );

struct EFI_DRIVER_STOP_PROTOCOL_S {
  EFI_DRIVER_STOP_PROTOCOL_STOP_DRIVER StopDriver;
  EFI_DRIVER_STOP_PROTOCOL_START_DRIVER StartDriver;
};

#endif /* START_STOP_H_ */
