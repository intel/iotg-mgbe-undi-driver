/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Intelgbe.h"

#include "ComponentName.h"
#include "DeviceSupport.h"

/* Protocol structures tentative definitions */
EFI_COMPONENT_NAME_PROTOCOL  gUndiComponentName;
EFI_COMPONENT_NAME2_PROTOCOL gUndiComponentName2;

/* To differentiate between standard and open source versions .1 suffix has been added
 to open source version number */
CHAR16 * mDriverNameFormat = L"Intel(R) GbE Driver %1d.%1d.%02d";


// Version and Branding Information
CHAR16 DriverNameString[50];

STATIC EFI_UNICODE_STRING_TABLE mUndiDriverNameTable[] = {
  { "eng", DriverNameString},
  { "en-US", DriverNameString},
  { NULL, NULL}
};



/** Searches through the branding string list for the best possible match for the controller
   associated with UndiPrivateData.

   @param[in,out]   UndiPrivateData   Driver private data structure

   @return    Controller name initialized according to device
**/
VOID
ComponentNameInitializeControllerName (
  UNDI_PRIVATE_DATA *UndiPrivateData
  )
{
  CHAR16 *BrandingString;

  DEBUGPRINT (INIT, ("ComponentNameInitializeDriverAndControllerName\n"));

  BrandingString = GetDeviceBrandingString (UndiPrivateData);

  if (BrandingString != NULL) {
    UndiPrivateData->Brand = BrandingString;
    AddUnicodeString2 (
      "eng",
      gUndiComponentName.SupportedLanguages,
      &UndiPrivateData->ControllerNameTable,
      BrandingString,
      TRUE
    );

    AddUnicodeString2 (
      "en-US",
      gUndiComponentName2.SupportedLanguages,
      &UndiPrivateData->ControllerNameTable,
      BrandingString,
      FALSE
    );
  }
}

/** Retrieves a Unicode string that is the user readable name of the EFI Driver.

   @param[in]   This   A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
   @param[in]   Language   A pointer to a three character ISO 639-2 language identifier.
                           This is the language of the driver name that that the caller
                           is requesting, and it must match one of the languages specified
                           in SupportedLanguages.  The number of languages supported by a
                           driver is up to the driver writer.
   @param[out]   DriverName   A pointer to the Unicode string to return.  This Unicode string
                              is the name of the driver specified by This in the language
                              specified by Language.

   @retval   EFI_SUCCES             The Unicode string for the Driver specified by This
                                    and the language specified by Language was returned
                                    in DriverName.
   @retval   EFI_INVALID_PARAMETER  Language is NULL.
   @retval   EFI_INVALID_PARAMETER  DriverName is NULL.
   @retval   EFI_UNSUPPORTED        The driver specified by This does not support the
                                    language specified by Language.
**/
EFI_STATUS
EFIAPI
ComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL *This,
  IN  CHAR8 *                      Language,
  OUT CHAR16 **                    DriverName
  )
{
  UnicodeSPrint (
    DriverNameString,
    sizeof (DriverNameString),
    mDriverNameFormat,
    MAJORVERSION,
    MINORVERSION,
    BUILDNUMBER
  );

  if (This == NULL) {

    // This is a special case when function is being called by driver itself
    *DriverName = DriverNameString;
    return EFI_SUCCESS;
  }

  return LookupUnicodeString2 (
           Language,
           This->SupportedLanguages,
           mUndiDriverNameTable,
           DriverName,
           (BOOLEAN) (This == &gUndiComponentName)
         );
}

/** Retrieves a Unicode string that is the user readable name of the controller
   that is being managed by an EFI Driver.

   @param[in]   This   A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
   @param[in]   ControllerHandle   The handle of a controller that the driver specified by
                                   This is managing.  This handle specifies the controller
                                   whose name is to be returned.
   @param[in]   ChildHandle   The handle of the child controller to retrieve the name
                              of.  This is an optional parameter that may be NULL.  It
                              will be NULL for device drivers.  It will also be NULL
                              for a bus drivers that wish to retrieve the name of the
                              bus controller.  It will not be NULL for a bus driver
                              that wishes to retrieve the name of a child controller.
   @param[in]   Language  A pointer to a three character ISO 639-2 language
                          identifier.  This is the language of the controller name
                          that that the caller is requesting, and it must match one
                          of the languages specified in SupportedLanguages.  The
                          number of languages supported by a driver is up to the
                          driver writer.
   @param[out]  ControllerName   A pointer to the Unicode string to return.  This Unicode
                                  string is the name of the controller specified by
                                  ControllerHandle and ChildHandle in the language specified
                                  by Language from the point of view of the driver specified
                                  by This.

   @retval   EFI_SUCCESS            The Unicode string for the user readable name in the
                                    language specified by Language for the driver
                                    specified by This was returned in DriverName.
   @retval   EFI_INVALID_PARAMETER  ControllerHandle is not a valid EFI_HANDLE.
   @retval   EFI_INVALID_PARAMETER  ChildHandle is not NULL and it is not a valid EFI_HANDLE.
   @retval   EFI_INVALID_PARAMETER  Language is NULL.
   @retval   EFI_INVALID_PARAMETER  ControllerName is NULL.
   @retval   EFI_UNSUPPORTED        The driver specified by This is not currently managing
                                    the controller specified by ControllerHandle and
                                    ChildHandle.
   @retval   EFI_UNSUPPORTED        The driver specified by This does not support the
                                    language specified by Language.
**/
EFI_STATUS
EFIAPI
ComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL *                               This,
  IN  EFI_HANDLE                                                  ControllerHandle,
  IN  EFI_HANDLE                                      ChildHandle OPTIONAL,
  IN  CHAR8 *                                                     Language,
  OUT CHAR16 **                                                   ControllerName
  )
{
  EFI_NII_POINTER_PROTOCOL *NiiPointerProtocol;
  EFI_DEVICE_PATH_PROTOCOL *UndiDevicePath;
  UNDI_PRIVATE_DATA *       UndiPrivateData;
  EFI_STATUS                Status;

  if ((NULL == Language)
    || (NULL == ControllerHandle))
  {
    return EFI_INVALID_PARAMETER;
  }

  // Make sure this driver is currently managing ControllerHandle
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID * *) &UndiDevicePath,
                  gUndiDriverBinding.DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                );

  if (Status != EFI_ALREADY_STARTED) {
    gBS->CloseProtocol (
           ControllerHandle,
           &gEfiDevicePathProtocolGuid,
           gUndiDriverBinding.DriverBindingHandle,
           ControllerHandle
         );
    return EFI_UNSUPPORTED;
  }

  // Make sure this driver is currently managing ControllerHandle
  if (ChildHandle == NULL) {

    // Don't allow NULL ChildHandle here
    return EFI_UNSUPPORTED;
  } else {
    Status = gBS->OpenProtocol (
                    ChildHandle,
                    &gEfiNiiPointerGuid,
                    (VOID * *) &NiiPointerProtocol,
                    gUndiDriverBinding.DriverBindingHandle,
                    ChildHandle,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

    if (EFI_ERROR (Status)) {
      return Status;
    }

    UndiPrivateData = UNDI_PRIVATE_DATA_FROM_THIS (NiiPointerProtocol->NiiProtocol31);

    return LookupUnicodeString2 (
             Language,
             This->SupportedLanguages,
             UndiPrivateData->ControllerNameTable,
             ControllerName,
             (BOOLEAN) (This == &gUndiComponentName)
           );
  }
}

/* Component Name protocol structures definition and initialization */

EFI_COMPONENT_NAME_PROTOCOL gUndiComponentName = {
  ComponentNameGetDriverName,
  ComponentNameGetControllerName,
  "eng"
};

EFI_COMPONENT_NAME2_PROTOCOL gUndiComponentName2 = {
  (EFI_COMPONENT_NAME2_GET_DRIVER_NAME) ComponentNameGetDriverName,
  (EFI_COMPONENT_NAME2_GET_CONTROLLER_NAME) ComponentNameGetControllerName,
  "en-US"
};

/* Supported EFI version protocol structures definition and initialization */

EFI_DRIVER_SUPPORTED_EFI_VERSION_PROTOCOL gUndiSupportedEfiVersion = {
  sizeof (EFI_DRIVER_SUPPORTED_EFI_VERSION_PROTOCOL),
  EFI_2_70_SYSTEM_TABLE_REVISION
};
