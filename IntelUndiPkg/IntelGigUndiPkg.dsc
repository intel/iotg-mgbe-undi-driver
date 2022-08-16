## @file
#
#  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = IntelGigUndiPkg
  PLATFORM_GUID                  = 248eb597-ad11-4a22-a800-42f0880dcaa4
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

################################################################################
#
# SKU Identification section - list of all SKU IDs supported by this
#                              Platform.
#
################################################################################
[SkuIds]
  0|DEFAULT              # The entry: 0|DEFAULT is reserved and always required.

[LibraryClasses]
 # StdLibC|IntelUndiPkg/LibC/LibC.inf
  DebugLib|MdePkg/Library/UefiDebugLibStdErr/UefiDebugLibStdErr.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  TimerLib|MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFeatureFlag]
  #gEfiMdePkgTokenSpaceGuid.PcdComponentNameDisable|FALSE
  #gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnosticsDisable|FALSE
  #gEfiMdePkgTokenSpaceGuid.PcdComponentName2Disable|FALSE
  #gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnostics2Disable|FALSE
  #gIntelUndiPkgTokenSpaceGuid.PcdSupportScsiPassThru|TRUE
  #gIntelUndiPkgTokenSpaceGuid.PcdSupportExtScsiPassThru|TRUE

[PcdsFixedAtBuild]
  #gEfiMdePkgTokenSpaceGuid.PcdMaximumUnicodeStringLength|0x0
  #gEfiMdePkgTokenSpaceGuid.PcdMaximumAsciiStringLength|0x0
  #gEfiMdePkgTokenSpaceGuid.PcdMaximumLinkedListLength|0x0
  #gEfiMdePkgTokenSpaceGuid.PcdSpinLockTimeout|0x0
  #gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
  #gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000042
  #gEfiMdePkgTokenSpaceGuid.PcdDebugClearMemoryValue|0x0
  #gIntelUndiPkgTokenSpaceGuid.PcdDriverSupportedEfiVersion|0x0002000a # EFI_2_10_SYSTEM_TABLE_REVISION

###################################################################################################
#
# Components Section - list of the modules and components that will be processed by compilation
#                      tools and the EDK II tools to generate PE32/PE32+/Coff image files.
#
# Note: The EDK II DSC file is not used to specify how compiled binary images get placed
#       into firmware volume images. This section is just a list of modules to compile from
#       source into UEFI-compliant binaries.
#       It is the FDF file that contains information on combining binary files into firmware
#       volume images, whose concept is beyond UEFI and is described in PI specification.
#       Binary modules do not need to be listed in this section, as they should be
#       specified in the FDF file. For example: Shell binary (Shell_Full.efi), FAT binary (Fat.efi),
#       Logo (Logo.bmp), and etc.
#       There may also be modules listed in this section that are not required in the FDF file,
#       When a module listed here is excluded from FDF file, then UEFI-compliant binary will be
#       generated for it, but the binary will not be put into any firmware volume.
#
###################################################################################################

[Components]

  IntelUndiPkg/IntelGigUndiDxe.inf
