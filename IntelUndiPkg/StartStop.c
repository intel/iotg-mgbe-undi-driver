/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Intelgbe.h"
#include "StartStop.h"

EFI_GUID gEfiStartStopProtocolGuid = EFI_DRIVER_STOP_PROTOCOL_GUID;

/** Issues a call to stop the driver so diagnostic application can access the hardware.

   @param[in]   This       Pointer to the EFI_DRIVER_STOP_PROTOCOL instance.

   @retval   EFI_SUCCESS   Driver is stopped successfully
**/
EFI_STATUS
EFIAPI
StopDriver (
  IN EFI_DRIVER_STOP_PROTOCOL *This
  )
{
  EFI_STATUS         Status = EFI_SUCCESS;
  UNDI_PRIVATE_DATA *GigPrivate;

  DEBUGPRINT (DIAG, ("Entering StopDriver\n"));
  DEBUGWAIT (DIAG);

  GigPrivate = UNDI_PRIVATE_DATA_FROM_DRIVER_STOP (This);

  GigPrivate->NicInfo.DriverBusy = TRUE;

  return Status;
}

/** Issues a call to start the driver after diagnostic application has completed.

   @param[in]   This       Pointer to the EFI_DRIVER_STOP_PROTOCOL instance.

   @retval   EFI_SUCCESS   If driver has restarted successfully
**/
EFI_STATUS
EFIAPI
StartDriver (
  IN EFI_DRIVER_STOP_PROTOCOL *This
  )
{
  EFI_STATUS         Status = EFI_SUCCESS;
  UNDI_PRIVATE_DATA *GigPrivate;
  BOOLEAN            ReceiveStarted;

  DEBUGPRINT (DIAG, ("Entering StartDriver\n"));
  DEBUGWAIT (DIAG);

  GigPrivate = UNDI_PRIVATE_DATA_FROM_DRIVER_STOP (This);

  // Save off the value of ReceiveStarted as it will be reset by InitializeGigUNDIDriver
  ReceiveStarted = GigPrivate->NicInfo.ReceiveStarted;

  GigPrivate->NicInfo.HwInitialized = FALSE;
  if (GigPrivate->NicInfo.State == PXE_STATFLAGS_GET_STATE_INITIALIZED
    && GigPrivate->IsChildInitialized)
  {
    IntelgbeInititialize (&GigPrivate->NicInfo);
    DEBUGPRINT (DIAG, ("IntelgbeInititialize complete\n"));

    //  Restart the receive unit if it was running on entry
    if (ReceiveStarted) {
      DEBUGPRINT (DIAG, ("RESTARTING RU\n"));
    }
  }

  GigPrivate->NicInfo.DriverBusy = FALSE;

  return Status;
}

/* Protocol structure definition and initialization */
EFI_DRIVER_STOP_PROTOCOL gUndiDriverStop = {
  StopDriver,
  StartDriver
};
