/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "DeviceSupport.h"

BRAND_STRUCT mBrandingTable[] = {
    {0x8086, 0x8086, 0x4b32, 0x7270,
      L"Ethernet controller: Intel Corporation Device 4b32"},
    {0x8086, 0x8086, 0x4ba0, 0x7270,
      L"Ethernet controller: Intel Corporation Device 4ba0"},
    {0x8086, 0x8086, 0x4ba1, 0x7270,
      L"Ethernet controller: Intel Corporation Device 4ba1"},
    {0x8086, 0x8086, 0x4ba2, 0x7270,
      L"Ethernet controller: Intel Corporation Device 4ba2"},
    {0x8086, 0x8086, 0x4bb0, 0x7270,
      L"Ethernet controller: Intel Corporation Device 4bb0"},
    {0x8086, 0x8086, 0x4bb1, 0x7270,
      L"Ethernet controller: Intel Corporation Device 4bb1"},
    {0x8086, 0x8086, 0x4bb2, 0x7270,
      L"Ethernet controller: Intel Corporation Device 4bb2"},
    {0x8086, 0x8086, 0x43ac, 0x7270,
      L"Ethernet controller: Intel Corporation Device 43ac"},   // TGL-H TSN 1E.4
    {0x8086, 0x8086, 0x43a2, 0x7270,
      L"Ethernet controller: Intel Corporation Device 43a2"},   // TGL-H TSN 1E.5
    {0x8086, 0x8086, 0xa0ac, 0x7270,
      L"Ethernet controller: Intel Corporation Device a0ac"},   // TGL-U
    {INVALID_VENDOR_ID, INVALID_SUBVENDOR_ID,
      INVALID_DEVICE_ID, INVALID_SUBSYSTEM_ID, L""},
};

UINTN mBrandingTableSize =
(sizeof (mBrandingTable) / sizeof (mBrandingTable[0]));
