/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Intelgbe.h"
#include "DriverConfiguration.h"

/* Protocol structure tentative definition */
EFI_DRIVER_CONFIGURATION_PROTOCOL gGigUndiDriverConfiguration;

CHAR16 *mConfigMenu[] = {
  L"Set adapter to Autonegotiate (recommended)",
  L"Set adapter to 100Mbps full duplex",
  L"Set adapter to 100Mbps half duplex",
  L"Set adapter to 10Mbps full duplex",
  L"Set adapter to 10Mbps half duplex",
  L"Save settings to NVRAM",
  L"Exit (maintain current settings)",
  NULL
};


/** Displays the options for the configuration menu to allow the user
   to change the speed/duplex settings

   @param[in]   UndiPrivateData   Driver private data structure

   @retval   None
**/
VOID
GigUndiDriverConfigurationDisplayMenu (
  UNDI_PRIVATE_DATA *UndiPrivateData
  )
{
}

/** Callback function for Driver Configuration protocol.  Finds the NII adapter handle for
   Controller handle and then calls the setup menu

   @param[in]   This              Driver configuration protocol instance
   @param[in]   ControllerHandle  The network driver controller handle
   @param[in]   ChildHandle       The NII child handle (not used)
   @param[in]   Language          Always english
   @param[out]  ActionRequired    Not used

   @retval   EFI_UNSUPPORTED   Unable to open the driver configuration protocol for ControllerHandle
   @retval   EFI_UNSUPPORTED   ControllerHandle is NULL
   @retval   EFI_UNSUPPORTED   Driver is not managing ControllerHandle
   @retval   EFI_UNSUPPORTED   It is fiber or SerDes card
   @retval   EFI_SUCCESS       Configuration was successful
**/
EFI_STATUS
EFIAPI
GigUndiDriverConfigurationSetOptions (
  IN EFI_DRIVER_CONFIGURATION_PROTOCOL *          This,
  IN EFI_HANDLE                                   ControllerHandle,
  IN EFI_HANDLE                                   ChildHandle, OPTIONAL
  IN CHAR8 *                                      Language,
  OUT EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED *  ActionRequired
  )
{
  return EFI_SUCCESS;
}

/** Not implemented

   @param[in]   This                Driver configuration protocol current instance
   @param[in]   ControllerHandle    Controller handle
   @param[in]   ChildHandle

   @retval   EFI_SUCCESS   Always returned
**/
EFI_STATUS
EFIAPI
GigUndiDriverConfigurationOptionsValid (
  IN EFI_DRIVER_CONFIGURATION_PROTOCOL *   This,
  IN EFI_HANDLE                            ControllerHandle,
  IN EFI_HANDLE                            ChildHandle OPTIONAL
  )
{
  return EFI_SUCCESS;
}

/** Restores the speed/duplex settings to the autonegotiation default value

   @param[in]   This              Driver configuration protocol instance
   @param[in]   ControllerHandle  The network driver controller handle
   @param[in]   ChildHandle       The NII child handle (not used)
   @param[in]   DefaultType       Not used
   @param[out]  ActionRequired    Not used

   @retval   EFI_UNSUPPORTED   Unable to open the driver configuration protocol for ControllerHandle
   @retval   EFI_UNSUPPORTED   ControllerHandle is NULL
   @retval   EFI_UNSUPPORTED   Driver is not managing ControllerHandle
   @retval   EFI_UNSUPPORTED   It is fiber or SerDes card
   @retval   EFI_SUCCESS       Configuration was successful
**/
EFI_STATUS
EFIAPI
GigUndiDriverConfigurationForceDefaults (
  IN EFI_DRIVER_CONFIGURATION_PROTOCOL *                   This,
  IN EFI_HANDLE                                            ControllerHandle,
  IN EFI_HANDLE                                            ChildHandle, OPTIONAL
  IN UINT32                                                DefaultType,
  OUT EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED *           ActionRequired
  )
{
  return EFI_SUCCESS;
}

/* Protocol structure definition and initialization */

EFI_DRIVER_CONFIGURATION_PROTOCOL gGigUndiDriverConfiguration = {
  GigUndiDriverConfigurationSetOptions,
  GigUndiDriverConfigurationOptionsValid,
  GigUndiDriverConfigurationForceDefaults,
  "eng"
};
