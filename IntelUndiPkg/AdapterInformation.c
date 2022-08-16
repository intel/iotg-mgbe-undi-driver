/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include "Intelgbe.h"
#include "AdapterInformation.h"

/* Global variables */

EFI_GUID gEfiAdapterInformationProtocolGuid =
EFI_ADAPTER_INFORMATION_PROTOCOL_GUID;

/*Protocol structure tentative definition */
EFI_ADAPTER_INFORMATION_PROTOCOL               gUndiAdapterInfo;
STATIC UINT8                                   mInformationCount;
STATIC EFI_ADAPTER_INFORMATION_TYPE_DESCRIPTOR
mSupportedInformationTypes[MAX_SUPPORTED_INFORMATION_TYPE];

/** Gets media state information block

  @param[in]   This                  Current EFI_ADAPTER_INFORMATION_PROTOCOL instance.
  @param[out]  InformationBlock      Media state information block.
  @param[out]  InformationBlockSize  Media state information block size.

  @retval      EFI_SUCCESS           Information block returned successfully
  @retval      EFI_OUT_OF_RESOURCES  Not enough resources to store media state info
**/
STATIC
EFI_STATUS
GetMediaStateInformationBlock (
  IN  EFI_ADAPTER_INFORMATION_PROTOCOL *This,
  OUT VOID **                           InformationBlock,
  OUT UINTN *                           InformationBlockSize
  )
{
  EFI_ADAPTER_INFO_MEDIA_STATE *Buffer;
  UNDI_PRIVATE_DATA *           UndiPrivateData;

  Buffer = AllocatePool (sizeof (EFI_ADAPTER_INFO_MEDIA_STATE));

  if (Buffer == NULL) {
    DEBUGPRINT (ADAPTERINFO, ("AllocatePool failed\n"));
    return EFI_OUT_OF_RESOURCES;
  }
  UndiPrivateData = UNDI_PRIVATE_DATA_FROM_AIP (This);

  if (IsLinkUp (&UndiPrivateData->NicInfo)) {
    Buffer->MediaState = EFI_SUCCESS;
  } else {
    Buffer->MediaState = EFI_NO_MEDIA;
  }

  *InformationBlock = Buffer;
  *InformationBlockSize = sizeof (EFI_ADAPTER_INFO_MEDIA_STATE);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetIpv6SupportInformationBlock (
  IN  EFI_ADAPTER_INFORMATION_PROTOCOL  *This,
  OUT VOID                              **InformationBlock,
  OUT UINTN                             *InformationBlockSize
)
{
  EFI_ADAPTER_INFO_UNDI_IPV6_SUPPORT  *Buffer;

  Buffer = AllocatePool (sizeof(EFI_ADAPTER_INFO_UNDI_IPV6_SUPPORT));

  if (Buffer == NULL) {
    DEBUGPRINT (ADAPTERINFO, ("AllocatePool failed\n"));
    return EFI_OUT_OF_RESOURCES;
  }
  Buffer->Ipv6Support = TRUE;

  *InformationBlock = Buffer;
  *InformationBlockSize = sizeof (EFI_ADAPTER_INFO_UNDI_IPV6_SUPPORT);

  return EFI_SUCCESS;
}


/** Returns the current state information for the adapter

   @param[in]   This                   Current EFI_ADAPTER_INFORMATION_PROTOCOL instance.
   @param[in]   InformationType        Defines the contents of InformationBlock
   @param[out]  InformationBlock       Ptr to buffer with InformationBlock structure
   @param[out]  InformationBlockSize   Size of the information block in bytes

   @retval    EFI_SUCCESS              InformationBlock successfully returned
   @retval    EFI_INVALID_PARAMETER    One of the parameters is NULL
   @retval    EFI_UNSUPPORTED          GetInformationBlock function is undefined for
                                       specified GUID or GUID is unsupported
**/
STATIC
EFI_STATUS
EFIAPI
GetInformation (
  IN  EFI_ADAPTER_INFORMATION_PROTOCOL *This,
  IN  EFI_GUID *                        InformationType,
  OUT VOID **                           InformationBlock,
  OUT UINTN *                           InformationBlockSize
  )
{
  UINTN i;

  DEBUGPRINT (ADAPTERINFO, ("\n"));

  if (This == NULL) {
    DEBUGPRINT (ADAPTERINFO, ("This is NULL\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (InformationType == NULL) {
    DEBUGPRINT (ADAPTERINFO, ("InformationType is NULL\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (InformationBlock == NULL) {
    DEBUGPRINT (ADAPTERINFO, ("InformationBlock is NULL\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (InformationBlockSize == NULL) {
    DEBUGPRINT (ADAPTERINFO, ("InformationBlockSize is NULL\n"));
    return EFI_INVALID_PARAMETER;
  }

  for (i = 0; i < mInformationCount; ++i) {
    if (CompareGuid (InformationType, &mSupportedInformationTypes[i].Guid)) {
      DEBUGPRINT (ADAPTERINFO, ("Recognized supported GUID\n"));
      if (mSupportedInformationTypes[i].GetInformationBlock) {
        return mSupportedInformationTypes[i].GetInformationBlock (
                                              This,
                                              InformationBlock,
                                              InformationBlockSize
                                            );
      } else {
        return EFI_UNSUPPORTED;
      }
    }
  }

  DEBUGPRINT (ADAPTERINFO, ("Returning EFI_UNSUPPORTED\n"));
  return EFI_UNSUPPORTED;
}

/** Sets state information for an adapter

   @param[in]  This                  Current EFI_ADAPTER_INFORMATION_PROTOCOL instance.
   @param[in]  InformationType       Defines the contents of InformationBlock
   @param[in]  InformationBlock      Ptr to buffer with InformationBlock structure
   @param[in]  InformationBlockSize  Size of the information block in bytes

   @retval    EFI_SUCCESS             InformationBlock successfully set
   @retval    EFI_INVALID_PARAMETER   InformationBlock is NULL
   @retval    EFI_UNSUPPORTED         Specified GUID is unsupported
   @retval    EFI_WRITE_PROTECTED     SetInformationBlock function is undefined for
                                      specified GUID
**/
STATIC
EFI_STATUS
EFIAPI
SetInformation (
  IN  EFI_ADAPTER_INFORMATION_PROTOCOL *This,
  IN  EFI_GUID *                        InformationType,
  IN  VOID *                            InformationBlock,
  IN  UINTN                             InformationBlockSize
  )
{
  UINTN i;

  if (InformationBlock == NULL) {
    DEBUGPRINT (ADAPTERINFO, ("InformationBlock is NULL\n"));
    return EFI_INVALID_PARAMETER;
  }

  for (i = 0; i < mInformationCount; ++i) {
    if (CompareGuid (InformationType, &mSupportedInformationTypes[i].Guid)) {
      DEBUGPRINT (ADAPTERINFO, ("Recognized supported GUID\n"));
      if (mSupportedInformationTypes[i].SetInformationBlock) {
        return mSupportedInformationTypes[i].SetInformationBlock (
                                               This,
                                               InformationBlock,
                                               InformationBlockSize
                                             );
      } else {
        return EFI_WRITE_PROTECTED;
      }
    }
  }
  DEBUGPRINT (ADAPTERINFO, ("Unsupported function called\n"));
  return EFI_UNSUPPORTED;
}

/** Get a list of supported information types for this instance of the protocol

   @param[in]    This             Current EFI_ADAPTER_INFORMATION_PROTOCOL instance.
   @param[out]   InfoTypesBuffer  A pointer to the array of InformationTypeGUIDs that are
                                  supported by This
   @param[out]   InfoTypesBufferCount   Number of GUIDs present in InfoTypesBuffer

   @retval    EFI_SUCCESS             InfoTypesBuffer returned successfully
   @retval    EFI_INVALID_PARAMETER   One of the input parameters is NULL
   @retval    EFI_OUT_OF_RESOURCES    Failed to allocate memory for
                                      InfoTypesBuffer
**/
STATIC
EFI_STATUS
EFIAPI
GetSupportedTypes (
  IN  EFI_ADAPTER_INFORMATION_PROTOCOL *This,
  OUT EFI_GUID **                       InfoTypesBuffer,
  OUT UINTN *                           InfoTypesBufferCount
  )
{
  EFI_GUID *Buffer;
  UINT32    i = 0;

  DEBUGPRINT (ADAPTERINFO, ("%a, %d\n", __FUNCTION__, __LINE__));

  if (This == NULL) {
    DEBUGPRINT (ADAPTERINFO, ("This is NULL\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (InfoTypesBuffer == NULL) {
    DEBUGPRINT (ADAPTERINFO, ("InfoTypesBuffer is NULL\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (InfoTypesBufferCount == NULL) {
    DEBUGPRINT (ADAPTERINFO, ("InfoTypesBufferCount is NULL\n"));
    return EFI_INVALID_PARAMETER;
  }

  Buffer = AllocatePool (mInformationCount * sizeof (EFI_GUID));

  // It is the responsibility of the caller to free this buffer after using it.
  if (Buffer == NULL) {
    DEBUGPRINT (ADAPTERINFO, ("AllocatePool failed\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  for (i = 0; i < mInformationCount; i++) {
    CopyMem (&Buffer[i],
      &mSupportedInformationTypes[i].Guid, sizeof (EFI_GUID));
  }
  *InfoTypesBuffer = Buffer;
  *InfoTypesBufferCount = mInformationCount;

  return EFI_SUCCESS;
}

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
  IN EFI_ADAPTER_INFORMATION_TYPE_DESCRIPTOR *InformationDescriptor
  )
{
  UINT32 i = 0;

  if (InformationDescriptor == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (mInformationCount > MAX_SUPPORTED_INFORMATION_TYPE - 1) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (i = 0; i < mInformationCount; i++)
  {
    if (CompareGuid (
          &InformationDescriptor->Guid,
          &mSupportedInformationTypes[i].Guid
        ))
    {
      return EFI_ALREADY_STARTED;
    }
  }

  CopyMem (
    &mSupportedInformationTypes[mInformationCount].Guid,
    &InformationDescriptor->Guid,
    sizeof (EFI_GUID)
  );
  mSupportedInformationTypes[mInformationCount].GetInformationBlock =
  InformationDescriptor->GetInformationBlock;
  mSupportedInformationTypes[mInformationCount].SetInformationBlock =
  InformationDescriptor->SetInformationBlock;

  mInformationCount++;
  return EFI_SUCCESS;
}

/** Initializes and installs Adapter Info Protocol on adapter

   @param[in]   UndiPrivateData   Driver private data structure

   @retval    EFI_SUCCESS   Protocol installed successfully
   @retval    !EFI_SUCCESS  Failed to install and initialize protocol
**/
EFI_STATUS
InitAdapterInformationProtocol (
  IN UNDI_PRIVATE_DATA *UndiPrivateData
  )
{
  EFI_STATUS                              Status;
  EFI_ADAPTER_INFORMATION_TYPE_DESCRIPTOR InformationType;

  EFI_GUID MediaStateGuid      = EFI_ADAPTER_INFO_MEDIA_STATE_GUID;
  EFI_GUID Ipv6SupportInfoGuid = EFI_ADAPTER_INFO_UNDI_IPV6_SUPPORT_GUID;

  DEBUGPRINT (ADAPTERINFO, ("%a, %d\n", __FUNCTION__, __LINE__));

  UndiPrivateData->AdapterInformation = gUndiAdapterInfo;

  SetMem (&InformationType,
    sizeof (EFI_ADAPTER_INFORMATION_TYPE_DESCRIPTOR), 0);
  CopyMem (&InformationType.Guid, &MediaStateGuid, sizeof (EFI_GUID));
  InformationType.GetInformationBlock = GetMediaStateInformationBlock;
  InformationType.SetInformationBlock = NULL;
  AddSupportedInformationType (&InformationType);

  SetMem (&InformationType,
    sizeof (EFI_ADAPTER_INFORMATION_TYPE_DESCRIPTOR), 0);
  CopyMem (&InformationType.Guid, &Ipv6SupportInfoGuid, sizeof (EFI_GUID));
  InformationType.GetInformationBlock = GetIpv6SupportInformationBlock;
  InformationType.SetInformationBlock = NULL;
  AddSupportedInformationType (&InformationType);


  Status = gBS->InstallProtocolInterface (
                  &UndiPrivateData->DeviceHandle,
                  &gEfiAdapterInformationProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &UndiPrivateData->AdapterInformation
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (ADAPTERINFO,
      ("InstallProtocolInterface returned %r\n", Status));
    return Status;
  }

  return Status;
}

/** Uninstalls Adapter Info Protocol

   @param[in]   UndiPrivateData   Driver private data structure

   @retval     EFI_SUCCESS    Protocol uninstalled successfully
   @retval     !EFI_SUCCESS   Failed to uninstall protocol
**/
EFI_STATUS
UninstallAdapterInformationProtocol (
  IN UNDI_PRIVATE_DATA *UndiPrivateData
  )
{
  EFI_STATUS Status;

  DEBUGPRINT (ADAPTERINFO, ("%a, %d\n", __FUNCTION__, __LINE__));

  Status = gBS->UninstallProtocolInterface (
                  UndiPrivateData->DeviceHandle,
                  &gEfiAdapterInformationProtocolGuid,
                  &UndiPrivateData->AdapterInformation
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (ADAPTERINFO,
      ("UnInstallProtocolInterface returned %r\n", Status));
    return Status;
  }

  return Status;
}

/* Adapter Info Protocol structure definition and initialization */

EFI_ADAPTER_INFORMATION_PROTOCOL gUndiAdapterInfo = {
  GetInformation,
  SetInformation,
  GetSupportedTypes
};
