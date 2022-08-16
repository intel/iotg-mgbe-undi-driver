/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef INIT_H_
#define INIT_H_

#include <Uefi.h>

#include <Base.h>
#include <Guid/EventGroup.h>
#include <Protocol/PciIo.h>
#include <Protocol/NetworkInterfaceIdentifier.h>
#include <Protocol/DevicePath.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/DriverDiagnostics.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/DriverSupportedEfiVersion.h>
#include <Protocol/PlatformToDriverConfiguration.h>
#include <Protocol/FirmwareManagement.h>
#include <Protocol/DriverHealth.h>

#include <Protocol/HiiConfigRouting.h>
#include <Protocol/FormBrowser2.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiString.h>

#include <Guid/MdeModuleHii.h>

#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PrintLib.h>

#include <IndustryStandard/Pci.h>

#define EFI_NII_POINTER_PROTOCOL_GUID \
  { \
    0xE3161450, 0xAD0F, 0x11D9, \
    { \
      0x96, 0x69, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66 \
    } \
  }

/* This is a macro to convert the preprocessor defined version number into a hex value
 that can be registered with EFI. */
#define VERSION_TO_HEX  ((MAJORVERSION << 24) + (MINORVERSION << 16) + \
                        (BUILDNUMBER))

#define MAC_ADDRESS_SIZE_IN_BYTES 6

#endif /* INIT_H_ */
