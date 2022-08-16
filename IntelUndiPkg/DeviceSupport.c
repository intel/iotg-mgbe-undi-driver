/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "DeviceSupport.h"

/** Seeks for current device's entry in branding table

   @param[in]   VendorId      Device's vendor ID
   @param[in]   DeviceId      Device's device ID
   @param[in]   SubvendorId   Device's subvendor ID
   @param[in]   SubdeviceId   Device's subdevice ID
   @param[in]   ExactMatch    Indicator whether full 4-part device ID match is expected.
                              If FALSE, function returns best matching device's info.

   @return   Device info structure pointer when match was found, NULL otherwise.
**/
BRAND_STRUCT*
FindDeviceInTableByIds (
  UINT16  VendorId,
  UINT16  DeviceId,
  UINT16  SubvendorId,
  UINT16  SubdeviceId,
  BOOLEAN ExactMatch
  )
{
  UINTN         i;
  BRAND_STRUCT *Device = NULL;

  if (ExactMatch) {
    for (i = 0; i < mBrandingTableSize; i++) {
      if ((VendorId == mBrandingTable[i].VendorId) &&
        (DeviceId == mBrandingTable[i].DeviceId) &&
        (SubvendorId == mBrandingTable[i].SubvendorId) &&
        (SubdeviceId == mBrandingTable[i].SubsystemId))
      {
        Device = &mBrandingTable[i];
        break;
      }
    }
  } else {
    INTN SubsystemMatch  = -1;
    INTN SubvendorMatch  = -1;
    INTN DeviceMatch = -1;
    INTN VendorMatch = -1;

    for (i = 0; i < mBrandingTableSize; i++) {
      if (VendorId == mBrandingTable[i].VendorId) {
        if (DeviceId == mBrandingTable[i].DeviceId) {
          if (SubvendorId == mBrandingTable[i].SubvendorId) {
            if (SubdeviceId == mBrandingTable[i].SubsystemId) {
              SubsystemMatch = i;
              break;
            } else if (mBrandingTable[i].SubsystemId == WILD_CARD) {
              SubvendorMatch = i;
            }
          } else if (mBrandingTable[i].SubvendorId == WILD_CARD) {
            DeviceMatch = i;
          }
        } else if (mBrandingTable[i].DeviceId == WILD_CARD) {
          VendorMatch = i;
        }
      }
    }
    do {
      if (SubsystemMatch != -1) {
        Device = &mBrandingTable[SubsystemMatch];
        break;
      }
      if (SubvendorMatch != -1) {
        Device = &mBrandingTable[SubvendorMatch];
        break;
      }
      if (DeviceMatch != -1) {
        Device = &mBrandingTable[DeviceMatch];
        break;
      }
      if (VendorMatch != -1) {
        Device = &mBrandingTable[VendorMatch];
        break;
      }
    } while (0);

  }
  return Device;
}

/** Seeks for current device's entry in branding table

   @param[in]   UndiPrivateData   Driver private data structure
   @param[in]   ExactMatch        Indicator whether full 4-part device ID match is expected.
                                  If FALSE, function returns best matching device's info.

   @return   Device info structure pointer when match was found, NULL otherwise.
**/
BRAND_STRUCT*
FindDeviceInTable (
  UNDI_PRIVATE_DATA *UndiPrivateData,
  BOOLEAN            ExactMatch
  )
{
  struct intelgbe_hw *Nic = &UndiPrivateData->NicInfo.Hw;

  return FindDeviceInTableByIds (
           Nic->vendor_id,
           Nic->device_id,
           Nic->subsystem_vendor_id,
           Nic->subsystem_device_id,
           ExactMatch
         );
}


/** Returns pointer to current device's branding string (looks for best match)

   @param[in]   UndiPrivateData   Points to the driver instance private data

   @return   Pointer to current device's branding string
**/
CHAR16*
GetDeviceBrandingString (
  UNDI_PRIVATE_DATA *UndiPrivateData
  )
{
  BRAND_STRUCT *Device = NULL;
  CHAR16 *      BrandingString = NULL;


  // It will return last, INVALID entry at least
  Device = FindDeviceInTable (UndiPrivateData, FALSE);
  if (Device != NULL) {
    BrandingString = Device->BrandString;
  }

  return BrandingString;
}

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
  )
{
  UINTN i;

  if (VendorId == INTEL_VENDOR_ID) {
    for (i = 0; i < mBrandingTableSize; i++) {
      if (mBrandingTable[i].DeviceId != INVALID_DEVICE_ID) {
        if (DeviceId == mBrandingTable[i].DeviceId) {
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}
