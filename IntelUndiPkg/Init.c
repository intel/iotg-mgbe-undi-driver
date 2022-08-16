/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Init.h"
#include "Intelgbe.h"
#include "DeviceSupport.h"
#include "Decode.h"

/* Global Variables */
VOID *             mIntelgbePxeMemPtr = NULL;
PXE_SW_UNDI *      mIntelgbePxe31 = NULL;  // 3.1 entry
EFI_DRIVER_BINDING_PROTOCOL gUndiDriverBinding;
EFI_SYSTEM_TABLE * gSystemTable;
UNDI_PRIVATE_DATA *mIntelgbeUndi32DeviceList[MAX_NIC_INTERFACES];
UINT8              mActiveControllers = 0;
UINT16             mActiveChildren    = 0;
EFI_GUID gEfiNiiPointerGuid = EFI_NII_POINTER_PROTOCOL_GUID;

/** Checks if remaining device path is NULL or end of device path

   @param[in]   RemainingDevicePath   Device Path

   @retval   TRUE   RemainingDevicePath is NULL or end of device path
**/
BOOLEAN
IsNullOrEndOfDevicePath (
  IN VOID *RemainingDevicePath
  )
{
  if ((RemainingDevicePath == NULL)
    || (IsDevicePathEnd (RemainingDevicePath))) {
    return TRUE;
  }
  return FALSE;
}

/** Gets controller private data structure

   @param[in]  ControllerHandle     Controller handle

   @return     UNDI_PRIVATE_DATA    Pointer to Private Data Structure.
   @return     NULL                 Controller is not initialized
**/
UNDI_PRIVATE_DATA*
GetControllerPrivateData (
  IN  EFI_HANDLE ControllerHandle
  )
{
  UINT32 i = 0;

  for (i = 0; i < mActiveControllers; i++) {
    if (mIntelgbeUndi32DeviceList[i] != NULL) {
      if (mIntelgbeUndi32DeviceList[i]->ControllerHandle == ControllerHandle) {
        return mIntelgbeUndi32DeviceList[i];
      }
    }
  }
  return NULL;
}

/** Checks if device path type is supported by the driver

   @param[in]  RemainingDevicePath  Device Path

   @retval     TRUE                 Device path type supported by the driver
   @retval     FALSE                Device path type not supported by the driver
**/
BOOLEAN
IsDevicePathTypeSupported (
  IN VOID *RemainingDevicePath
  )
{
  UINT8                 PathType;
  UINT8                 PathSubType;
  MAC_ADDR_DEVICE_PATH *MacDevPath;

  if (RemainingDevicePath == NULL) {
    return FALSE;
  }

  PathType    = DevicePathType (RemainingDevicePath);
  PathSubType = DevicePathSubType (RemainingDevicePath);

  if ((PathType == MESSAGING_DEVICE_PATH)
    && (PathSubType == MSG_MAC_ADDR_DP))
  {
    MacDevPath = RemainingDevicePath;
    if (MacDevPath->IfType == PXE_IFTYPE_ETHERNET) {
      return TRUE;
    }
  }
  return FALSE;
}

/** Checks if device path is supported by the driver

   @param[in]       RemainingDevicePath  Device Path
   @param[in]       MacAddr              MAC Address

   @retval          TRUE                 Device path supported by the driver
   @retval          FALSE                Device path not supported by the driver
**/
BOOLEAN
IsDevicePathSupported (
  IN VOID * RemainingDevicePath,
  IN UINT8 *MacAddr
  )
{
  MAC_ADDR_DEVICE_PATH *MacDevPath;
  UINT8                 Index;

  if (RemainingDevicePath == NULL
    || MacAddr == NULL)
  {
    return FALSE;
  }

  if (IsDevicePathTypeSupported (RemainingDevicePath)) {
    MacDevPath = RemainingDevicePath;
    for (Index = 0; Index < MAC_ADDRESS_SIZE_IN_BYTES; Index++) {
      if (MacDevPath->MacAddress.Addr[Index] != MacAddr[Index]) {
        return FALSE;
      }
    }
    return TRUE;
  }
  return FALSE;
}

/** Analyzes Remaining Device Path.

   @param[in]       UndiPrivateData        Driver private data structure
   @param[in]       RemainingDevicePath    Device Path

   @retval          EFI_SUCCESS            Device supported by the driver
   @retval          EFI_ALREADY_STARTED    Device already managed by the driver
   @retval          EFI_UNSUPPORTED        Device not supported by the driver
**/
EFI_STATUS
AnalyzeRemainingDevicePath (
  IN UNDI_PRIVATE_DATA *UndiPrivateData,
  IN VOID              *RemainingDevicePath
  )
{
  if (UndiPrivateData == NULL) {
    if (IsNullOrEndOfDevicePath (RemainingDevicePath)) {
      return EFI_SUCCESS;
    }
    if (IsDevicePathTypeSupported (RemainingDevicePath)) {
      return EFI_SUCCESS;
    }
  }

  if (UndiPrivateData != NULL) {
    if (IsNullOrEndOfDevicePath (RemainingDevicePath)) {
      if (UndiPrivateData->IsChildInitialized) {
        return EFI_ALREADY_STARTED;
      } else {
        return EFI_SUCCESS;
      }
    }
    if (IsDevicePathSupported (RemainingDevicePath,
      UndiPrivateData->NicInfo.Hw.mac.addr)) {
      if (UndiPrivateData->IsChildInitialized) {
        return EFI_ALREADY_STARTED;
      } else {
        return EFI_SUCCESS;
      }
    }
  }
  return EFI_UNSUPPORTED;
}

/** Checks if device path is not end of device path

   @param[in]  RemainingDevicePath  Device Path

   @retval     TRUE                 Device path is not end of device path
   @retval     FALSE                Device path is end of device path
**/
BOOLEAN
IsNotEndOfDevicePathNode (
  IN VOID *RemainingDevicePath
  )
{
  return !(IsDevicePathEnd (RemainingDevicePath));
}

/** Test to see if this driver supports ControllerHandle.

   Any ControllerHandle than contains a  DevicePath, PciIo protocol,
   Class code of 2, Vendor ID of 0x8086, and DeviceId matching an Intel
   adapter can be supported.

   @param[in]   This                  Protocol instance pointer.
   @param[in]   Controller            Handle of device to test.
   @param[in]   RemainingDevicePath   Remaining part of device path.

   @retval   EFI_SUCCESS          This driver supports this device.
   @retval   EFI_UNSUPPORTED      This driver does not support this device
   @retval   EFI_ALREADY_STARTED  Device already managed by the driver
   @retval   !EFI_SUCCESS         Opening PciIo, or Pci.Read failed
**/
EFI_STATUS
EFIAPI
IntelgbeUndiDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL *   RemainingDevicePath
  )
{
  EFI_STATUS           Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00           Pci;
  UNDI_PRIVATE_DATA *UndiPrivateData;
  UndiPrivateData = GetControllerPrivateData (Controller);

  if (UndiPrivateData == NULL) {
    Status = gBS->OpenProtocol (
                    Controller,
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &PciIo,
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

    if (EFI_ERROR (Status)) {
      return Status;
    }
  } else {
    PciIo = UndiPrivateData->NicInfo.PciIo;
    if (PciIo == NULL) {
      return EFI_INVALID_PARAMETER;
    }
  }

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        0,
                        sizeof (PCI_CONFIG_HEADER),
                        &Pci
                      );
  if (EFI_ERROR (Status)) {
    goto ExitSupported;
  }

  if (!IsDeviceIdSupported (Pci.Hdr.VendorId, Pci.Hdr.DeviceId)) {
    Status = EFI_UNSUPPORTED;
    goto ExitSupported;
  }
  Status = AnalyzeRemainingDevicePath (
             UndiPrivateData,
             RemainingDevicePath
           );
  if (EFI_ERROR (Status)) {
    goto ExitSupported;
  }
ExitSupported:
  if (UndiPrivateData == NULL) {
    gBS->CloseProtocol (
           Controller,
           &gEfiPciIoProtocolGuid,
           This->DriverBindingHandle,
           Controller
         );
  }

  return Status;
}

/** Closes controller protocols

   @param[in]       Controller             Controller handle
   @param[in]       This                   Driver Binding protocol instance

   @retval          EFI_SUCCESS            Procedure returned successfully
   @retval          EFI_INVALID_PARAMETER  Invalid parameter passed
**/
EFI_STATUS
CloseControllerProtocols (
  IN EFI_HANDLE                   Controller,
  IN EFI_DRIVER_BINDING_PROTOCOL *This
  )
{
  EFI_STATUS Status;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->CloseProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  This->DriverBindingHandle,
                  Controller
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL,
      ("CloseProtocol gEfiDevicePathProtocolGuid returned %r\n", Status));
    return Status;
  }

  Status = gBS->CloseProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  This->DriverBindingHandle,
                  Controller
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL,
      ("CloseProtocol gEfiPciIoProtocolGuid returned %r\n", Status));
    return Status;
  }
  return EFI_SUCCESS;
}

/** Opens controller protocols

   @param[in]       Controller             Controller handle
   @param[in]       This                   Driver Binding Protocol instance
   @param[in]       UndiPrivateData        Driver private data

   @retval          EFI_SUCCESS            Procedure returned successfully
   @retval          EFI_INVALID_PARAMETER  Invalid parameter passed
**/
EFI_STATUS
OpenControllerProtocols (
  IN  EFI_HANDLE                   Controller,
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  UNDI_PRIVATE_DATA *          UndiPrivateData
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL *DevicePath;

  if (This == NULL
    || UndiPrivateData == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &UndiPrivateData->NicInfo.PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("OpenProtocol returns %r\n", Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("OpenProtocol returns %r\n", Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }
  UndiPrivateData->Undi32BaseDevPath = DevicePath;

  return EFI_SUCCESS;
}

/** This does an 8 bit check sum of the passed in buffer for Len bytes.
   This is primarily used to update the check sum in the SW UNDI header.

   @param[in]   Buffer   Pointer to the passed in buffer to check sum
   @param[in]   Len   Length of buffer to be check summed in bytes.

   @return   The 8-bit checksum of the array pointed to by Buffer.
**/
UINT8
IntelgbeChkSum (
  IN VOID * Buffer,
  IN UINT16 Len
  )
{
  UINT8 Chksum;
  INT8 *Bp;

  Chksum = 0;

  if ((Bp = Buffer) != NULL) {
    while (Len--) {
      Chksum = (UINT8) (Chksum + *Bp++);
    }
  }

  return Chksum;
}

/** Updates active children number and PXE structure on child stop/init

   When called with a null NicPtr, this routine decrements the number of NICs
   this UNDI is supporting and removes the NIC_DATA_POINTER from the array.
   Otherwise, it increments the number of NICs this UNDI is supported and
   updates the PXE. Fudge to ensure a proper check sum results.

   @param[in]   NicPtr   Pointer to the NIC data structure information which the
                         UNDI driver is layering on..
   @param[in]   PxePtr   Pointer to the PXE structure

   @retval   EFI_SUCCESS           PxeStruct updated successful.
   @retval   EFI_OUT_OF_RESOURCES  Too many NIC (child) interfaces.
**/
EFI_STATUS
GigUndiPxeUpdate (
  IN GIG_DRIVER_DATA *NicPtr,
  IN PXE_SW_UNDI     *PxePtr
  )
{
  if (NicPtr == NULL) {

    // IFcnt is equal to the number of NICs this UNDI supports - 1
    if (mActiveChildren > 0) {
      mActiveChildren--;
    }
  }
  else {
    if (mActiveChildren < MAX_NIC_INTERFACES) {
      mActiveChildren++;
    } else {
      return EFI_OUT_OF_RESOURCES;
    }
  }

  // number of NICs this UNDI supports
  PxePtr->IFcnt = (UINT8) mActiveChildren - 1;
  PxePtr->Fudge = (UINT8)(PxePtr->Fudge - IntelgbeChkSum ((VOID *) PxePtr, PxePtr->Len));
  DEBUGPRINT (INIT,
    ("GigUndiPxeUpdate: ActiveChildren = %d\n", mActiveChildren));
  DEBUGPRINT (INIT, ("GigUndiPxeUpdate: PxePtr->IFcnt = %d\n", PxePtr->IFcnt));
  return EFI_SUCCESS;
}

/** Initializes Undi Callback functions in Adapter structure.

    @param[out]      NicInfo    Adapter Structure which shall be initialized

    @return          NicInfo    Initialized adapter structure
**/
VOID
InitUndiCallbackFunctions (
  OUT GIG_DRIVER_DATA *NicInfo
  )
{
  // Initialize the UNDI callback functions to 0 so that the default boot services
  // callback is used instead of the SNP callback.
  NicInfo->Delay       = (VOID *) 0;
  NicInfo->Virt2Phys   = (VOID *) 0;
  NicInfo->Block       = (VOID *) 0;
  NicInfo->MapMem      = (VOID *) 0;
  NicInfo->UnMapMem    = (VOID *) 0;
  NicInfo->SyncMem     = (VOID *) 0;
  NicInfo->UniqueId    = (UINT64) NicInfo;
  NicInfo->VersionFlag = 0x31;
}

/** Initializes UNDI (PXE) structures

   @param[in]       UndiPrivateData      Private data structure

   @retval          EFI_SUCCESS          Undi structure initialized correctly.
   @retval          EFI_OUT_OF_RESOURCES Too many NIC (child) interfaces.
**/
EFI_STATUS
InitUndiStructures (
  IN UNDI_PRIVATE_DATA *UndiPrivateData
  )
{
  EFI_STATUS Status;
  // the IfNum index for the current interface will be the total number
  // of interfaces initialized so far
  Status = GigUndiPxeUpdate (&UndiPrivateData->NicInfo, mIntelgbePxe31);
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("GigUndiPxeUpdate returns %r\n", Status));
    return Status;
  }
  InitUndiCallbackFunctions (&UndiPrivateData->NicInfo);
  return EFI_SUCCESS;
}

/** Initializes Network Interface Identifier Pointer Protocol

   @param[in]       Handle              Controller/Child handle
   @param[in]       NiiProtocol31      NII Protocol instance
   @param[out]      NIIPointerProtocol  NII Pointer Protocol instance

   @retval          EFI_SUCCESS            Procedure returned successfully
   @retval          EFI_INVALID_PARAMETER  Invalid parameter passed
   @retval          !EFI_SUCCESS           Failed to initialize NII Pointer Protocol
**/
EFI_STATUS
InitNiiPointerProtocol (
  IN   EFI_HANDLE *                               Handle,
  IN   EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL *NiiProtocol31,
  OUT  EFI_NII_POINTER_PROTOCOL *                 NIIPointerProtocol
  )
{
  EFI_STATUS Status;

  if (Handle == NULL
    || NiiProtocol31 == NULL
    || NIIPointerProtocol == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  NIIPointerProtocol->NiiProtocol31 = NiiProtocol31;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  Handle,
                  &gEfiNiiPointerGuid,
                  NIIPointerProtocol,
                  NULL
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL,
      ("InstallMultipleProtocolInterfaces gEfiNiiPointerGuid returns %r\n",
        Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }
  return EFI_SUCCESS;
}

/** Allocates new device path which consists of original and MAC address appended

   Using the NIC data structure information, read the EEPROM to get the
   MAC address and then allocate space for a new device path (**DevPtr) which will
   contain the original device path the NIC was found on (*BaseDevPtr)
   and an added MAC node.

   @param[in,out]   DevPtr       Pointer which will point to the newly created
                                 device path with the MAC node attached.
   @param[in]       BaseDevPtr   Pointer to the device path which the
                                 UNDI device driver is latching on to.
   @param[in]   GigAdapterInfo   Pointer to the NIC data structure information
                                 which the UNDI driver is layering on..

   @retval   EFI_SUCCESS           A MAC address was successfully appended to the Base Device Path.
   @retval   EFI_OUT_OF_RESOURCES  Not enough resources available to create new Device Path node.
**/
EFI_STATUS
GigAppendMac2DevPath (
  IN OUT EFI_DEVICE_PATH_PROTOCOL ** DevPtr,
  IN EFI_DEVICE_PATH_PROTOCOL *      BaseDevPtr,
  IN GIG_DRIVER_DATA *               GigAdapterInfo
  )
{
  MAC_ADDR_DEVICE_PATH      MacAddrNode;
  EFI_DEVICE_PATH_PROTOCOL *EndNode;
  UINT16                    i;
  UINT16                    TotalPathLen;
  UINT16                    BasePathLen;
  EFI_STATUS                Status;
  UINT8 *                   DevicePtr;

  DEBUGPRINT (INIT, ("GigAppendMac2DevPath\n"));

  ZeroMem (
    (CHAR8 *) &MacAddrNode,
    sizeof (MacAddrNode)
  );

  for (i = 0; i < 6; i++) {
    MacAddrNode.MacAddress.Addr[i] = GigAdapterInfo->Hw.mac.perm_addr[i];
  }

  DEBUGPRINT (INIT, ("\n"));
  for (i = 0; i < 6; i++) {
    DEBUGPRINT (INIT, ("%2x ", MacAddrNode.MacAddress.Addr[i]));
  }

  DEBUGPRINT (INIT, ("\n"));
  for (i = 0; i < 6; i++) {
    DEBUGPRINT (INIT, ("%2x ", GigAdapterInfo->Hw.mac.perm_addr[i]));
  }

  DEBUGPRINT (INIT, ("\n"));
  DEBUGWAIT (INIT);

  MacAddrNode.Header.Type       = MESSAGING_DEVICE_PATH;
  MacAddrNode.Header.SubType    = MSG_MAC_ADDR_DP;
  MacAddrNode.Header.Length[0]  = sizeof (MacAddrNode);
  MacAddrNode.Header.Length[1]  = 0;
  MacAddrNode.IfType            = PXE_IFTYPE_ETHERNET;

  // find the size of the base dev path.
  EndNode = BaseDevPtr;
  while (!IsDevicePathEnd (EndNode)) {
    EndNode = NextDevicePathNode (EndNode);
  }

  BasePathLen = (UINT16) ((UINTN) (EndNode) - (UINTN) (BaseDevPtr));

  // create space for full dev path
  TotalPathLen = (UINT16) (BasePathLen + sizeof (MacAddrNode) +
    sizeof (EFI_DEVICE_PATH_PROTOCOL));

  Status = gBS->AllocatePool (
                  EfiBootServicesData, // EfiRuntimeServicesData,
                  TotalPathLen,
                  (VOID **)&DevicePtr
                );

  if (Status != EFI_SUCCESS) {
    return Status;
  }

  // copy the base path, mac addr and end_dev_path nodes
  *DevPtr = (EFI_DEVICE_PATH_PROTOCOL *) DevicePtr;
  CopyMem (DevicePtr, (CHAR8 *) BaseDevPtr, BasePathLen);
  DevicePtr += BasePathLen;
  CopyMem (DevicePtr, (CHAR8 *) &MacAddrNode, sizeof (MacAddrNode));
  DevicePtr += sizeof (MacAddrNode);
  CopyMem (DevicePtr, (CHAR8 *) EndNode, sizeof (EFI_DEVICE_PATH_PROTOCOL));

  return EFI_SUCCESS;
}


/** Initializes Network Interface Identifier Protocol

   @param[in]       Handle           Controller/Child handle
   @param[out]      NiiProtocol31   NII Protocol instance

   @retval          EFI_SUCCESS            Procedure returned successfully
   @retval          EFI_INVALID_PARAMETER  Invalid parameter passed
   @retval          !EFI_SUCCESS           Failed to install NII Protocol 3.1
**/
EFI_STATUS
InitNiiProtocol (
  IN   EFI_HANDLE *                               Handle,
  OUT  EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL *NiiProtocol31
  )
{
  EFI_STATUS Status;

  if (Handle == NULL
    || NiiProtocol31 == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  NiiProtocol31->Id             = (UINT64) (mIntelgbePxe31);
  NiiProtocol31->IfNum          = mIntelgbePxe31->IFcnt;

  NiiProtocol31->Revision       = EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL_REVISION_31;
  NiiProtocol31->Type           = EfiNetworkInterfaceUndi;
  NiiProtocol31->MajorVer       = PXE_ROMID_MAJORVER;
  NiiProtocol31->MinorVer       = PXE_ROMID_MINORVER_31;
  NiiProtocol31->ImageSize      = 0;
  NiiProtocol31->ImageAddr      = 0;
  NiiProtocol31->Ipv6Supported  = TRUE;

  NiiProtocol31->StringId[0]    = 'U';
  NiiProtocol31->StringId[1]    = 'N';
  NiiProtocol31->StringId[2]    = 'D';
  NiiProtocol31->StringId[3]    = 'I';

  Status = gBS->InstallMultipleProtocolInterfaces (
                  Handle,
                  &gEfiNetworkInterfaceIdentifierProtocolGuid_31,
                  NiiProtocol31,
                  NULL
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (
      CRITICAL,
      ("InstallMultipleProtocolInterfaces \
        gEfiNetworkInterfaceIdentifierProtocolGuid_31 returns %r\n",
      Status)
    );
    DEBUGWAIT (CRITICAL);
    return Status;
  }
  return EFI_SUCCESS;
}

/** Initializes Driver Stop Protocol

   @param[in]       UndiPrivateData        Driver private data

   @retval          EFI_SUCCESS            Procedure returned successfully
   @retval          EFI_INVALID_PARAMETER  Invalid parameter passed
   @retval          !EFI_SUCCESS           Failed to initialize Driver Stop Protocol
**/
EFI_STATUS
InitDriverStopProtocol (
  IN  UNDI_PRIVATE_DATA *UndiPrivateData
  )
{
  EFI_STATUS Status;

  if (UndiPrivateData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  UndiPrivateData->DriverStop = gUndiDriverStop;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &UndiPrivateData->DeviceHandle,
                  &gEfiStartStopProtocolGuid,
                  &UndiPrivateData->DriverStop,
                  NULL
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("InstallMultipleProtocolInterfaces returns %r\n",
      Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }

  return EFI_SUCCESS;
}

/** Initializes Device Path Protocol

   @param[in]       UndiPrivateData        Driver private data

   @retval          EFI_SUCCESS            Procedure returned successfully
   @retval          EFI_INVALID_PARAMETER  Invalid parameter passed
   @retval          !EFI_SUCCESS           Failed to initialize Device Path Protocol
**/
EFI_STATUS
InitDevicePathProtocol (
  IN   UNDI_PRIVATE_DATA *UndiPrivateData
  )
{
  EFI_STATUS Status;

  if (UndiPrivateData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GigAppendMac2DevPath (
             &UndiPrivateData->Undi32DevPath,
             UndiPrivateData->Undi32BaseDevPath,
             &UndiPrivateData->NicInfo
           );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("GigAppendMac2DevPath returns %r\n", Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &UndiPrivateData->DeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  UndiPrivateData->Undi32DevPath,
                  NULL
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("InstallMultipleProtocolInterfaces returns %r\n",
      Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }

  return EFI_SUCCESS;
}


/** Initializes child protocols

   @param[in]       UndiPrivateData        Driver private data

   @retval          EFI_SUCCESS            Procedure returned successfully
   @retval          EFI_INVALID_PARAMETER  Invalid parameter passed
   @retval          !EFI_SUCCESS           Failed to init child protocols
**/
EFI_STATUS
InitChildProtocols (
  IN UNDI_PRIVATE_DATA *UndiPrivateData
  )
{
  EFI_STATUS Status;

  if (UndiPrivateData == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Status = InitDriverStopProtocol (UndiPrivateData);
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("InitDriverStopProtocol returned %r\n", Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }

  Status = InitDevicePathProtocol (
             UndiPrivateData
           );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("InitDevicePathProtocol returned %r\n", Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }

  Status = InitNiiPointerProtocol (
             &UndiPrivateData->DeviceHandle,
             &UndiPrivateData->NiiProtocol31,
             &UndiPrivateData->NIIPointerProtocol
           );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("InitNiiPointerProtocol returned %r\n", Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }

  if (UndiPrivateData->NicInfo.UndiEnabled) {
    Status = InitNiiProtocol (
               &UndiPrivateData->DeviceHandle,
               &UndiPrivateData->NiiProtocol31
             );
    if (EFI_ERROR (Status)) {
      DEBUGPRINT (CRITICAL, ("InitNiiProtocol returned %r\n", Status));
      DEBUGWAIT (CRITICAL);
      return Status;
    }
  }

  Status = InitAdapterInformationProtocol (UndiPrivateData);
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (
      CRITICAL, ("Could not install Adapter Information Protocol interface. \
        Error = %d %r\n",
      Status, Status)
    );
    DEBUGWAIT (CRITICAL);
    return Status;
  }

  return EFI_SUCCESS;
}

/** Initializes child

   @param[in]       UndiPrivateData        Driver private data

   @retval          EFI_SUCCESS            Procedure returned successfully
   @retval          EFI_INVALID_PARAMETER  Invalid parameter passed
   @retval          EFI_DEVICE_ERROR       Init HW failed
**/
EFI_STATUS
InitChild (
  IN UNDI_PRIVATE_DATA *UndiPrivateData
  )
{
  EFI_STATUS Status;

  if (UndiPrivateData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &UndiPrivateData->DeviceHandle,
                  NULL
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("InstallMultipleProtocolInterfaces returns %r\n",
      Status));
    return Status;
  }

  return EFI_SUCCESS;
}


/** Initializes Undi Private Data structure

   @param[in]       Controller             Controller handle
   @param[out]      UndiPrivateData        Driver private data

   @retval          EFI_SUCCESS            Procedure returned successfully
   @retval          EFI_OUT_OF_RESOURCES   Out of memory resources
**/
EFI_STATUS
InitUndiPrivateData (
  IN  EFI_HANDLE          Controller,
  OUT UNDI_PRIVATE_DATA **UndiPrivateData
  )
{
  UNDI_PRIVATE_DATA *PrivateData;

  PrivateData = AllocateZeroPool (sizeof (UNDI_PRIVATE_DATA));
  if (PrivateData == NULL) {
    DEBUGPRINT (CRITICAL, ("AllocateZeroPool returns %r\n", PrivateData));
    DEBUGWAIT (CRITICAL);
    return EFI_OUT_OF_RESOURCES;
  }
  PrivateData->Signature              = GIG_UNDI_DEV_SIGNATURE;
  PrivateData->DeviceHandle           = NULL;
  PrivateData->NicInfo.HwInitialized  = FALSE;

  // Save off the controller handle so we can disconnect the driver later
  PrivateData->ControllerHandle = Controller;
  DEBUGPRINT (
    INIT, ("ControllerHandle = %x\n",
    PrivateData->ControllerHandle)
  );

  mIntelgbeUndi32DeviceList[mActiveControllers] = PrivateData;
  mActiveControllers++;

  *UndiPrivateData = PrivateData;
  return EFI_SUCCESS;
}

/** Initializes controller

   @param[in]       UndiPrivateData        Driver private data

   @retval          EFI_SUCCESS            Procedure returned successfully
   @retval          EFI_INVALID_PARAMETER  Invalid parameter passed
   @retval          EFI_OUT_OF_RESOURCES   PCI Init failed
   @retval          EFI_ACCESS_DENIED      Cannot acquire controller
**/
EFI_STATUS
InitController (
  IN UNDI_PRIVATE_DATA *UndiPrivateData
  )
{
  EFI_STATUS Status;

  if (UndiPrivateData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Initialize PCI-E Bus and read PCI related information.
  Status = IntelgbePciInit (&UndiPrivateData->NicInfo);
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("IntelgbePciInit fails: %r\n", Status));
    return EFI_OUT_OF_RESOURCES;
  }

  // Perform the first time initialization of the hardware
  Status = IntelgbeFirstTimeInit (&UndiPrivateData->NicInfo);
  if (EFI_ERROR (Status)
    && (Status != EFI_ACCESS_DENIED))
  {
    DEBUGPRINT (CRITICAL, ("IntelgbeFirstTimeInit returns %r\n", Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }

  UndiPrivateData->NicInfo.UndiEnabled = TRUE;
  if (Status == EFI_ACCESS_DENIED) {
    UndiPrivateData->NicInfo.UndiEnabled = FALSE;
  }

  UndiPrivateData->AltMacAddrSupported = FALSE;
  return EFI_SUCCESS;
}

/** Opens protocols for Child device

   @param[in]       UndiPrivateData        Driver private data
   @param[in]       This                   Driver Binding protocol instance
   @param[in]       Controller             Controller handle

   @retval          EFI_SUCCESS            Procedure returned successfully
   @retval          EFI_INVALID_PARAMETER  Invalid parameter passed
   @retval          EFI_DEVICE_ERROR       Failed to open PCI IO protocol
**/
EFI_STATUS
OpenChildProtocols (
  IN  UNDI_PRIVATE_DATA *          UndiPrivateData,
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                   Controller
  )
{
  EFI_STATUS           Status;
  EFI_PCI_IO_PROTOCOL *PciIo;

  if (UndiPrivateData == NULL
    || This == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  // Open For Child Device
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  UndiPrivateData->DeviceHandle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("OpenProtocol returns %r\n", Status));
    return Status;
  }
  return EFI_SUCCESS;
}

/** Initializes controller protocols

   @param[in]       UndiPrivateData        Driver private data
   @param[in]       Controller             Controller handle

   @retval          EFI_SUCCESS            Procedure returned successfully
   @retval          EFI_INVALID_PARAMETER  Invalid parameter passed
**/
EFI_STATUS
InitControllerProtocols (
  IN UNDI_PRIVATE_DATA *UndiPrivateData,
  IN EFI_HANDLE         Controller
  )
{
  EFI_STATUS Status;

  if (UndiPrivateData == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  // Figure out the controllers name for the Component Naming protocol
  // This must be performed after GigAppendMac2DevPath because we need the MAC
  // address of the controller to name it properly
  ComponentNameInitializeControllerName (UndiPrivateData);


  // The EFI_NII_POINTER_PROTOCOL protocol is used only by this driver.  It is done so that
  // we can get the NII protocol from either the parent or the child handle.  This is convenient
  // in the Diagnostic protocol because it allows the test to be run when called from either the
  // parent or child handle which makes it more user friendly.
  Status = InitNiiPointerProtocol (
             &Controller,
             &UndiPrivateData->NiiProtocol31,
             &UndiPrivateData->NIIPointerProtocol
           );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("InitNiiPointerProtocol returned %r\n", Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }
  return EFI_SUCCESS;
}

/** Start this driver on Controller by opening PciIo and DevicePath protocol.

   Initialize PXE structures, create a copy of the Controller Device Path with the
   NIC's MAC address appended to it, install the NetworkInterfaceIdentifier protocol
   on the newly created Device Path.

   @param[in]   This                  Protocol instance pointer.
   @param[in]   Controller            Handle of device to work with.
   @param[in]   RemainingDevicePath   With its specific type (or being NULL) indicates
                                      whether to produce children or not

   @retval   EFI_SUCCESS         This driver is added to controller or controller
                                 and specific child is already initialized
   @retval   !EFI_SUCCESS        Failed to initialize controller or child
**/
EFI_STATUS
EFIAPI
IntelgbeUndiDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL *   RemainingDevicePath
  )
{
  UNDI_PRIVATE_DATA *UndiPrivateData      = NULL;
  EFI_STATUS         Status               = EFI_SUCCESS;
  BOOLEAN            InitializeChild      = TRUE;
  BOOLEAN            InitializeController = TRUE;

  DEBUGPRINT (INIT, ("GigUndiDriverStart\n"));
  DEBUGWAIT (INIT);

  UndiPrivateData = GetControllerPrivateData (Controller);
  if (UndiPrivateData != NULL) {
    InitializeController = FALSE;
  }

  if (RemainingDevicePath != NULL) {
    InitializeChild = IsNotEndOfDevicePathNode (RemainingDevicePath);
  }

  if (!InitializeController
    && !InitializeChild)
  {
    return EFI_SUCCESS;
  }

  if (InitializeController) {
    Status = InitUndiPrivateData (
               Controller,
               &UndiPrivateData
             );
    if (EFI_ERROR (Status)) {
      DEBUGPRINT (CRITICAL, ("InitUndiPrivateData returns %r\n", Status));
      DEBUGWAIT (CRITICAL);
      goto UndiError;
    }
    Status = OpenControllerProtocols (
               Controller,
               This,
               UndiPrivateData
             );
    if (EFI_ERROR (Status)) {
      DEBUGPRINT (CRITICAL, ("OpenContollerProtocols returns %r\n", Status));
      DEBUGWAIT (CRITICAL);
      goto UndiError;
    }
    Status = InitController (UndiPrivateData);
    if (EFI_ERROR (Status) &&
      (Status != EFI_ACCESS_DENIED))
    {
      DEBUGPRINT (CRITICAL, ("InitController fails: %r", Status));
      goto UndiErrorDeleteDevicePath;
    }
    Status = InitControllerProtocols (
               UndiPrivateData,
               Controller
             );
    if (EFI_ERROR (Status)) {
      DEBUGPRINT (CRITICAL,
        ("InitControllerProtocols failed with %r\n", Status));
      goto UndiErrorDeleteDevicePath;
    }
  }

  if (InitializeChild) {
    Status = InitUndiStructures (UndiPrivateData);
    if (EFI_ERROR (Status)) {
      DEBUGPRINT (CRITICAL, ("InitUndiStructures failed with %r\n", Status));
      goto UndiErrorDeleteDevicePath;
    }
    Status = InitChild (UndiPrivateData);
    if (EFI_ERROR (Status)) {
      DEBUGPRINT (CRITICAL, ("InitChild failed with %r\n", Status));
      goto UndiErrorDeleteDevicePath;
    }
    Status = InitChildProtocols (
               UndiPrivateData
             );
    if (EFI_ERROR (Status)) {
      DEBUGPRINT (CRITICAL, ("InitChildProtocols failed with %r\n", Status));
      goto UndiErrorDeleteDevicePath;
    }
    Status = OpenChildProtocols (
               UndiPrivateData,
               This,
               Controller
             );
    if (EFI_ERROR (Status)) {
      DEBUGPRINT (CRITICAL, ("OpenChildProtocols failed with %r\n", Status));
    }
    UndiPrivateData->IsChildInitialized = TRUE;
  }
  return EFI_SUCCESS;

UndiErrorDeleteDevicePath:
  GigUndiPxeUpdate (NULL, mIntelgbePxe31);
  gBS->FreePool (UndiPrivateData->Undi32DevPath);
UndiError:
  mActiveControllers--;

  CloseControllerProtocols (
    Controller,
    This
  );
  gBS->FreePool ((VOID **) UndiPrivateData);
  return Status;
}

/** Initializes the !PXE structure

   @param[in,out]   PxePtr        Pointer to the PXE structure to initialize
   @param[in]       VersionFlag   Indicates PXE version 3.0 or 3.1

   @retval    None
**/
VOID
GigUndiPxeStructInit (
  PXE_SW_UNDI *PxePtr,
  UINTN        VersionFlag
  )
{
  // initialize the !PXE structure
  PxePtr->Signature = PXE_ROMID_SIGNATURE;
  PxePtr->Len       = sizeof (PXE_SW_UNDI);
  PxePtr->Fudge     = 0;  // cksum
  PxePtr->IFcnt     = 0;  // number of NICs this UNDI supports
  PxePtr->Rev       = PXE_ROMID_REV;
  PxePtr->MajorVer  = PXE_ROMID_MAJORVER;
  PxePtr->MinorVer  = PXE_ROMID_MINORVER_31;
  PxePtr->reserved1 = 0;

  PxePtr->Implementation = PXE_ROMID_IMP_SW_VIRT_ADDR |
                           PXE_ROMID_IMP_FRAG_SUPPORTED |
                           PXE_ROMID_IMP_CMD_LINK_SUPPORTED |
                           PXE_ROMID_IMP_NVDATA_READ_ONLY |
                           PXE_ROMID_IMP_STATION_ADDR_SETTABLE |
                           PXE_ROMID_IMP_PROMISCUOUS_MULTICAST_RX_SUPPORTED |
                           PXE_ROMID_IMP_PROMISCUOUS_RX_SUPPORTED |
                           PXE_ROMID_IMP_BROADCAST_RX_SUPPORTED |
                           PXE_ROMID_IMP_FILTERED_MULTICAST_RX_SUPPORTED |
                           PXE_ROMID_IMP_TX_COMPLETE_INT_SUPPORTED |
                           PXE_ROMID_IMP_PACKET_RX_INT_SUPPORTED;

  PxePtr->EntryPoint    = (UINT64) IntelgbeUndiApiEntry;
  PxePtr->reserved2[0]  = 0;
  PxePtr->reserved2[1]  = 0;
  PxePtr->reserved2[2]  = 0;
  PxePtr->BusCnt        = 1;
  PxePtr->BusType[0]    = PXE_BUSTYPE_PCI;

  PxePtr->Fudge         = (UINT8)
  (PxePtr->Fudge - IntelgbeChkSum ((VOID *) PxePtr, PxePtr->Len));
}

/** Allocate and initialize both (old and new) the !PXE structures here.

   There should only be one copy of each of these structure for any number
   of NICs this UNDI supports. Also, these structures need to be on a
   paragraph boundary as per the spec. so, while allocating space for these,
   make sure that there is space for 2 !PXE structures (old and new) and a
   32 bytes padding for alignment adjustment (in case)

   @param[in]   VOID

   @retval   EFI_SUCCESS            !PXE structure initialized
   @retval   EFI_OUT_OF_RESOURCES   Failed to allocate memory for !PXE structure
**/
EFI_STATUS
InitializePxeStruct (
  VOID
  )
{
  EFI_STATUS Status;

  Status = gBS->AllocatePool (
                  EfiBootServicesData,  // EfiRuntimeServicesData,
                  (sizeof (PXE_SW_UNDI) + sizeof (PXE_SW_UNDI) + 32),
                  &mIntelgbePxeMemPtr
                );

  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("AllocatePool returns %r\n", Status));
    return Status;
  }

  ZeroMem (
    mIntelgbePxeMemPtr,
    sizeof (PXE_SW_UNDI) + sizeof (PXE_SW_UNDI) + 32
  );

  // check for paragraph alignment here, assuming that the pointer is
  // already 8 byte aligned.
  if (((UINTN) mIntelgbePxeMemPtr & 0x0F) != 0) {
    mIntelgbePxe31 = (PXE_SW_UNDI *)
    ((UINTN) ((((UINTN) mIntelgbePxeMemPtr) & (0xFFFFFFFFFFFFFFF0)) + 0x10));
  } else {
    mIntelgbePxe31 = (PXE_SW_UNDI *) mIntelgbePxeMemPtr;
  }

  GigUndiPxeStructInit (mIntelgbePxe31, 0x31);  // 3.1 entry

  return Status;
}

/** Register Driver Binding protocol for this driver.
   @param[in]   ImageHandle   Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT
   @param[in]   SystemTable   EFI System Table structure pointer
   @retval   EFI_SUCCESS   Driver is successfully loaded
   @retval   EFI_OUT_OF_RESOURCES   Failed to install DriverBinding, ComponentName
                                    and Diagnostics Protocols
   @retval   EFI_OUT_OF_RESOURCES   Failed to install DriverHealth or supported EFI
                                    version protocols
**/
EFI_STATUS
EFIAPI
InitializeIntelgbeUNDIDriver (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS                 Status;

  gSystemTable = SystemTable;

  Status = EfiLibInstallDriverBinding (ImageHandle, SystemTable,
    &gUndiDriverBinding, ImageHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  DEBUGPRINT (INIT, ("SystemTable->Hdr.Revision = %x\n",
    SystemTable->Hdr.Revision));
  DEBUGPRINT (INIT,
    ("Installing UEFI 1.10/2.10 Driver Diags and Component Name protocols.\n"));
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &gUndiDriverBinding.DriverBindingHandle,
                  &gEfiComponentNameProtocolGuid,
                  &gUndiComponentName,
                  &gEfiComponentName2ProtocolGuid,
                  &gUndiComponentName2,
                  &gEfiDriverConfigurationProtocolGuid,
                  &gGigUndiDriverConfiguration,
                  NULL
                );

  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("InstallMultipleProtocolInterfaces returns %x\n",
      Status));
    return Status;
  }
  if (SystemTable->Hdr.Revision >= EFI_2_10_SYSTEM_TABLE_REVISION) {
    DEBUGPRINT (INIT,
      ("Installing UEFI 2.1 Supported EFI Version Protocol.\n"));
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &ImageHandle,
                    &gEfiDriverSupportedEfiVersionProtocolGuid,
                    &gUndiSupportedEfiVersion,
                    NULL
                  );
  }
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("InstallMultipleProtocolInterfaces returns %x\n",
      Status));
    return Status;
  }

  Status = InitializePxeStruct ();

  return Status;
}

/** Stops controller

   @param[in]       This                   Driver Binding protocol instance
   @param[in]       Controller             Controller handle
   @param[in]       UndiPrivateData        Driver private data structure

   @retval          EFI_SUCCESS            Procedure returned successfully
   @retval          EFI_INVALID_PARAMETER  Invalid parameter passed
   @retval          !EFI_SUCCESS           Failed to stop controller
**/
EFI_STATUS
StopController (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                   Controller,
  IN  UNDI_PRIVATE_DATA *          UndiPrivateData
  )
{
  EFI_STATUS Status;

  if (This == NULL
    || UndiPrivateData == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Controller,
                  &gEfiNiiPointerGuid,
                  &UndiPrivateData->NIIPointerProtocol,
                  NULL
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("UninstallMultipleProtocolInterfaces returns %r\n",
      Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }

  DEBUGPRINT (INIT, ("EfiLibFreeUnicodeStringTable"));
  FreeUnicodeStringTable (UndiPrivateData->ControllerNameTable);

  mIntelgbeUndi32DeviceList[UndiPrivateData->NiiProtocol31.IfNum] = NULL;
  DEBUGPRINT (INIT, ("FreePool(UndiPrivateData->Undi32DevPath)"));
  Status = gBS->FreePool (UndiPrivateData->Undi32DevPath);
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL,
      ("FreePool(UndiPrivateData->Undi32DevPath) returns %r\n", Status));
  }

  // Free DMA resources: Tx & Rx descriptors, Rx buffers
  UndiDmaFreeCommonBuffer (
    UndiPrivateData->NicInfo.PciIo,
    &UndiPrivateData->NicInfo.TxRing
    );

  UndiDmaFreeCommonBuffer (
    UndiPrivateData->NicInfo.PciIo,
    &UndiPrivateData->NicInfo.RxRing
    );

  UndiDmaFreeCommonBuffer (
    UndiPrivateData->NicInfo.PciIo,
    &UndiPrivateData->NicInfo.RxBufferMapping
    );

  DEBUGPRINT (INIT, ("Attributes"));
  Status = UndiPrivateData->NicInfo.PciIo->Attributes (
                                             UndiPrivateData->NicInfo.PciIo,
                                             EfiPciIoAttributeOperationSet,
                                UndiPrivateData->NicInfo.OriginalPciAttributes,
                                             NULL
                                           );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("PCI IO Attributes returns %r\n", Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }
  // Decrement the number of interfaces this driver is supporting
  DEBUGPRINT (INIT, ("IntelgbeUndiPxeUpdate"));

  Status = CloseControllerProtocols (
             Controller,
             This
           );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("CloseControllerProtocols failed with %r\n",
      Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }

  Status = gBS->FreePool (UndiPrivateData);
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("FreePool(UndiPrivateData) returns %r\n", Status));
    DEBUGWAIT (CRITICAL);
  }
  return EFI_SUCCESS;
}

/** Stops child

   @param[in]       This                   Driver Binding protocol instance
   @param[in]       Controller             Controller handle
   @param[in]       ChildHandleBuffer      Buffer with child handles
   @param[in]       UndiPrivateData        Driver private data structure

   @retval          EFI_SUCCESS            Procedure returned successfully
   @retval          EFI_INVALID_PARAMETER  Invalid parameter passed
   @retval          !EFI_SUCCESS           Failed to close PciIo protocol
**/
EFI_STATUS
StopChild (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                   Controller,
  IN  EFI_HANDLE *                 ChildHandleBuffer,
  IN  UNDI_PRIVATE_DATA *          UndiPrivateData
  )
{
  EFI_STATUS Status;

  if (This == NULL
    || ChildHandleBuffer == NULL
    || UndiPrivateData == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  // Close the bus driver
  DEBUGPRINT (INIT, ("Removing gEfiPciIoProtocolGuid\n"));
  Status = gBS->CloseProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  This->DriverBindingHandle,
                  ChildHandleBuffer[0]
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("Close of gEfiPciIoProtocolGuid failed with %r\n",
      Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }


  Status = UninstallAdapterInformationProtocol (UndiPrivateData);
  if ((EFI_ERROR (Status)) && (Status != EFI_UNSUPPORTED)) {
    DEBUGPRINT (CRITICAL, ("UninstallAdapterInformationProtocol returns %r\n",
      Status));
    DEBUGWAIT (CRITICAL);
  }

  DEBUGPRINT (INIT, ("UninstallMultipleProtocolInterfaces\n"));


  if (UndiPrivateData->NicInfo.UndiEnabled) {
    Status = gBS->UninstallMultipleProtocolInterfaces (
                    UndiPrivateData->DeviceHandle,
                    &gEfiStartStopProtocolGuid,
                    &UndiPrivateData->DriverStop,
                    &gEfiNetworkInterfaceIdentifierProtocolGuid_31,
                    &UndiPrivateData->NiiProtocol31,
                    &gEfiNiiPointerGuid,
                    &UndiPrivateData->NIIPointerProtocol,
                    &gEfiDevicePathProtocolGuid,
                    UndiPrivateData->Undi32DevPath,
                    NULL
                  );
  } else {
    Status = gBS->UninstallMultipleProtocolInterfaces (
                    UndiPrivateData->DeviceHandle,
                    &gEfiStartStopProtocolGuid,
                    &UndiPrivateData->DriverStop,
                    &gEfiNiiPointerGuid,
                    &UndiPrivateData->NIIPointerProtocol,
                    &gEfiDevicePathProtocolGuid,
                    UndiPrivateData->Undi32DevPath,
                    NULL
                  );
  }
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("UninstallMultipleProtocolInterfaces returns %r\n",
      Status));
    DEBUGWAIT (CRITICAL);
  }

  // If we get the ACCESS_DENIED status code usually calling UninstallMultipleProtocolInterfaces a second
  // time will uninstall the protocols successfully.
  if (Status == EFI_ACCESS_DENIED) {
    DEBUGPRINT (INIT, ("UninstallMultipleProtocolInterfaces\n"));
    Status = gBS->UninstallMultipleProtocolInterfaces (
                    UndiPrivateData->DeviceHandle,
                    &gEfiNetworkInterfaceIdentifierProtocolGuid_31,
                    &UndiPrivateData->NiiProtocol31,
                    &gEfiNiiPointerGuid,
                    &UndiPrivateData->NIIPointerProtocol,
                    &gEfiDevicePathProtocolGuid,
                    UndiPrivateData->Undi32DevPath,
                    NULL
                  );
    if (EFI_ERROR (Status)) {
      DEBUGPRINT (CRITICAL, ("UninstallMultipleProtocolInterfaces returns %r\n",
        Status));
      DEBUGWAIT (CRITICAL);
    }
  }

  if (UndiPrivateData->NicInfo.UndiEnabled) {

    // Call shutdown to clear DRV_LOAD bit and stop Rx and Tx
    IntelgbeShutdown (&UndiPrivateData->NicInfo);
  }
  return EFI_SUCCESS;
}

/** Stops driver on children and controller

   Stop this driver on Controller by removing NetworkInterfaceIdentifier protocol and
   closing the DevicePath and PciIo protocols on Controller. Stops controller only when
   all children were stopped in first place.

   @param[in]   This                Protocol instance pointer.
   @param[in]   Controller          Handle of device to stop driver on.
   @param[in]   NumberOfChildren    How many children need to be stopped.
   @param[in]   ChildHandleBuffer   Child handle buffer to uninstall.

   @retval   EFI_SUCCESS       This driver is removed Controller.
   @retval   EFI_DEVICE_ERROR  The controller or child could not be successfully stopped.
   @retval   EFI_OUT_OF_RESOURCES   Number of children exceeds 1
   @retval   !EFI_SUCCESS      Failed to open NII pointer protocol
**/
EFI_STATUS
EFIAPI
IntelgbeUndiDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                   Controller,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE *                 ChildHandleBuffer
  )
{
  EFI_STATUS                Status;
  EFI_NII_POINTER_PROTOCOL *NiiPointerProtocol;
  UNDI_PRIVATE_DATA *       UndiPrivateData;

  DEBUGPRINT (INIT, ("DriverStop\n"));
  DEBUGWAIT (INIT);

  DEBUGPRINT (INIT, ("Number of children %d\n", NumberOfChildren));
  //  Open an instance for the Network Interface Identifier Protocol so we can check to see
  // if the interface has been shutdown.  Does not need to be closed because we use the
  // GET_PROTOCOL attribute to open it.
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiNiiPointerGuid,
                  (VOID **) &NiiPointerProtocol,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("OpenProtocol returns %r\n", Status));
    return Status;
  }

  UndiPrivateData =
  UNDI_PRIVATE_DATA_FROM_THIS (NiiPointerProtocol->NiiProtocol31);
  DEBUGPRINT (INIT, ("State = %X\n", UndiPrivateData->NicInfo.State));
  DEBUGWAIT (INIT);

  if (NumberOfChildren == 0) {
    Status = StopController (
               This,
               Controller,
               UndiPrivateData
             );
    if (EFI_ERROR (Status)) {
      DEBUGPRINT (CRITICAL,
        ("StopController failed with status: %r\n", Status));
      return EFI_DEVICE_ERROR;
    }
    mActiveControllers--;
    return EFI_SUCCESS;
  }

  if (NumberOfChildren > 1) {
    DEBUGPRINT (INIT, ("Unexpected number of child handles.\n"));
    return EFI_INVALID_PARAMETER;
  }
  Status = StopChild (
             This,
             Controller,
             ChildHandleBuffer,
             UndiPrivateData
           );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("StopChild failed with status: %r\n", Status));
    return EFI_DEVICE_ERROR;
  }
  mActiveChildren--;
  return EFI_SUCCESS;
}

/** Callback to unload the GigUndi from memory.

   @param[in]   ImageHandle   Image Handle to driver

   @retval   EFI_SUCCESS            This driver was unloaded successfully.
   @retval   EFI_INVALID_PARAMETER  Failed to disconnect controller
   @retval   !EFI_SUCCESS           Failed to unload driver
**/
EFI_STATUS
EFIAPI
IntelgbeUndiUnload (
  IN EFI_HANDLE ImageHandle
  )
{
  EFI_HANDLE *DeviceHandleBuffer;
  UINTN       DeviceHandleCount;
  UINTN       Index;

  EFI_STATUS Status;

  DEBUGPRINT (INIT, ("IntelgbeUndiUnload intelgbe_pxe->IFcnt = %d\n",
    mIntelgbePxe31->IFcnt));
  DEBUGWAIT (INIT);

  // Get the list of all the handles in the handle database.
  // If there is an error getting the list, then the unload operation fails.
  Status = gBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &DeviceHandleCount,
                  &DeviceHandleBuffer
                );
  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("LocateHandleBuffer returns %r\n", Status));
    return Status;
  }

  // Disconnect the driver specified by ImageHandle from all the devices in the
  // handle database.
  DEBUGPRINT (INIT, ("Active interfaces = %d\n", mActiveControllers));
  DEBUGPRINT (INIT, ("Active children = %d\n", mActiveChildren));

  for (Index = 0; Index < DeviceHandleCount; Index++) {
    Status = gBS->DisconnectController (
                    DeviceHandleBuffer[Index],
                    ImageHandle,
                    NULL
                  );
  }
  DEBUGPRINT (INIT, ("Active interfaces = %d\n", mActiveControllers));
  DEBUGPRINT (INIT, ("Active children = %d\n", mActiveChildren))

  // Free the buffer containing the list of handles from the handle database
  if (DeviceHandleBuffer != NULL) {
    gBS->FreePool (DeviceHandleBuffer);
  }
  if (mActiveControllers == 0) {

    // Free PXE structures since they will no longer be needed
    Status = gBS->FreePool (mIntelgbePxeMemPtr);
    if (EFI_ERROR (Status)) {
      DEBUGPRINT (CRITICAL, ("FreePool returns %r\n", Status));
      return Status;
    }
    DEBUGPRINT (INIT,
      ("Uninstalling UEFI 1.10/2.10 Driver Diags and \
        Component Name protocols.\n"));
    Status = gBS->UninstallMultipleProtocolInterfaces (
                    ImageHandle,
                    &gEfiDriverBindingProtocolGuid,
                    &gUndiDriverBinding,
                    &gEfiComponentNameProtocolGuid,
                    &gUndiComponentName,
                    &gEfiComponentName2ProtocolGuid,
                    &gUndiComponentName2,
                    &gEfiDriverConfigurationProtocolGuid,
                    &gGigUndiDriverConfiguration,
                    NULL
                  );

    if (EFI_ERROR (Status)) {
      DEBUGPRINT (CRITICAL, ("UninstallMultipleProtocolInterfaces returns %x\n",
        Status));
      return Status;
    }

    if (gST->Hdr.Revision >= EFI_2_10_SYSTEM_TABLE_REVISION) {
      DEBUGPRINT (INIT, ("Uninstalling UEFI 2.1 Supported \
        EFI Version Protocol.\n"));
      Status = gBS->UninstallMultipleProtocolInterfaces (
                      ImageHandle,
                      &gEfiDriverSupportedEfiVersionProtocolGuid,
                      &gUndiSupportedEfiVersion,
                      NULL
                    );
    }
    if (EFI_ERROR (Status)) {
      DEBUGPRINT (CRITICAL, ("UninstallMultipleProtocolInterfaces returns %x\n",
        Status));
      return Status;
    }


  } else {
    DEBUGPRINT (INIT, ("Returning EFI_INVALID_PARAMETER\n"));
    DEBUGWAIT (INIT);
    return EFI_INVALID_PARAMETER;
  }
  return Status;
}

/* Protocol structure definition and initialization */
EFI_DRIVER_BINDING_PROTOCOL gUndiDriverBinding = {
  IntelgbeUndiDriverSupported, // Supported
  IntelgbeUndiDriverStart,     // Start
  IntelgbeUndiDriverStop,      // Stop
  VERSION_TO_HEX,              // Driver Version
  NULL,                      // ImageHandle
  NULL                       // Driver Binding Handle
};
