/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Init.h"
#include "Intelgbe.h"

/* Global variables for blocking IO */
STATIC BOOLEAN  mInitializeLock = TRUE;
STATIC EFI_LOCK mLock;

VOID IntelgbeRegsDump(GIG_DRIVER_DATA *GigAdapter)
{
#if DBG_LVL
  u32 k;

  for (k=0; k<GigAdapter->rxqnum; k++) {
    struct intelgbe_rx_queue *rx_q = &GigAdapter->rx_queue[k];
    UINT32 i;
    struct intelgbe_tx_queue       *tx_q = &GigAdapter->tx_queue[k];
    INTELGBE_TRANSMIT_DESCRIPTOR *p =
    (INTELGBE_TRANSMIT_DESCRIPTOR *)tx_q->dma_tx;
    DEBUGPRINT (CRITICAL, ("TX descriptor ring: %d\n", k));
    DEBUGPRINT (CRITICAL, ("curr=%d \n", tx_q->cur_tx));
    for (i = 0; i < DEFAULT_TX_DESCRIPTORS; i++) {
      UNDI_DMA_MAPPING *TxBufMapping = &GigAdapter->TxBufferMappings[i];
      DEBUGPRINT (CRITICAL, ("%03d [0x%x]: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                        i, (UINT64)p,
                        (p->des0), (p->des1),
                        (p->des2), (p->des3),
                        (TxBufMapping->UnmappedAddress),
                        (TxBufMapping->PhysicalAddress)));
      p++;
    }
    DEBUGPRINT (CRITICAL, ("RX descriptor ring: %d\n", k));
    DEBUGPRINT (CRITICAL, ("curr=%d \n", rx_q->cur_rx));
    INTELGBE_RECEIVE_DESCRIPTOR *rp =
    (INTELGBE_RECEIVE_DESCRIPTOR *)rx_q->dma_rx;
    for (i = 0; i < DEFAULT_RX_DESCRIPTORS; i++) {
      DEBUGPRINT (CRITICAL, ("%03d %d [0x%x]: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                        i, sizeof(*rp), (UINT64)rp,
                        (rp->des0), (rp->des1),
                        (rp->des2), (rp->des3),
                        (&rx_q->rx_buff[i]),
                        (&rx_q->dma_rx_buff[i])));
        rp++;
    }

    DEBUGPRINT (CRITICAL, ("DMA_RX_CONTROL_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, DMA_RX_CONTROL_CH(rx_q->chan))));
    DEBUGPRINT (CRITICAL, ("DMA_CONTROL_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, ((0x0080*rx_q->chan)+0x1100))));
    DEBUGPRINT (CRITICAL, ("DMA_RXDESC_LIST_ADDR_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, DMA_RXDESC_LIST_ADDR_CH(rx_q->chan))));
    DEBUGPRINT (CRITICAL, ("DMA_CURR_RXDESC_LIST_ADDR_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, ((0x0080*rx_q->chan)+0x114c))));
    DEBUGPRINT (CRITICAL, ("DMA_CURR_RXDESC_BUFF_ADDR_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, ((0x0080*rx_q->chan)+0x115c))));
    DEBUGPRINT (CRITICAL, ("DMA_CURR_RXDESC_BUFF_ADDR_H_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, ((0x0080*rx_q->chan)+0x1158))));
    DEBUGPRINT (CRITICAL, ("DMA_RXDESC_TAIL_PTR_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, DMA_RXDESC_TAIL_PTR_CH(rx_q->chan))));
    DEBUGPRINT (CRITICAL, ("DMA_RXDESC_RING_LENGTH_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw,
        DMA_RXDESC_RING_LENGTH_CH(rx_q->chan))));
    DEBUGPRINT (CRITICAL, ("MTL_Q0_Interrupt_Control_Status %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, ((0x0040*rx_q->chan)+0x0D2C))));
    DEBUGPRINT (CRITICAL, ("MTL_RxQ0_Operation_Mode %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, ((0x0040*rx_q->chan)+0x0D30))));
    DEBUGPRINT (CRITICAL, ("MTL_RxQ0_Debug %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, ((0x0040*rx_q->chan)+0x0D38))));
    DEBUGPRINT (CRITICAL, ("MAC_RxQ_Ctrl0 %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, 0xa0)));
    DEBUGPRINT (CRITICAL, ("MAC_DEBUG %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, 0x114)));
    DEBUGPRINT (CRITICAL, ("MAC_Rx_Tx_Status %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, 0xb8)));

    DEBUGPRINT (CRITICAL, ("DMA_TX_CONTROL_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, DMA_TX_CONTROL_CH(k))));
    DEBUGPRINT (CRITICAL, ("MTL_TXQ_OPERATION_MODE(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, MTL_TXQ_OPERATION_MODE(k))));
    DEBUGPRINT (CRITICAL, ("DMA_TXDESC_LIST_ADDR_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, DMA_TXDESC_LIST_ADDR_CH(k))));
    DEBUGPRINT (CRITICAL, ("DMA_CURR_TXDESC_LIST_ADDR_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, ((0x0080*k)+0x1144))));
    DEBUGPRINT (CRITICAL, ("DMA_CURR_TXDESC_BUFF_ADDR_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, ((0x0080*k)+0x1154))));
    DEBUGPRINT (CRITICAL, ("DMA_TXDESC_TAIL_PTR_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, DMA_TXDESC_TAIL_PTR_CH(k))));
    DEBUGPRINT (CRITICAL, ("DMA_TXDESC_RING_LENGTH_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, DMA_TXDESC_RING_LENGTH_CH(k))));
    DEBUGPRINT (CRITICAL, ("DMA_INTR_EN_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, DMA_INTR_EN_CH(k))));
    DEBUGPRINT (CRITICAL, ("DMA_SYSBUS_MODE %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, DMA_SYSBUS_MODE)));
    DEBUGPRINT (CRITICAL, ("MAC_CONFIGURATION %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, MAC_CONFIGURATION)));
    DEBUGPRINT (CRITICAL, ("MTL_OPERATION_MODE %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, MTL_OPERATION_MODE)));
    DEBUGPRINT (CRITICAL, ("DMA_Interrupt_Status %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, 0x1008)));
    DEBUGPRINT (CRITICAL, ("DMA_Debug_Status0 %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, 0x100c)));
    DEBUGPRINT (CRITICAL, ("MTL_DBG_CTL %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, 0xc08)));
    DEBUGPRINT (CRITICAL, ("MTL_DBG_STS %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, 0xc0c)));
    DEBUGPRINT (CRITICAL, ("MTL_INTR_STS %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, 0xc20)));
    DEBUGPRINT (CRITICAL, ("MTL_RxQ_DMA_Map0 %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, 0xc30)));
    DEBUGPRINT (CRITICAL, ("MTL_RxQ_DMA_Map1 %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, 0xc34)));
    DEBUGPRINT (CRITICAL, ("DMA_INTR_STATUS_CH(0) %x\n",
      INTELGBE_READ_REG(&GigAdapter->Hw, DMA_INTR_STATUS_CH(rx_q->chan))));
  }
#endif
  return;
}

/** Implements IO blocking when reading DMA memory.

   @param[in]   GigAdapter   Pointer to the NIC data structure information
                             which the UNDI driver is layering on.
   @param[in]   Flag         Block flag

   @return    Lock is acquired or released according to Flag
**/
VOID
IntelgbeBlockIt (
  IN GIG_DRIVER_DATA *GigAdapter,
  UINT32              Flag
  )
{
  if (GigAdapter->Block != NULL) {
    (*GigAdapter->Block) (GigAdapter->UniqueId, Flag);
  } else {
    if (mInitializeLock) {
      EfiInitializeLock (&mLock, TPL_NOTIFY);
      mInitializeLock = FALSE;
    }

    if (Flag != 0) {
      EfiAcquireLock (&mLock);
    } else {
      EfiReleaseLock (&mLock);
    }
  }
}

/** Delay a specified number of microseconds

   @param[in]   Adapter        Pointer to the NIC data structure information
                               which the UNDI driver is layering on..
   @param[in]   MicroSeconds   Time to delay in Microseconds.

   @return   Execution of code delayed
**/
VOID
DelayInMicroseconds (
  IN GIG_DRIVER_DATA *Adapter,
  IN UINTN               MicroSeconds
  )
{
  gBS->Stall (MicroSeconds);
}

/** Detects surprise removal device status in PCI controller register

   @param[in]   Adapter   Pointer to the device instance

   @retval   TRUE    Surprise removal has been detected
   @retval   FALSE   Surprise removal has not been detected
**/
BOOLEAN
IsSurpriseRemoval (
  IN  GIG_DRIVER_DATA *Adapter
  )
{
  return FALSE;
}

/* Maps a virtual address to a physical address.  This is necessary for runtime functionality and also
   on some platforms that have a chipset that cannot allow DMAs above 4GB.

   @param[in]   GigAdapter   Pointer to the NIC data structure information
                             which the UNDI driver is layering on.
   @param[in]   VirtualAddress   Virtual Address of the data buffer to map.
   @param[in]   Size             Minimum size of the buffer to map.
   @param[out]  MappedAddress    Pointer to store the address of the mapped buffer.

   @return    Virtual address mapped to physical
**/
VOID
IntelgbeMapMem (
  IN GIG_DRIVER_DATA *GigAdapter,
  IN UINT64           VirtualAddress,
  IN UINT32           Size,
  OUT UINTN *         MappedAddress
  )
{
  if (GigAdapter->MapMem != NULL) {
    (*GigAdapter->MapMem) (
                    GigAdapter->UniqueId,
                    VirtualAddress,
                    Size,
                    TO_DEVICE,
                    (UINT64) MappedAddress
                  );

    if (*MappedAddress == 0) {
      *((UINTN *) MappedAddress) = (UINTN) VirtualAddress;
    }
  } else {
    *((UINTN *) MappedAddress) = (UINTN) VirtualAddress;
  }
}

/** Free TX buffers that have been transmitted by the hardware.

   @param[in]   GigAdapter   Pointer to the NIC data structure information
                             which the UNDI driver is layering on.
   @param[in]   NumEntries   Number of entries in the array which can be freed.
   @param[out]  TxBuffer     Array to pass back free TX buffer

   @return   Number of TX buffers written.
**/
UINT16
IntelgbeFreeTxBuffers (
  IN GIG_DRIVER_DATA *GigAdapter,
  IN UINT16           NumEntries,
  OUT UINT64 *        TxBuffer
  )
{
  struct intelgbe_tx_queue  *tx_q = &GigAdapter->tx_queue[0];
  UINT32                     entry;
  UINT16                     count = 0;
  UNDI_DMA_MAPPING          *TxBufMapping;

  DEBUGPRINT (DECODE, ("INTELGBEFreeTxBuffers cur %d dirty %d NumEntries %d\n",
    tx_q->cur_tx, tx_q->dirty_tx, NumEntries));

  entry = tx_q->dirty_tx;
  while ((entry != tx_q->cur_tx) && (count < NumEntries)) {
    INTELGBE_TRANSMIT_DESCRIPTOR *p = &tx_q->tx_desc[entry];
    TxBufMapping = &GigAdapter->TxBufferMappings[entry];
    UINT32 tdes3 = p->des3;
    if (tdes3 & BIT(31)) {
      DEBUGPRINT (INTELGBE, ("TX desc busy\n"));
      break;
    }
    if (tdes3 & BIT(28)) {
      if (tdes3 & BIT(15)) {
        DEBUGPRINT (CRITICAL, ("TX Error\n"));
      }
    }
    if (TxBufMapping->UnmappedAddress == 0) {
      DEBUGPRINT (CRITICAL,
        ("ERROR: TX buffer complete without being marked used!\n"));
      break;
    }
    UndiDmaUnmapMemory (GigAdapter->PciIo, TxBufMapping);

    TxBuffer[count] = TxBufMapping->UnmappedAddress;
    count++;

    ZeroMem (TxBufMapping, sizeof (UNDI_DMA_MAPPING));

    p->des0 = 0;
    p->des1 = 0;
    p->des2 = 0;
    p->des3 = 0;
    entry = (entry +1) & (DEFAULT_TX_DESCRIPTORS -1);
  }
  tx_q->dirty_tx = entry;

  return count;
}

/** Takes a command Block pointer (cpb) and sends the frame.  Takes either one fragment or many
   and places them onto the wire.  Cleanup of the send happens in the function UNDI_Status in DECODE.C

   @param[in]   GigAdapter   Pointer to the instance data
   @param[in]   Cpb       The command parameter Block address.  64 bits since this is Itanium(tm)
                          processor friendly
   @param[in]   OpFlags   The operation flags, tells if there is any special sauce on this transmit

   @retval   PXE_STATCODE_SUCCESS        If the frame goes out
   @retval   PXE_STATCODE_QUEUE_FULL     Transmit buffers aren't freed by upper layer
   @retval   PXE_STATCODE_DEVICE_FAILURE Frame failed to go out
   @retval   PXE_STATCODE_BUSY           If they need to call again later
**/
UINTN
IntelgbeTransmit (
  GIG_DRIVER_DATA *GigAdapter,
  UINT64           Cpb,
  UINT16           OpFlags
  )
{
  struct intelgbe_tx_queue       *tx_q = &GigAdapter->tx_queue[0];
  PXE_CPB_TRANSMIT_FRAGMENTS *TxFrags;
  PXE_CPB_TRANSMIT *          TxBuffer;
  INTELGBE_TRANSMIT_DESCRIPTOR *desc, *first;
  EFI_STATUS                  Status;
  UNDI_DMA_MAPPING            *TxBufMapping;
  UINT32 entry, avail, needed_descs;
  UINT32 i;

  TxBufMapping = &GigAdapter->TxBufferMappings[tx_q->cur_tx];

  if (tx_q->dirty_tx > tx_q->cur_tx)
    avail = tx_q->dirty_tx - tx_q->cur_tx - 1;
  else
    avail = DEFAULT_TX_DESCRIPTORS - tx_q->cur_tx +
                 tx_q->dirty_tx - 1;
 // Make some short cut pointers so we don't have to worry about typecasting later.
  // If the TX has fragments we will use the
  // tx_tpr_f pointer, otherwise the tx_ptr_l (l is for linear)
  TxBuffer  = (PXE_CPB_TRANSMIT *) (UINTN) Cpb;
  TxFrags   = (PXE_CPB_TRANSMIT_FRAGMENTS *) (UINTN) Cpb;
  needed_descs = (TxFrags->FragCnt == 0)? 1 : TxFrags->FragCnt;
  // Transmit buffers must be freed by the upper layer before we can transmit any more.
  if (avail < needed_descs) {
    DEBUGWAIT (CRITICAL);
    // According to UEFI spec we should return PXE_STATCODE_BUFFER_FULL,
    // but SNP is not implemented to recognize this callback.
    return PXE_STATCODE_QUEUE_FULL;
  }

  entry = tx_q->cur_tx;
  // quicker pointer to the next available Tx descriptor to use.
  desc = &tx_q->tx_desc[entry];
  tx_q->cur_tx++;
  if (tx_q->cur_tx >= DEFAULT_TX_DESCRIPTORS) {
    tx_q->cur_tx = 0;
  }

  // Opflags will tell us if this Tx has fragments
  // So far the linear case (the no fragments case, the else on this if) is the majority
  // of all frames sent.
  if (OpFlags & PXE_OPFLAGS_TRANSMIT_FRAGMENTED) {
      // this count cannot be more than 8;
      DEBUGPRINT (CRITICAL, ("Fragments %x\n", TxFrags->FragCnt));
      first = desc;
    // for each fragment, give it a descriptor, being sure to keep track of the number used.
    for (i = 1; i < TxFrags->FragCnt; i++) {
      desc = &tx_q->tx_desc[tx_q->cur_tx];
      tx_q->cur_tx++;
      if (tx_q->cur_tx >= DEFAULT_TX_DESCRIPTORS) {
        tx_q->cur_tx = 0;
      }
      // Put the size of the fragment in the descriptor
      IntelgbeMapMem (
        GigAdapter,
        TxFrags->FragDesc[i].FragAddr,
        TxFrags->FragDesc[i].FragLen,
        (UINTN *) &desc->des0
      );
      desc->des1 = 0;
      desc->des2 = TxFrags->FragDesc[i].FragLen;

      UINT32 tdes3 = desc->des3;
      tdes3 = TxFrags->FrameLen + TxFrags->MediaheaderLen;
      desc->des3 = tdes3;
      if ((i + 1) == TxFrags->FragCnt) {
        desc->des2 |= (BIT(31));
        tdes3 |= BIT(28);
      }
      DEBUGWAIT (INTELGBE);
      tdes3 = desc->des3;
      tdes3 |= BIT(31);
      desc->des3 = tdes3;
    }
    desc = first;
    // Put the size of the fragment in the descriptor
    IntelgbeMapMem (
        GigAdapter,
        TxFrags->FragDesc[0].FragAddr,
        TxFrags->FragDesc[0].FragLen,
        (UINTN *) &desc->des0
    );
    desc->des1 = 0;
    desc->des2 = TxFrags->FragDesc[0].FragLen;

    UINT32 tdes3 = desc->des3;
    tdes3 = TxFrags->FrameLen + TxFrags->MediaheaderLen;
    desc->des3 = tdes3;
    tdes3 |= BIT(29);
    DEBUGWAIT (INTELGBE);
    tdes3 = desc->des3;
    tdes3 |= BIT(31);
    desc->des3 = tdes3;

    tx_q->tx_tail_addr = (UINT32)(UINT64)tx_q->dma_tx +
                         (tx_q->cur_tx * sizeof(INTELGBE_TRANSMIT_DESCRIPTOR));
    INTELGBE_WRITE_REG (&GigAdapter->Hw, DMA_TXDESC_TAIL_PTR_CH(0),
      tx_q->tx_tail_addr);
  } else {
    TxBufMapping->UnmappedAddress = TxBuffer->FrameAddr;
    TxBufMapping->Size = TxBuffer->DataLen + TxBuffer->MediaheaderLen;
    UINTN RequestedSize = TxBufMapping->Size;
    Status = UndiDmaMapMemoryRead (
               GigAdapter->PciIo,
               TxBufMapping
               );
    if (Status != EFI_SUCCESS) {
      DEBUGPRINT (INTELGBE, ("TX DMA mapping failed\n"));
    }

    if (TxBufMapping->Size != RequestedSize) {
      DEBUGPRINT (CRITICAL, ("Failed to map whole DMA area. \
        Requested: %d, Obtained: %d\n", RequestedSize, TxBufMapping->Size));
      DEBUGWAIT (CRITICAL);
    }

    desc->des0 = (UINT32)TxBufMapping->PhysicalAddress;
    desc->des1 = 0;
    desc->des2 = (UINT32)TxBufMapping->Size;
    desc->des2 |= (BIT(31));

    UINT32 tdes3 = desc->des3;
    tdes3 = (UINT32)TxBufMapping->Size;
    tdes3 |= BIT(29);
    tdes3 |= BIT(28);
    desc->des3 = tdes3;
    DEBUGWAIT (INTELGBE);
    tdes3 = desc->des3;
    tdes3 |= BIT(31);
    desc->des3 = tdes3;
    tx_q->tx_tail_addr = (UINT32)(UINT64)tx_q->dma_tx +
                          (tx_q->cur_tx * sizeof(INTELGBE_TRANSMIT_DESCRIPTOR));
    INTELGBE_WRITE_REG (&GigAdapter->Hw, DMA_TXDESC_TAIL_PTR_CH(0),
      tx_q->tx_tail_addr);
  }
 // If the OpFlags tells us to wait for the packet to hit the wire, we will wait.
  if ((OpFlags & PXE_OPFLAGS_TRANSMIT_BLOCK) != 0) {
      DEBUGPRINT (INTELGBE, ("Wait for Transmit to complete\n"));
  }
  return PXE_STATCODE_SUCCESS;
}

/** Initializes the gigabit adapter, setting up memory addresses, MAC Addresses,
   Type of card, etc.

   @param[in]   GigAdapter   Pointer to adapter structure

   @retval   PXE_STATCODE_SUCCESS       Initialization succeeded
   @retval   PXE_STATCODE_NOT_STARTED   Hardware Init failed
**/
PXE_STATCODE
IntelgbeInititialize (
  GIG_DRIVER_DATA *GigAdapter
  )
{
  PXE_STATCODE PxeStatcode;

  DEBUGPRINT (INTELGBE, ("IntelgbeInititialize\n"));

  PxeStatcode = PXE_STATCODE_SUCCESS;
  DEBUGWAIT (INTELGBE);

  //IntelgbeSetSpeedDuplex (GigAdapter);
 // If the hardware has already been initialized then don't bother with a reset
  // We want to make sure we do not have to restart autonegotiation and two-pair
  // downshift.
  if (!GigAdapter->HwInitialized) {
    DEBUGPRINT (INTELGBE, ("Initializing hardware!\n"));

    if (intelgbe_init_hw (&GigAdapter->Hw) == 0) {
      DEBUGPRINT (INTELGBE, ("intelgbe_init_hw success\n"));
      PxeStatcode = PXE_STATCODE_SUCCESS;
      GigAdapter->HwInitialized      = TRUE;
    } else {
      DEBUGPRINT (CRITICAL, ("Hardware Init failed\n"));
      PxeStatcode = PXE_STATCODE_NOT_STARTED;
    }
  } else {
    DEBUGPRINT (INTELGBE, ("Skipping adapter reset\n"));
    PxeStatcode = PXE_STATCODE_SUCCESS;
  }

  // Re-read the MAC address.  The CLP configured MAC address is being reset by
  // hardware to the factory address after init, so we need to reset it here.
  if (intelgbe_read_mac_addr_generic (&GigAdapter->Hw) != INTELGBE_SUCCESS) {
    DEBUGPRINT (CRITICAL, ("Could not read MAC address.\n"));
  }

  DEBUGWAIT (INTELGBE);

  return PxeStatcode;
}

/** This function performs PCI-E initialization for the device.

   @param[in]   GigAdapter   Pointer to adapter structure

   @retval   EFI_SUCCESS            PCI-E initialized successfully
   @retval   EFI_UNSUPPORTED        Failed to get supported PCI command options
   @retval   EFI_UNSUPPORTED        Failed to set PCI command options
   @retval   EFI_OUT_OF_RESOURCES   The memory pages for transmit and receive resources could
                                    not be allocated
**/
EFI_STATUS
IntelgbePciInit (
  GIG_DRIVER_DATA *GigAdapter
  )
{
  EFI_STATUS Status;
  UINT64     Result = 0;
  BOOLEAN    PciAttributesSaved = FALSE;

  // Save original PCI attributes
  Status = GigAdapter->PciIo->Attributes (
                                GigAdapter->PciIo,
                                EfiPciIoAttributeOperationGet,
                                0,
                                &GigAdapter->OriginalPciAttributes
                               );

  if (EFI_ERROR (Status)) {
    goto PciIoError;
  }
  PciAttributesSaved = TRUE;

  // Get the PCI Command options that are supported by this controller.
  Status = GigAdapter->PciIo->Attributes (
                                GigAdapter->PciIo,
                                EfiPciIoAttributeOperationSupported,
                                0,
                                &Result
                              );

  DEBUGPRINT (INIT, ("Attributes supported %x\n", Result));

  if (!EFI_ERROR (Status)) {

    // Set the PCI Command options to enable device memory mapped IO,
    // port IO, and bus mastering.
    Status = GigAdapter->PciIo->Attributes (
                                  GigAdapter->PciIo,
                                  EfiPciIoAttributeOperationEnable,
                                  Result & (EFI_PCI_DEVICE_ENABLE |
                                  EFI_PCI_IO_ATTRIBUTE_DUAL_ADDRESS_CYCLE),
                                  NULL
                                );
  }

  if (EFI_ERROR (Status)) {
    DEBUGPRINT (CRITICAL, ("Attributes returns %r\n", Status));
    goto PciIoError;
  }

  // Allocate common DMA buffer for Tx descriptors
  GigAdapter->TxRing.Size = TX_RING_SIZE;

  Status = UndiDmaAllocateCommonBuffer (
             GigAdapter->PciIo,
             &GigAdapter->TxRing
             );

  if (EFI_ERROR (Status)) {
    goto OnAllocError;
  }

  // Allocate common DMA buffer for Rx descriptors
  GigAdapter->RxRing.Size = RX_RING_SIZE;

  Status = UndiDmaAllocateCommonBuffer (
             GigAdapter->PciIo,
             &GigAdapter->RxRing
             );

  if (EFI_ERROR (Status)) {
    goto OnAllocError;
  }

  // Allocate common DMA buffer for Rx buffers
  GigAdapter->RxBufferMapping.Size = RX_BUFFERS_SIZE;

  Status = UndiDmaAllocateCommonBuffer (
             GigAdapter->PciIo,
             &GigAdapter->RxBufferMapping
             );

  if (EFI_ERROR (Status)) {
    goto OnAllocError;
  }

  ZeroMem (
    (VOID *) GigAdapter->RxRing.UnmappedAddress,
    RX_RING_SIZE
    );

  ZeroMem (
    (VOID *) GigAdapter->TxRing.UnmappedAddress,
    TX_RING_SIZE
    );

  ZeroMem (
    (VOID *) GigAdapter->RxBufferMapping.UnmappedAddress,
    RX_BUFFERS_SIZE
    );

  return EFI_SUCCESS;

OnAllocError:
      if (GigAdapter->TxRing.Mapping != NULL) {
        UndiDmaFreeCommonBuffer (GigAdapter->PciIo, &GigAdapter->TxRing);
      }

      if (GigAdapter->RxRing.Mapping != NULL) {
        UndiDmaFreeCommonBuffer (GigAdapter->PciIo, &GigAdapter->RxRing);
      }

      if (GigAdapter->RxBufferMapping.Mapping != NULL) {
        UndiDmaFreeCommonBuffer (GigAdapter->PciIo,
          &GigAdapter->RxBufferMapping);
      }

PciIoError:
  if (PciAttributesSaved) {

    // Restore original PCI attributes
    GigAdapter->PciIo->Attributes (
                         GigAdapter->PciIo,
                         EfiPciIoAttributeOperationSet,
                         GigAdapter->OriginalPciAttributes,
                         NULL
                       );
  }

    return Status;
}

/** This function is called as early as possible during driver start to ensure the
   hardware has enough time to autonegotiate when the real SNP device initialize call is made.

   @param[in]   GigAdapter   Pointer to adapter structure

   @retval   EFI_SUCCESS        Hardware init success
   @retval   EFI_DEVICE_ERROR   Hardware init failed
   @retval   EFI_UNSUPPORTED    Unsupported MAC type
   @retval   EFI_UNSUPPORTED    intelgbe_setup_init_funcs failed
   @retval   EFI_UNSUPPORTED    Could not read bus information
   @retval   EFI_UNSUPPORTED    Could not read MAC address
   @retval   EFI_ACCESS_DENIED  iSCSI Boot detected on port
   @retval   EFI_DEVICE_ERROR   Failed to reset hardware
**/
EFI_STATUS
IntelgbeFirstTimeInit (
  GIG_DRIVER_DATA *GigAdapter
  )
{
  PCI_CONFIG_HEADER *PciConfigHeader;
  UINT32 *           TempBar;
  UINT8              BarIndex;
  struct intelgbe_hw *hw = &GigAdapter->Hw;
  struct intelgbe_phy_info *phy = &hw->phy;

  GigAdapter->DriverBusy = FALSE;

  // Read all the registers from the device's PCI Configuration space
  GigAdapter->PciIo->Pci.Read (
                           GigAdapter->PciIo,
                           EfiPciIoWidthUint32,
                           0,
                           MAX_PCI_CONFIG_LEN,
                           GigAdapter->PciConfig
                         );

  PciConfigHeader = (PCI_CONFIG_HEADER *) GigAdapter->PciConfig;

  // Enumerate through the PCI BARs for the device to determine which one is
  // the IO BAR.  Save the index of the BAR into the adapter info structure.
  TempBar = &PciConfigHeader->BaseAddressReg0;
  for (BarIndex = 0; BarIndex <= 5; BarIndex++) {
    DEBUGPRINT (INTELGBE, ("BAR = %X\n", *TempBar));
    if ((*TempBar & PCI_BAR_MEM_MASK) == PCI_BAR_MEM_64BIT) {

      // This is a 64-bit memory bar, skip this and the
      // next bar as well.
      TempBar++;
      DEBUGPRINT (INTELGBE, ("skip BAR = %X\n", *TempBar));
      BarIndex++;
    }

    // Find the IO BAR and save it's number into IoBar
    if ((*TempBar & PCI_BAR_IO_MASK) == PCI_BAR_IO_MODE) {

      // Here is the IO Bar - save it to the Gigabit adapter struct.
      GigAdapter->IoBarIndex = BarIndex;
      break;
    }

    // Advance the pointer to the next bar in PCI config space
    TempBar++;
  }

  GigAdapter->PciIo->GetLocation (
                       GigAdapter->PciIo,
                       &GigAdapter->Segment,
                       &GigAdapter->Bus,
                       &GigAdapter->Device,
                       &GigAdapter->Function
                     );

  DEBUGPRINT (INIT, ("GigAdapter->IoBarIndex = %X\n", GigAdapter->IoBarIndex));
  DEBUGPRINT (INIT, ("PCI Command Register = %X\n", PciConfigHeader->Command));
  DEBUGPRINT (INIT, ("PCI Status Register = %X\n", PciConfigHeader->Status));
  DEBUGPRINT (INIT, ("PCI VendorID = %X\n", PciConfigHeader->VendorId));
  DEBUGPRINT (INIT, ("PCI DeviceID = %X\n", PciConfigHeader->DeviceId));
  DEBUGPRINT (INIT, ("PCI SubVendorID = %X\n", PciConfigHeader->SubVendorId));
  DEBUGPRINT (INIT, ("PCI SubSystemID = %X\n", PciConfigHeader->SubSystemId));
  DEBUGPRINT (INIT, ("PCI Segment = %X\n", GigAdapter->Segment));
  DEBUGPRINT (INIT, ("PCI Bus = %X\n", GigAdapter->Bus));
  DEBUGPRINT (INIT, ("PCI Device = %X\n", GigAdapter->Device));
  DEBUGPRINT (INIT, ("PCI Function = %X\n", GigAdapter->Function));
  DEBUGPRINT (INIT, ("PCI BaseAddressReg0 = %llX\n",
    PciConfigHeader->BaseAddressReg0));

  ZeroMem (GigAdapter->BroadcastNodeAddress, PXE_MAC_LENGTH);
  SetMem (GigAdapter->BroadcastNodeAddress, PXE_HWADDR_LEN_ETHER, 0xFF);

  // Initialize all parameters needed for the shared code
  if ((PciConfigHeader->BaseAddressReg0 & PCI_BAR_MEM_MASK) ==
    PCI_BAR_MEM_64BIT) {
    GigAdapter->Hw.hw_addr = (UINT8 *)
  ((UINT64)((UINT64)(PciConfigHeader->BaseAddressReg1) << 32) |
                             ((UINTN) PciConfigHeader->BaseAddressReg0));
    GigAdapter->Hw.hw_addr = (UINT8 *)
    ((UINT64) (GigAdapter->Hw.hw_addr) & ~((UINT64)0x4));
    DEBUGPRINT (INIT, ("PCI BaseAddressReg0 = %llX\n", GigAdapter->Hw.hw_addr));
  } else {
    GigAdapter->Hw.hw_addr              = (UINT8 *)
                                      (UINTN) PciConfigHeader->BaseAddressReg0;
  }
  GigAdapter->Hw.back                   = GigAdapter;
  GigAdapter->Hw.vendor_id              = PciConfigHeader->VendorId;
  GigAdapter->Hw.device_id              = PciConfigHeader->DeviceId;
  GigAdapter->Hw.revision_id            = (UINT8) PciConfigHeader->RevId;
  GigAdapter->Hw.subsystem_vendor_id    = PciConfigHeader->SubVendorId;
  GigAdapter->Hw.subsystem_device_id    = PciConfigHeader->SubSystemId;
  GigAdapter->Hw.revision_id            = (UINT8) PciConfigHeader->RevId;

  GigAdapter->txqnum     = INTELGBE_MAX_TX_QUEUES;
  GigAdapter->rxqnum     = INTELGBE_MAX_RX_QUEUES;
  GigAdapter->rx_queue[0].chan     = 1;
  GigAdapter->PciClass    = (UINT8)
  ((PciConfigHeader->ClassId & PCI_CLASS_MASK) >> 8);
  GigAdapter->PciSubClass = (UINT8)
  (PciConfigHeader->ClassId) & PCI_SUBCLASS_MASK;

  // We need to set the IO bar to zero for the shared code because the EFI PCI protocol
  // gets the BAR for us.
  GigAdapter->Hw.io_base               = 0;

  if (intelgbe_setup_init_funcs (&GigAdapter->Hw, TRUE) != INTELGBE_SUCCESS) {
    DEBUGPRINT (CRITICAL, ("intelgbe_setup_init_funcs failed!\n"));
    return EFI_UNSUPPORTED;
  }

  DEBUGPRINT (INTELGBE, ("Calling intelgbe_get_id\n"));
  GigAdapter->DeviceId = (UINT8)intelgbe_get_id (&GigAdapter->Hw);
  if (GigAdapter->DeviceId < 0x50){
    DEBUGPRINT (CRITICAL, ("Invalid DWC EQoS MAC IP Version 0x%02X\n",
      GigAdapter->DeviceId));
    return EFI_UNSUPPORTED;
  }
  DEBUGPRINT (INTELGBE, ("INTELGBE version = %x\n", GigAdapter->DeviceId));

  DEBUGPRINT (INTELGBE, ("Calling intelgbe_read_mac_addr\n"));
  if (intelgbe_read_mac_addr_generic (&GigAdapter->Hw) != INTELGBE_SUCCESS) {
    DEBUGPRINT (CRITICAL, ("Could not read MAC address\n"));
    return EFI_UNSUPPORTED;
  }

  if (phy->interface == PHY_INTERFACE_SGMII) {
      DEBUGPRINT (INIT, ("Identified SGMII\n"));
      DEBUGPRINT (INIT, ("initialize Modphy\n"));
      int retval = intelgbe_modphy_init(&GigAdapter->Hw);
      if (retval < 0) {
        return retval;
     }
     DEBUGPRINT (INIT, ("Modphy done\n"));
     DEBUGPRINT (INIT, ("Initialize xpcs\n"));
     retval = intelgbe_xpcs_init(&GigAdapter->Hw);
     if (retval < 0)
       return retval;
     DEBUGPRINT (INIT, ("xpcs done\n"));
  }

  /* PHY init*/
  DEBUGPRINT (CRITICAL, ("PHY initialization start\n"));
  if (intelgbe_phy_init (&GigAdapter->Hw) != INTELGBE_SUCCESS) {
    DEBUGPRINT (CRITICAL, ("PHY initialization failed\n"));
    return EFI_UNSUPPORTED;
  }

  /* software reset */
  if (intelgbe_reset (&GigAdapter->Hw) != INTELGBE_SUCCESS) {
    DEBUGPRINT (CRITICAL, ("Could not reset MAC controller\n"));
    return EFI_UNSUPPORTED;
  }

  DEBUGPRINT (INTELGBE, ("Calling intelgbe_write_mac_addr\n"));
  if (intelgbe_write_mac_addr_generic (&GigAdapter->Hw) != INTELGBE_SUCCESS) {
    DEBUGPRINT (INTELGBE, ("Could not write MAC address\n"));
    return EFI_UNSUPPORTED;
  }

  DEBUGPRINT (INTELGBE, ("Calling intelgbe_init_hw\n"));
  if (intelgbe_init_hw (&GigAdapter->Hw) != INTELGBE_SUCCESS) {
    DEBUGPRINT (CRITICAL, ("Hardware Init initialization failed\n"));
    GigAdapter->HwInitialized = FALSE;
    return EFI_UNSUPPORTED;
  } else {
    DEBUGPRINT (INTELGBE, ("intelgbe_init_hw success\n"));
    GigAdapter->HwInitialized = TRUE;
  }
  GigAdapter->tx_queue[0].cur_tx = 0;
  GigAdapter->rx_queue[0].cur_rx = 0;

  return EFI_SUCCESS;
}

/** Checks if link is up

   @param[in]   GigAdapter   Pointer to the NIC data structure information
                             which the UNDI driver is layering on.

   @retval   TRUE   Link is up
   @retval   FALSE  Link is down
**/
BOOLEAN
IsLinkUp (
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  struct intelgbe_hw *hw = &GigAdapter->Hw;
  struct intelgbe_mac_info *mac = &hw->mac;
  bool link = FALSE;
  if (mac->ops.check_for_link(hw, &link) != 0) {
    DEBUGPRINT (CRITICAL, ("PHY not initialized \n"));
  }
  return link;
}

/** This routine blocks until auto-negotiation completes or times out (after 4.5 seconds).

   @param[in]   GigAdapter   Pointer to the NIC data structure information
                             which the UNDI driver is layering on..

   @retval   TRUE   Auto-negotiation completed successfully,
   @retval   FALSE  Auto-negotiation did not complete (i.e., timed out)
**/
BOOLEAN
IntelgbeWaitForAutoNeg (
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  BOOLEAN AutoNegComplete;
  AutoNegComplete   = FALSE;

  DEBUGPRINT (INIT, ("IntelgbeWaitForAutoNeg\n"));

  if (!GigAdapter->CableDetect) {

    // Caller specified not to detect cable, so we return true.
    DEBUGPRINT (INIT, ("Cable detection disabled.\n"));
    return TRUE;
  }

  DEBUGPRINT (INIT, ("Return %d\n", AutoNegComplete));
  DEBUGWAIT (INIT);
  return AutoNegComplete;
}

/** This is the drivers copy function so it does not need to rely on the BootServices
   copy which goes away at runtime.

   This copy function allows 64-bit or 32-bit copies
   depending on platform architecture.  On Itanium we must check that both addresses
   are naturally aligned before attempting a 64-bit copy.

   @param[in]   Dest     Destination memory pointer to copy data to.
   @param[in]   Source   Source memory pointer.
   @param[in]   Count    Number of bytes to copy

   @return    Memory copied from source to destination
**/
VOID
IntelgbeMemCopy (
  IN UINT8* Dest,
  IN UINT8* Source,
  IN UINT32 Count
  )
{
  UINT32 BytesToCopy;
  UINT32 IntsToCopy;
  UINTN* SourcePtr;
  UINTN* DestPtr;
  UINT8* SourceBytePtr;
  UINT8* DestBytePtr;

  IntsToCopy = Count / sizeof (UINTN);
  BytesToCopy = Count % sizeof (UINTN);
#ifdef EFI64

  // Itanium cannot handle memory accesses that are not naturally aligned.  Determine
  // if 64-bit copy is even possible with these start addresses.
  if (((((UINTN) Source) & 0x0007) != 0)
    || (( ((UINTN) Dest) & 0x0007) != 0))
  {
    IntsToCopy = 0;
    BytesToCopy = Count;
  }
#endif /* EFI64 */

  SourcePtr = (UINTN *) Source;
  DestPtr = (UINTN *) Dest;

  while (IntsToCopy > 0) {
    *DestPtr = *SourcePtr;
    SourcePtr++;
    DestPtr++;
    IntsToCopy--;
  }

  // Copy the leftover bytes.
  SourceBytePtr = (UINT8 *) SourcePtr;
  DestBytePtr = (UINT8 *) DestPtr;
  while (BytesToCopy > 0) {
    *DestBytePtr = *SourceBytePtr;
    SourceBytePtr++;
    DestBytePtr++;
    BytesToCopy--;
  }
}

/** Copies the frame from our internal storage ring (As pointed to by GigAdapter->rx_ring)
   to the command Block passed in as part of the cpb parameter.

   The flow:
   Ack the interrupt, setup the pointers, find where the last Block copied is, check to make
   sure we have actually received something, and if we have then we do a lot of work.
   The packet is checked for errors, size is adjusted to remove the CRC, adjust the amount
   to copy if the buffer is smaller than the packet, copy the packet to the EFI buffer,
   and then figure out if the packet was targetted at us, broadcast, multicast
   or if we are all promiscuous.  We then put some of the more interesting information
   (protocol, src and dest from the packet) into the db that is passed to us.
   Finally we clean up the frame, set the return value to _SUCCESS, and inc the cur_rx_ind, watching
   for wrapping.  Then with all the loose ends nicely wrapped up, fade to black and return.

   @param[in]   GigAdapter   pointer to the driver data
   @param[in]   Cpb          Pointer (Ia-64 friendly) to the command parameter Block.
                             The frame will be placed inside of it.
   @param[out]  Db     The data buffer.  The out of band method of passing pre-digested
                       information to the protocol.

   @retval   PXE_STATCODE_NO_DATA If there is no data
   @retval   PXE_STATCODE_SUCCESS If we passed the goods to the protocol.
**/
UINTN
IntelgbeReceive (
  GIG_DRIVER_DATA *GigAdapter,
  UINT64           Cpb,
  UINT64           Db
  )
{
  PXE_CPB_RECEIVE *          CpbReceive;
  PXE_DB_RECEIVE *           DbReceive;
  PXE_FRAME_TYPE             PacketType;
  INTELGBE_RECEIVE_DESCRIPTOR  *desc;
  PXE_STATCODE              StatCode;
  ETHER_HEADER *            EtherHeader;
  struct intelgbe_rx_queue   *rx_q = &GigAdapter->rx_queue[0];
  UINT32 entry;
  int frame_len;


  PacketType  = PXE_FRAME_TYPE_NONE;
  StatCode    = PXE_STATCODE_NO_DATA;

  // Make quick copies of the buffer pointers so we can use them without fear of corrupting the originals
  CpbReceive  = (PXE_CPB_RECEIVE *) (UINTN) Cpb;
  DbReceive   = (PXE_DB_RECEIVE *) (UINTN) Db;

  // Get a pointer to the buffer that should have a rx in it, IF one is really there.
  //ReceiveDescriptor = INTELGBE_RX_DESC (&GigAdapter->RxRing, GigAdapter->CurRxInd); // AR check unmapped
  entry = rx_q->cur_rx;
  desc  = &rx_q->rx_desc[entry];
  if(!(desc->des3 & BIT(31))) {
    UINT32 rdes2 = desc->des2;
    UINT32 rdes3 = desc->des3;
    s32 ret = 0;

    if (!(rdes3 & BIT(28))) {
      DEBUGPRINT (CRITICAL, ("Not Last descriptor\n"));
      ret = -1;
    }
    if (rdes3 & (BIT(23) | BIT(24))) {
      DEBUGPRINT (CRITICAL, ("rdes3 status Error"));
      DEBUGPRINT (CRITICAL, (" desc->des3 %x, entry %d\n", desc->des3, entry));
      ret = -1;
    }
    if (rdes2 & (BIT(16) | BIT(17))) {
      DEBUGPRINT (CRITICAL, ("rdes2 status Error\n"));
      ret = -1;
    }
    rx_q->cur_rx++;
    if (rx_q->cur_rx >= DEFAULT_RX_DESCRIPTORS) {
      rx_q->cur_rx = 0;
    }

    if (!ret) {
      frame_len = rdes3 & 0x7FFF;
      if (frame_len > (INT16) CpbReceive->BufferLen) {
        frame_len = (UINT16) CpbReceive->BufferLen;
      }
      // Copy the packet from our list to the EFI buffer.
      IntelgbeMemCopy (
           (UINT8 *) (UINTN) CpbReceive->BufferAddr,
           (UINT8 *) (UINTN) &rx_q->rx_buff[entry],
           frame_len
      );
      DbReceive->FrameLen       = frame_len;  // includes header
      DbReceive->MediaHeaderLen = PXE_MAC_HEADER_LEN_ETHER;
      EtherHeader = (ETHER_HEADER *) (UINTN) &rx_q->rx_buff[entry];

      // Obtain packet type from MAC address
      if (CompareMem(EtherHeader->DestAddr, GigAdapter->Hw.mac.perm_addr, PXE_HWADDR_LEN_ETHER) == 0) {
        DEBUGPRINT(DECODE, ("Unicast packet\n"));
        PacketType = PXE_FRAME_TYPE_UNICAST;
      }
      else if (CompareMem(EtherHeader->DestAddr, GigAdapter->BroadcastNodeAddress, PXE_HWADDR_LEN_ETHER) == 0) {
        DEBUGPRINT(DECODE, ("Broadcast packet\n"));
        PacketType = PXE_FRAME_TYPE_BROADCAST;
      }
      else if (BIT_TEST(EtherHeader->DestAddr[0], 1)) {
        DEBUGPRINT(DECODE, ("Multicast packet\n"));
        PacketType = PXE_FRAME_TYPE_MULTICAST;
      }
      else {
        DEBUGPRINT(DECODE, ("Promiscuous packet\n"));
        PacketType = PXE_FRAME_TYPE_PROMISCUOUS;
      }

      DbReceive->Type = PacketType;

      // Put the protocol (UDP, TCP/IP) in the data buffer.
      DbReceive->Protocol = EtherHeader->Type;

      INTELGBE_COPY_MAC (DbReceive->SrcAddr, EtherHeader->SrcAddr);
      INTELGBE_COPY_MAC (DbReceive->DestAddr, EtherHeader->DestAddr);
    }
    desc->des0 = (u32)(u64)&rx_q->dma_rx_buff[entry];
    desc->des1 = 0;
    desc->des2 = 0;
    desc->des3 = (BIT(31) | BIT(30) | BIT(24));
    /* Initialize RX descriptor ring tail pointer */
    rx_q->rx_tail_addr = (u32)(u64)desc;
    INTELGBE_WRITE_REG(&GigAdapter->Hw, DMA_RXDESC_TAIL_PTR_CH(0),
      rx_q->rx_tail_addr);
    StatCode = PXE_STATCODE_SUCCESS;
  }
  return StatCode;
}

/** Stop the hardware and put it all (including the PHY) into a known good state.

   @param[in]   GigAdapter   Pointer to the driver structure

   @retval   PXE_STATCODE_SUCCESS    Hardware stopped
**/
UINTN
IntelgbeShutdown (
  GIG_DRIVER_DATA *GigAdapter
  )
{
  DEBUGPRINT (INTELGBE, ("IntelgbeShutdown - adapter stop\n"));

  // Disable the transmit and receive DMA
  intelgbe_uninit_hw(&GigAdapter->Hw);
  GigAdapter->ReceiveStarted = FALSE;
  GigAdapter->RxFilter = 0;

  return PXE_STATCODE_SUCCESS;
};
