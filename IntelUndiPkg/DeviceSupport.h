/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef DEVICE_SUPPORT_H_
#define DEVICE_SUPPORT_H_

#include "Intelgbe.h"

typedef struct BRAND_STRUCT_S BRAND_STRUCT;
extern UINTN mBrandingTableSize;

/* Defines */
#define INVALID_VENDOR_ID     0xFFFF
#define INVALID_SUBVENDOR_ID  0xFFFF
#define INVALID_DEVICE_ID     0xFFFF
#define INVALID_SUBSYSTEM_ID  0xFFFF
#define WILD_CARD             0x0000


/* Types */
struct BRAND_STRUCT_S {
  UINT16  VendorId;
  UINT16  SubvendorId;
  UINT16  DeviceId;
  UINT16  SubsystemId;
  CHAR16 *BrandString;
};

extern BRAND_STRUCT mBrandingTable[];

/* Function declarations */

/** Returns pointer to current device's branding string (looks for best match)

   @param[in]   UndiPrivateData   Points to the driver instance private data

   @return   Pointer to current device's branding string
**/
CHAR16*
GetDeviceBrandingString (
  UNDI_PRIVATE_DATA *UndiPrivateData
  );

/** Returns information whether given device ID is supported basing on branding
   table.

   @param[in]   VendorId      Device's vendor ID
   @param[in]   DeviceId      Device's device ID

   @retval   TRUE    Device ID is supported
   @retval   FALSE   Device ID is not supported
**/
BOOLEAN
IsDeviceIdSupported (
  UINT16 VendorId,
  UINT16 DeviceId
  );
#endif /* DEVICE_SUPPORT_H_ */
