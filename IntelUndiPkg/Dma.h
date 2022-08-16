/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _DMA_H_
#define _DMA_H_

#include <Uefi.h>
#include <Protocol/PciIo.h>

// Structure for DMA common buffer mapping
typedef struct _UNDI_DMA_MAPPING {
  EFI_VIRTUAL_ADDRESS     UnmappedAddress;
  EFI_PHYSICAL_ADDRESS    PhysicalAddress;
  UINTN                   Size;
  VOID                    *Mapping;
} UNDI_DMA_MAPPING;

/** Convert bytes to pages

    @param[in]  Bytes         Number of bytes

    @retval                   Number of pages fitting the given number of bytes
**/
UINTN
BytesToPages (
  UINTN       Bytes
  );

/** Allocate DMA common buffer (aligned to the page)

    @param[in]  PciIo         Pointer to PCI IO protocol installed on controller
                              handle.
    @param[in]  DmaMapping    Pointer to DMA mapping structure. Size must be filled in.

    @retval     EFI_INVALID_PARAMETER   Bad arguments provided.
    @retval     EFI_OUT_OF_RESOURCES    Failed to map whole requested area
    @retval     EFI_SUCCESS             Allocation succeeded.
**/
EFI_STATUS
UndiDmaAllocateCommonBuffer (
  EFI_PCI_IO_PROTOCOL       *PciIo,
  UNDI_DMA_MAPPING          *DmaMapping
  );

/** Free DMA common buffer

    @param[in]  PciIo         Pointer to PCI IO protocol installed on controller
                              handle.
    @param[in]  DmaMapping    Pointer to DMA mapping structure (previously
                              filled by allocation function.

    @retval     EFI_INVALID_PARAMETER   Bad arguments provided.
    @retval     EFI_SUCCESS             Deallocation succeeded.
**/
EFI_STATUS
UndiDmaFreeCommonBuffer (
  EFI_PCI_IO_PROTOCOL       *PciIo,
  UNDI_DMA_MAPPING          *DmaMapping
  );

/** Map DMA buffer as common buffer (read/write)

    @param[in]  PciIo         Pointer to PCI IO protocol installed on controller
                              handle.
    @param[in]  DmaMapping    Pointer to DMA mapping structure (previously
                              filled by allocation function)

    @retval     EFI_INVALID_PARAMETER   Bad arguments provided.
    @retval     EFI_SUCCESS             Mapping succeeded.
**/
EFI_STATUS
UndiDmaMapCommonBuffer (
  EFI_PCI_IO_PROTOCOL       *PciIo,
  UNDI_DMA_MAPPING          *DmaMapping
  );

/** Map DMA buffer for read operations

    @param[in]  PciIo         Pointer to PCI IO protocol installed on controller
                              handle.
    @param[in]  DmaMapping    Pointer to DMA mapping structure (previously
                              filled by allocation function)

    @retval     EFI_INVALID_PARAMETER   Bad arguments provided.
    @retval     EFI_SUCCESS             Mapping succeeded.
**/
EFI_STATUS
UndiDmaMapMemoryRead (
  EFI_PCI_IO_PROTOCOL       *PciIo,
  UNDI_DMA_MAPPING          *DmaMapping
  );

/** Unmap DMA buffer

    @param[in]  PciIo         Pointer to PCI IO protocol installed on controller
                              handle.
    @param[in]  DmaMapping    Pointer to DMA mapping structure (previously
                              filled by allocation function)

    @retval     EFI_INVALID_PARAMETER   Bad arguments provided.
    @retval     EFI_SUCCESS             Unmapping succeeded.
**/
EFI_STATUS
UndiDmaUnmapMemory (
  EFI_PCI_IO_PROTOCOL       *PciIo,
  UNDI_DMA_MAPPING          *DmaMapping
  );

#endif /* _DMA_H_ */
