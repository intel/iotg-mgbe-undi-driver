/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef COMPONENT_NAME_H_
#define COMPONENT_NAME_H_

#define EFI_2_70_SYSTEM_TABLE_REVISION  ((2 << 16) | (70))


/** Searches through the branding string list for the best possible match for the controller
   associated with UndiPrivateData.

   @param[in,out]   UndiPrivateData   Driver private data structure

   @return    Controller name initialized according to device
**/
VOID
ComponentNameInitializeControllerName (
  UNDI_PRIVATE_DATA *UndiPrivateData
  );

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
  );

#endif /* COMPONENT_NAME_H_ */
