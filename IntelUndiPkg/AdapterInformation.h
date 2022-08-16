/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef ADAPTER_INFORMATION_H_
#define ADAPTER_INFORMATION_H_

#include <Protocol/AdapterInformation.h>

typedef struct UNDI_PRIVATE_DATA_S UNDI_PRIVATE_DATA;


typedef
EFI_STATUS
(* GET_INFORMATION_BLOCK) (
  EFI_ADAPTER_INFORMATION_PROTOCOL *This,
  VOID                             **InformationBlock,
  UINTN                            *InformationBlockSize
  );

typedef
EFI_STATUS
(* SET_INFORMATION_BLOCK) (
  EFI_ADAPTER_INFORMATION_PROTOCOL *This,
  VOID                             *InformationBlock,
  UINTN                            InformationBlockSize
  );

typedef struct {
  EFI_GUID    Guid;

  GET_INFORMATION_BLOCK   GetInformationBlock;
  SET_INFORMATION_BLOCK   SetInformationBlock;
} EFI_ADAPTER_INFORMATION_TYPE_DESCRIPTOR;

#define MAX_SUPPORTED_INFORMATION_TYPE 20

/** Adds supported Information Type Descriptor to the list.
  Call before protocol installation.

  @param[in]   InformationDescriptor  Supported Information Descriptor

  @retval    EFI_SUCCESS             Descriptor added successfully
  @retval    EFI_INVALID_PARAMETER   InformationDescriptor is NULL
  @retval    EFI_OUT_OF_RESOURCES    Existent information count is maximum
  @retval    EFI_ALREADY_STARTED     Specified InformationDescriptor already added
**/
EFI_STATUS
AddSupportedInformationType (
  IN EFI_ADAPTER_INFORMATION_TYPE_DESCRIPTOR  *InformationDescriptor
  );

/** Initializes and installs Adapter Info Protocol on adapter

   @param[in]   UndiPrivateData   Driver private data structure

   @retval    EFI_SUCCESS   Protocol installed successfully
   @retval    !EFI_SUCCESS  Failed to install and initialize protocol
**/
EFI_STATUS
InitAdapterInformationProtocol (
  IN UNDI_PRIVATE_DATA *UndiPrivateData
  );

/** Uninstalls Adapter Info Protocol

   @param[in]   UndiPrivateData   Driver private data structure

   @retval     EFI_SUCCESS    Protocol uninstalled successfully
   @retval     !EFI_SUCCESS   Failed to uninstall protocol
**/
EFI_STATUS
UninstallAdapterInformationProtocol (
  IN UNDI_PRIVATE_DATA *UndiPrivateData
  );

#endif /* ADAPTER_INFORMATION_H_ */
