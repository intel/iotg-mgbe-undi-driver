/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Intelgbe.h"

#include "Dma.h"

/** Convert bytes to pages

    @param[in]  Bytes         Number of bytes

    @retval                   Number of pages fitting the given number of bytes
**/
UINTN
BytesToPages (
  UINTN       Bytes
  )
{
  UINTN   Result;

  Result = Bytes & 0xFFF ? 1 : 0;
  Result += Bytes >> 12;

  DEBUGPRINT (DMA, ("Bytes: %d, Pages: %d\n", Bytes, Result));

  return Result;
}

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
  )
{
  EFI_STATUS    Status;
  UINTN         RequestedSize;

  if (PciIo == NULL || DmaMapping == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (DmaMapping->Size == 0) {
    return EFI_INVALID_PARAMETER;
  }

  Status = PciIo->AllocateBuffer (
             PciIo,
             AllocateAnyPages,
             EfiBootServicesData,
             BytesToPages (DmaMapping->Size),
             (VOID **) &DmaMapping->UnmappedAddress,
             0
             );

  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("Failed to allocate DMA buffer: %r\n", Status));
    DmaMapping->Size = 0;
  }

  RequestedSize = DmaMapping->Size;

  Status = UndiDmaMapCommonBuffer (PciIo, DmaMapping);

  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("DMA buffer mapping failed with: %r\n", Status));
    DEBUGWAIT (CRITICAL);
    goto FREE_BUF_ON_ERROR;
  }

  if (DmaMapping->Size != RequestedSize) {
    DEBUGPRINT (CRITICAL,
      ("Failed to map whole DMA area. Requested: %d, Obtained: %d\n",
        RequestedSize, DmaMapping->Size));
    DEBUGWAIT (CRITICAL);
    Status = EFI_OUT_OF_RESOURCES;
    goto UNMAP_ON_ERROR;
  }

  DEBUGPRINT (DMA,
    ("DMA allocation OK. VA: %lX, PA: %lX, Size: %d, Mapping: %x\n",
    DmaMapping->UnmappedAddress,
    DmaMapping->PhysicalAddress,
    DmaMapping->Size,
    DmaMapping->Mapping
    ));
  DEBUG ((DEBUG_ERROR,
    "DMA allocation OK. VA: %lX, PA: %lX, Size: %d, Mapping: %x\n",
    DmaMapping->UnmappedAddress,
    DmaMapping->PhysicalAddress,
    DmaMapping->Size,
    DmaMapping->Mapping
    ));

  DEBUGWAIT (DMA);

  return EFI_SUCCESS;

UNMAP_ON_ERROR:
  UndiDmaUnmapMemory (PciIo, DmaMapping);

FREE_BUF_ON_ERROR:
  PciIo->FreeBuffer (
           PciIo,
           BytesToPages (DmaMapping->Size),
           (VOID *) DmaMapping->UnmappedAddress
           );
  DmaMapping->Size = 0;
  DmaMapping->UnmappedAddress = 0;

  return Status;
}

/** Free DMA common buffer

    @param[in]  PciIo         Pointer to PCI IO protocol installed on controller
                              handle.
    @param[in]  DmaMapping    Pointer to DMA mapping structure (previously
                              filled by allocation function)

    @retval     EFI_INVALID_PARAMETER   Bad arguments provided.
    @retval     EFI_SUCCESS             Deallocation succeeded.
**/
EFI_STATUS
UndiDmaFreeCommonBuffer (
  EFI_PCI_IO_PROTOCOL       *PciIo,
  UNDI_DMA_MAPPING          *DmaMapping
  )
{
  EFI_STATUS    Status;

  if (PciIo == NULL || DmaMapping == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (DmaMapping->Size == 0 || DmaMapping->UnmappedAddress == 0 ||
      DmaMapping->PhysicalAddress == 0) {
    return EFI_INVALID_PARAMETER;
  }

  Status = UndiDmaUnmapMemory (PciIo, DmaMapping);

  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("Could not unmap DMA memory: %r\n", Status));
    DEBUGWAIT (CRITICAL);
    return Status;
  }

  PciIo->FreeBuffer (
           PciIo,
           BytesToPages (DmaMapping->Size),
           (VOID *) DmaMapping->UnmappedAddress
           );

  DmaMapping->UnmappedAddress = 0;
  DmaMapping->Size = 0;

  return EFI_SUCCESS;
}

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
  )
{
  if (PciIo == NULL || DmaMapping == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (DmaMapping->Size == 0 || DmaMapping->UnmappedAddress == 0) {
    return EFI_INVALID_PARAMETER;
  }

  return PciIo->Map (
                  PciIo,
                  EfiPciIoOperationBusMasterCommonBuffer,
                  (VOID *) DmaMapping->UnmappedAddress,
                  &DmaMapping->Size,
                  &DmaMapping->PhysicalAddress,
                  &DmaMapping->Mapping
                  );
}

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
  )
{
  if (PciIo == NULL || DmaMapping == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (DmaMapping->Size == 0 || DmaMapping->UnmappedAddress == 0) {
    return EFI_INVALID_PARAMETER;
  }

  return PciIo->Map (
                  PciIo,
                  EfiPciIoOperationBusMasterRead,
                  (VOID *) DmaMapping->UnmappedAddress,
                  &DmaMapping->Size,
                  &DmaMapping->PhysicalAddress,
                  &DmaMapping->Mapping
                  );
}

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
  )
{
  EFI_STATUS    Status;

  if (PciIo == NULL || DmaMapping == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = PciIo->Unmap (PciIo, DmaMapping->Mapping);

  if (Status == EFI_SUCCESS) {
    DmaMapping->PhysicalAddress = 0;
    DmaMapping->Mapping = NULL;
  }

  return Status;
}
