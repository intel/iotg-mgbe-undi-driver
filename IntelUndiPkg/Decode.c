/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi/UefiPxe.h>
#include "Intelgbe.h"

/** This routine determines the operational state of the UNDI.  It updates the state flags in the
   Command Descriptor Block based on information derived from the GigAdapter instance data.

   To ensure the command has completed successfully, CdbPtr->StatCode will contain the result of
   the command execution. The CdbPtr->StatFlags will contain a STOPPED, STARTED, or INITIALIZED
   state once the command has successfully completed. Keep in mind the GigAdapter->State is the
   active state of the adapter (based on software interrogation), and the CdbPtr->StateFlags is
   the passed back information that is reflected to the caller of the UNDI API.

   @param[in]   CdbPtr       Pointer to the command descriptor block.
   @param[in]   GigAdapter  Pointer to the NIC data structure information which the
                             UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiGetState (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** This routine is used to change the operational state of the 1-Gigabit UNDI
   from stopped to started.

   It will do this as long as the adapter's state is PXE_STATFLAGS_GET_STATE_STOPPED, otherwise
   the CdbPtr->StatFlags will reflect a command failure, and the CdbPtr->StatCode will reflect the
   UNDI as having already been started.
   This routine is modified to reflect the UNDI 1.1 specification changes. The
   changes in the spec. are mainly in the callback routines, the new spec. adds
   3 more callbacks and a unique id. Since this UNDI supports both old and new UNDI specifications,
   The NIC's data structure is filled in with the callback routines (depending
   on the version) pointed to in the caller's CpbPtr.  This seeds the Delay,
   Virt2Phys, Block, and Mem_IO for old and new versions and Map_Mem, UnMap_Mem
   and Sync_Mem routines and a unique id variable for the new version.
   This is the function which an external entity (SNP, O/S, etc) would call
   to provide it's I/O abstraction to the UNDI.
   It's final action is to change the GigAdapter->State to PXE_STATFLAGS_GET_STATE_STARTED.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiStart (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** This routine is used to change the operational state of the UNDI from started to stopped.

   It will not do this if the adapter's state is PXE_STATFLAGS_GET_STATE_INITIALIZED, otherwise
   the CdbPtr->StatFlags will reflect a command failure, and the CdbPtr->StatCode will reflect the
   UNDI as having already not been shut down.
   The NIC's data structure will have the Delay, Virt2Phys, and Block, pointers zero'd out..
   It's final action is to change the GigAdapter->State to PXE_STATFLAGS_GET_STATE_STOPPED.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiStop (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** This routine is used to retrieve the initialization information that is
   needed by drivers and applications to initialize the UNDI.

   This will fill in data in the Data Block structure that is pointed to by the
   caller's CdbPtr->DBaddr.  The fields filled in are as follows:
   MemoryRequired, FrameDataLen, LinkSpeeds[0-3], NvCount, NvWidth, MediaHeaderLen, HWaddrLen,
   MCastFilterCnt, TxBufCnt, TxBufSize, RxBufCnt, RxBufSize, IFtype, Duplex, and LoopBack.
   In addition, the CdbPtr->StatFlags ORs in that this NIC supports cable detection.  (APRIORI knowledge)

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiGetInitInfo (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** This routine is used to retrieve the configuration information about the NIC being controlled by
  this driver.

  This will fill in data in the Data Block structure that is pointed to by the caller's CdbPtr->DBaddr.
  The fields filled in are as follows:
  DbPtr->pci.BusType, DbPtr->pci.Bus, DbPtr->pci.Device, and DbPtr->pci.
  In addition, the DbPtr->pci.Config.Dword[0-63] grabs a copy of this NIC's PCI configuration space.

  @param[in]   CdbPtr        Pointer to the command descriptor block.
  @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                             UNDI driver is layering on..

  @retval      None
**/
VOID
IntelgbeUndiGetConfigInfo (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** This routine resets the network adapter and initializes the 1-Gigabit UNDI using the parameters
   supplied in the CPB.

   This command must be issued before the network adapter can be setup to transmit and receive packets.
   Once the memory requirements of the UNDI are obtained by using the GetInitInfo command, a block
   of non-swappable memory may need to be allocated.  The address of this memory must be passed to
   UNDI during the Initialize in the CPB.  This memory is used primarily for transmit and receive buffers.
   The fields CableDetect, LinkSpeed, Duplex, LoopBack, MemoryPtr, and MemoryLength are set with
   information that was passed in the CPB and the NIC is initialized.
   If the NIC initialization fails, the CdbPtr->StatFlags are updated with PXE_STATFLAGS_COMMAND_FAILED
   Otherwise, GigAdapter->State is updated with PXE_STATFLAGS_GET_STATE_INITIALIZED showing the state of
   the UNDI is now initialized.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                             UNDI driver is layering on..

   @retval      None
-**/
VOID
IntelgbeUndiInitialize (
  IN  PXE_CDB *    CdbPtr,
  GIG_DRIVER_DATA *GigAdapter
  );

/** This routine resets the network adapter and initializes the 1-Gigabit UNDI using the
   parameters supplied in the CPB.

   The transmit and receive queues are emptied and any pending interrupts are cleared.
   If the NIC reset fails, the CdbPtr->StatFlags are updated with PXE_STATFLAGS_COMMAND_FAILED

   @param[in]   CdbPtr         Pointer to the command descriptor block.
   @param[in]   GigAdapter    Pointer to the NIC data structure information which the
                               UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiReset (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** This routine resets the network adapter and leaves it in a safe state for another
   driver to initialize.

   Any pending transmits or receives are lost.  Receive filters and external
   interrupt enables are disabled.  Once the UNDI has been shutdown, it can then be stopped
   or initialized again.
   If the NIC reset fails, the CdbPtr->StatFlags are updated with PXE_STATFLAGS_COMMAND_FAILED
   Otherwise, GigAdapter->State is updated with PXE_STATFLAGS_GET_STATE_STARTED showing
   the state of the NIC as being started.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                             UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiShutdown (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** This routine can be used to read and/or change the current external interrupt enable
   settings.

   Disabling an external interrupt enable prevents and external (hardware)
   interrupt from being signalled by the network device.  Internally the interrupt events
   can still be polled by using the UNDI_GetState command.
   The resulting information on the interrupt state will be passed back in the CdbPtr->StatFlags.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on.

   @retval      None
**/
VOID
IntelgbeUndiInterrupt (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** This routine is used to read and change receive filters and, if supported, read
   and change multicast MAC address filter list.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                             UNDI driver is layering on..

   @retval     None
**/
VOID
IntelgbeUndiRecFilter (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** This routine is used to get the current station and broadcast MAC addresses,
   and to change the current station MAC address.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on.

   @retval      None
**/
VOID
IntelgbeUndiStnAddr (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** This routine is used to read and clear the NIC traffic statistics.  This command is supported
   only if the !PXE structure's Implementation flags say so.

   Results will be parsed out in the following manner:
   CdbPtr->DBaddr.Data[0]   R  Total Frames (Including frames with errors and dropped frames)
   CdbPtr->DBaddr.Data[1]   R  Good Frames (All frames copied into receive buffer)
   CdbPtr->DBaddr.Data[2]   R  Undersize Frames (Frames below minimum length for media <64 for ethernet)
   CdbPtr->DBaddr.Data[4]   R  Dropped Frames (Frames that were dropped because receive buffers were full)
   CdbPtr->DBaddr.Data[8]   R  CRC Error Frames (Frames with alignment or CRC errors)
   CdbPtr->DBaddr.Data[A]   T  Total Frames (Including frames with errors and dropped frames)
   CdbPtr->DBaddr.Data[B]   T  Good Frames (All frames copied into transmit buffer)
   CdbPtr->DBaddr.Data[C]   T  Undersize Frames (Frames below minimum length for media <64 for ethernet)
   CdbPtr->DBaddr.Data[E]   T  Dropped Frames (Frames that were dropped because of collisions)
   CdbPtr->DBaddr.Data[14]  T  Total Collision Frames (Total collisions on this subnet)

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiStatistics (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** This routine is used to translate a multicast IP address to a multicast MAC address.

   This results in a MAC address composed of 25 bits of fixed data with the upper 23 bits of the IP
   address being appended to it.  Results passed back in the equivalent of CdbPtr->DBaddr->MAC[0-5].

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiIp2Mac (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** This routine is used to read and write non-volatile storage on the NIC (if supported).  The NVRAM
   could be EEPROM, FLASH, or battery backed RAM.

   This is an optional function according to the UNDI specification  (or will be......)

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiNvData (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** This routine returns the current interrupt status and/or the transmitted buffer addresses.

   If the current interrupt status is returned, pending interrupts will be acknowledged by this
   command.  Transmitted buffer addresses that are written to the DB are removed from the transmit
   buffer queue.
   Normally, this command would be polled with interrupts disabled.
   The transmit buffers are returned in CdbPtr->DBaddr->TxBufer[0 - NumEntries].
   The interrupt status is returned in CdbPtr->StatFlags.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the 1-Gigabit
                             UNDI driver is layering on..

   @retval   None
**/
VOID
IntelgbeUndiStatus (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** This routine is used to fill media header(s) in transmit packet(s).

   Copies the MAC address into the media header whether it is dealing
   with fragmented or non-fragmented packets.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                             UNDI driver is layering on.

   @retval      None
**/
VOID
IntelgbeUndiFillHeader (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** This routine is used to place a packet into the transmit queue.

   The data buffers given to this command are to be considered locked and the application or
   network driver loses ownership of these buffers and must not free or relocate them until
   the ownership returns.
   When the packets are transmitted, a transmit complete interrupt is generated (if interrupts
   are disabled, the transmit interrupt status is still set and can be checked using the UNDI_Status
   command.
   Some implementations and adapters support transmitting multiple packets with one transmit
   command.  If this feature is supported, the transmit CPBs can be linked in one transmit
   command.
   All UNDIs support fragmented frames, now all network devices or protocols do.  If a fragmented
   frame CPB is given to UNDI and the network device does not support fragmented frames
   (see !PXE.Implementation flag), the UNDI will have to copy the fragments into a local buffer
   before transmitting.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiTransmit (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** When the network adapter has received a frame, this command is used to copy the frame
   into the driver/application storage location.

   Once a frame has been copied, it is removed from the receive queue.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiReceive (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  );

// Global variables defined in this file
UNDI_CALL_TABLE mIntelgbeApiTable[PXE_OPCODE_LAST_VALID + 1] = {
  {
    PXE_CPBSIZE_NOT_USED,
    PXE_DBSIZE_NOT_USED,
    0,
    (UINT16) (ANY_STATE),
    IntelgbeUndiGetState
  },
  {
    (UINT16) (DONT_CHECK),
    PXE_DBSIZE_NOT_USED,
    0,
    (UINT16) (ANY_STATE),
    IntelgbeUndiStart
  },
  {
    PXE_CPBSIZE_NOT_USED,
    PXE_DBSIZE_NOT_USED,
    0,
    MUST_BE_STARTED,
    IntelgbeUndiStop
  },
  {
    PXE_CPBSIZE_NOT_USED,
    sizeof (PXE_DB_GET_INIT_INFO),
    0,
    MUST_BE_STARTED,
    IntelgbeUndiGetInitInfo
  },
  {
    PXE_CPBSIZE_NOT_USED,
    sizeof (PXE_DB_GET_CONFIG_INFO),
    0,
    MUST_BE_STARTED,
    IntelgbeUndiGetConfigInfo
  },
  {
    sizeof (PXE_CPB_INITIALIZE),
    (UINT16) (DONT_CHECK),
    (UINT16) (DONT_CHECK),
    MUST_BE_STARTED,
    IntelgbeUndiInitialize
  },
  {
    PXE_CPBSIZE_NOT_USED,
    PXE_DBSIZE_NOT_USED,
    (UINT16) (DONT_CHECK),
    MUST_BE_INITIALIZED,
    IntelgbeUndiReset
  },
  {
    PXE_CPBSIZE_NOT_USED,
    PXE_DBSIZE_NOT_USED,
    0,
    MUST_BE_INITIALIZED,
    IntelgbeUndiShutdown
  },
  {
    PXE_CPBSIZE_NOT_USED,
    PXE_DBSIZE_NOT_USED,
    (UINT16) (DONT_CHECK),
    MUST_BE_INITIALIZED,
    IntelgbeUndiInterrupt
  },
  {
    (UINT16) (DONT_CHECK),
    (UINT16) (DONT_CHECK),
    (UINT16) (DONT_CHECK),
    MUST_BE_INITIALIZED,
    IntelgbeUndiRecFilter
  },
  {
    (UINT16) (DONT_CHECK),
    (UINT16) (DONT_CHECK),
    (UINT16) (DONT_CHECK),
    MUST_BE_INITIALIZED,
    IntelgbeUndiStnAddr
  },
  {
    PXE_CPBSIZE_NOT_USED,
    (UINT16) (DONT_CHECK),
    (UINT16) (DONT_CHECK),
    MUST_BE_INITIALIZED,
    IntelgbeUndiStatistics
  },
  {
    sizeof (PXE_CPB_MCAST_IP_TO_MAC),
    sizeof (PXE_DB_MCAST_IP_TO_MAC),
    (UINT16) (DONT_CHECK),
    MUST_BE_INITIALIZED,
    IntelgbeUndiIp2Mac
  },
  {
    (UINT16) (DONT_CHECK),
    (UINT16) (DONT_CHECK),
    (UINT16) (DONT_CHECK),
    MUST_BE_INITIALIZED,
    IntelgbeUndiNvData
  },
  {
    PXE_CPBSIZE_NOT_USED,
    (UINT16) (DONT_CHECK),
    (UINT16) (DONT_CHECK),
    MUST_BE_INITIALIZED,
    IntelgbeUndiStatus
  },
  {
    (UINT16) (DONT_CHECK),
    PXE_DBSIZE_NOT_USED,
    (UINT16) (DONT_CHECK),
    MUST_BE_INITIALIZED,
    IntelgbeUndiFillHeader
  },
  {
    (UINT16) (DONT_CHECK),
    PXE_DBSIZE_NOT_USED,
    (UINT16) (DONT_CHECK),
    MUST_BE_INITIALIZED,
    IntelgbeUndiTransmit
  },
  {
    sizeof (PXE_CPB_RECEIVE),
    sizeof (PXE_DB_RECEIVE),
    0,
    MUST_BE_INITIALIZED,
    IntelgbeUndiReceive
  }
};

/** This routine determines the operational state of the UNDI.  It updates the state flags in the
   Command Descriptor Block based on information derived from the GigAdapter instance data.

   To ensure the command has completed successfully, CdbPtr->StatCode will contain the result of
   the command execution. The CdbPtr->StatFlags will contain a STOPPED, STARTED, or INITIALIZED
   state once the command has successfully completed. Keep in mind the GigAdapter->State is the
   active state of the adapter (based on software interrogation), and the CdbPtr->StateFlags is
   the passed back information that is reflected to the caller of the UNDI API.


   @param[in]   CdbPtr       Pointer to the command descriptor block.
   @param[in]   GigAdapter  Pointer to the NIC data structure information which the
                             UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiGetState (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  DEBUGPRINT (CRITICAL, ("IntelgbeUndiGetState\n"));
  DEBUGWAIT (DECODE);

  CdbPtr->StatFlags |= GigAdapter->State;
  CdbPtr->StatFlags |= PXE_STATFLAGS_COMMAND_COMPLETE;

  CdbPtr->StatCode = PXE_STATCODE_SUCCESS;
}

/** This routine is used to change the operational state of the 1-Gigabit UNDI
   from stopped to started.

   It will do this as long as the adapter's state is PXE_STATFLAGS_GET_STATE_STOPPED, otherwise
   the CdbPtr->StatFlags will reflect a command failure, and the CdbPtr->StatCode will reflect the
   UNDI as having already been started.
   This routine is modified to reflect the UNDI 1.1 specification changes. The
   changes in the spec. are mainly in the callback routines, the new spec. adds
   3 more callbacks and a unique id. Since this UNDI supports both old and new UNDI specifications,
   The NIC's data structure is filled in with the callback routines (depending
   on the version) pointed to in the caller's CpbPtr.  This seeds the Delay,
   Virt2Phys, Block, and Mem_IO for old and new versions and Map_Mem, UnMap_Mem
   and Sync_Mem routines and a unique id variable for the new version.
   This is the function which an external entity (SNP, O/S, etc) would call
   to provide it's I/O abstraction to the UNDI.
   It's final action is to change the GigAdapter->State to PXE_STATFLAGS_GET_STATE_STARTED.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiStart (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  PXE_CPB_START_31 *CpbPtr31;

  DEBUGPRINT (DECODE, ("IntelgbeUndiStart\n"));
  DEBUGWAIT (DECODE);

  // check if it is already started.
  if (GigAdapter->State != PXE_STATFLAGS_GET_STATE_STOPPED) {
    CdbPtr->StatFlags     = PXE_STATFLAGS_COMMAND_COMPLETE;
    CdbPtr->StatCode      = PXE_STATCODE_SUCCESS;
    return;
  }

  if (CdbPtr->CPBsize != sizeof (PXE_CPB_START_30)
    && CdbPtr->CPBsize != sizeof (PXE_CPB_START_31))
  {
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
    CdbPtr->StatCode = PXE_STATCODE_INVALID_CDB;
    return;
  }

  CpbPtr31 = (PXE_CPB_START_31 *) (UINTN) (CdbPtr->CPBaddr);

  GigAdapter->Delay     = (BS_PTR) (UINTN) CpbPtr31->Delay;
  GigAdapter->Virt2Phys = (VIRT_PHYS) (UINTN) CpbPtr31->Virt2Phys;
  GigAdapter->Block     = (BLOCK) (UINTN) CpbPtr31->Block;
  GigAdapter->MemIo     = (MEM_IO) (UINTN) CpbPtr31->Mem_IO;
  GigAdapter->MapMem    = (MAP_MEM) (UINTN) CpbPtr31->Map_Mem;
  GigAdapter->UnMapMem  = (UNMAP_MEM) (UINTN) CpbPtr31->UnMap_Mem;
  GigAdapter->SyncMem   = (SYNC_MEM) (UINTN) CpbPtr31->Sync_Mem;
  GigAdapter->UniqueId = CpbPtr31->Unique_ID;
  DEBUGPRINT (DECODE, ("CpbPtr31->Unique_ID = %x\n", CpbPtr31->Unique_ID));
  DEBUGPRINT (DECODE, ("CpbPtr31->Delay = %x\n", CpbPtr31->Delay));
  DEBUGPRINT (DECODE, ("CpbPtr31->Virt2Phys = %x\n", CpbPtr31->Virt2Phys));
  DEBUGPRINT (DECODE, ("CpbPtr31->Block = %x\n", CpbPtr31->Block));
  DEBUGPRINT (DECODE, ("CpbPtr31->MemIo = %x\n", CpbPtr31->Mem_IO));
  DEBUGPRINT (DECODE, ("CpbPtr31->MapMem = %x\n", CpbPtr31->Map_Mem));
  DEBUGPRINT (DECODE, ("CpbPtr31->UnMapMem = %x\n", CpbPtr31->UnMap_Mem));
  DEBUGPRINT (DECODE, ("CpbPtr31->SyncMem = %x\n", CpbPtr31->Sync_Mem));

  GigAdapter->State = PXE_STATFLAGS_GET_STATE_STARTED;

  CdbPtr->StatFlags     = PXE_STATFLAGS_COMMAND_COMPLETE;
  CdbPtr->StatCode      = PXE_STATCODE_SUCCESS;
}

/** This routine is used to change the operational state of the UNDI from started to stopped.

   It will not do this if the adapter's state is PXE_STATFLAGS_GET_STATE_INITIALIZED, otherwise
   the CdbPtr->StatFlags will reflect a command failure, and the CdbPtr->StatCode will reflect the
   UNDI as having already not been shut down.
   The NIC's data structure will have the Delay, Virt2Phys, and Block, pointers zero'd out..
   It's final action is to change the GigAdapter->State to PXE_STATFLAGS_GET_STATE_STOPPED.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiStop (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  DEBUGPRINT (DECODE, ("IntelgbeUndiStop\n"));
  return;
}

/** This routine is used to retrieve the initialization information that is
   needed by drivers and applications to initialize the UNDI.

   This will fill in data in the Data Block structure that is pointed to by the
   caller's CdbPtr->DBaddr.  The fields filled in are as follows:
   MemoryRequired, FrameDataLen, LinkSpeeds[0-3], NvCount, NvWidth, MediaHeaderLen, HWaddrLen,
   MCastFilterCnt, TxBufCnt, TxBufSize, RxBufCnt, RxBufSize, IFtype, Duplex, and LoopBack.
   In addition, the CdbPtr->StatFlags ORs in that this NIC supports cable detection.  (APRIORI knowledge)

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiGetInitInfo (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  PXE_DB_GET_INIT_INFO *DbPtr;

  DEBUGPRINT (CRITICAL, ("IntelgbeUndiGetInitInfo\n"));
  DEBUGWAIT (DECODE);

  DbPtr                 = (PXE_DB_GET_INIT_INFO *) (UINTN) (CdbPtr->DBaddr);

  DbPtr->MemoryRequired = 0;
  DbPtr->FrameDataLen   = PXE_MAX_TXRX_UNIT_ETHER;

  DbPtr->LinkSpeeds[0]  = 10;
  DbPtr->LinkSpeeds[1]  = 100;
  DbPtr->LinkSpeeds[2]  = 0;
  DbPtr->LinkSpeeds[3]  = 0;

  DbPtr->NvCount        = MAX_EEPROM_LEN;
  DbPtr->NvWidth        = 4;
  DbPtr->MediaHeaderLen = PXE_MAC_HEADER_LEN_ETHER;
  DbPtr->HWaddrLen      = PXE_HWADDR_LEN_ETHER;
  DbPtr->MCastFilterCnt = MAX_MCAST_ADDRESS_CNT;

  DbPtr->TxBufCnt       = DEFAULT_TX_DESCRIPTORS;
  DbPtr->TxBufSize      = sizeof (INTELGBE_TRANSMIT_DESCRIPTOR);
  DbPtr->RxBufCnt       = DEFAULT_RX_DESCRIPTORS;
  DbPtr->RxBufSize      = sizeof (INTELGBE_RECEIVE_DESCRIPTOR) +
  sizeof (LOCAL_RX_BUFFER);

  DbPtr->IFtype         = PXE_IFTYPE_ETHERNET;
  DbPtr->SupportedDuplexModes         = PXE_DUPLEX_ENABLE_FULL_SUPPORTED |
  PXE_DUPLEX_FORCE_FULL_SUPPORTED;
  DbPtr->SupportedLoopBackModes       = 0;

  CdbPtr->StatFlags |= (PXE_STATFLAGS_CABLE_DETECT_SUPPORTED |
                        PXE_STATFLAGS_GET_STATUS_NO_MEDIA_SUPPORTED);

  CdbPtr->StatFlags |= PXE_STATFLAGS_COMMAND_COMPLETE;
  CdbPtr->StatCode = PXE_STATCODE_SUCCESS;

  if (!GigAdapter->UndiEnabled) {
    CdbPtr->StatCode = PXE_STATCODE_BUSY;
  }
  return;
}

/** This routine is used to retrieve the configuration information about the NIC being controlled by
  this driver.

  This will fill in data in the Data Block structure that is pointed to by the caller's CdbPtr->DBaddr.
  The fields filled in are as follows:
  DbPtr->pci.BusType, DbPtr->pci.Bus, DbPtr->pci.Device, and DbPtr->pci.
  In addition, the DbPtr->pci.Config.Dword[0-63] grabs a copy of this NIC's PCI configuration space.

  @param[in]   CdbPtr        Pointer to the command descriptor block.
  @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                             UNDI driver is layering on..

  @retval      None
**/
VOID
IntelgbeUndiGetConfigInfo (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  DEBUGPRINT (DECODE, ("IntelgbeUndiGetConfigInfo\n"));
  return;
}

/** This routine resets the network adapter and initializes the 1-Gigabit UNDI using the parameters
   supplied in the CPB.

   This command must be issued before the network adapter can be setup to transmit and receive packets.
   Once the memory requirements of the UNDI are obtained by using the GetInitInfo command, a block
   of non-swappable memory may need to be allocated.  The address of this memory must be passed to
   UNDI during the Initialize in the CPB.  This memory is used primarily for transmit and receive buffers.
   The fields CableDetect, LinkSpeed, Duplex, LoopBack, MemoryPtr, and MemoryLength are set with
   information that was passed in the CPB and the NIC is initialized.
   If the NIC initialization fails, the CdbPtr->StatFlags are updated with PXE_STATFLAGS_COMMAND_FAILED
   Otherwise, GigAdapter->State is updated with PXE_STATFLAGS_GET_STATE_INITIALIZED showing the state of
   the UNDI is now initialized.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                             UNDI driver is layering on..

   @retval      None
-**/
VOID
IntelgbeUndiInitialize (
  IN  PXE_CDB *    CdbPtr,
  GIG_DRIVER_DATA *GigAdapter
  )
{
  PXE_CPB_INITIALIZE *CpbPtr;
  PXE_DB_INITIALIZE * DbPtr;

  DEBUGPRINT (DECODE, ("IntelgbeUndiInitialize\n"));
  DEBUGWAIT (DECODE);

  if (GigAdapter->DriverBusy) {
    DEBUGPRINT (DECODE,
      ("ERROR: IntelgbeUndiInitialize called when driver busy\n"));
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
    CdbPtr->StatCode = PXE_STATCODE_BUSY;
    return;
  }

  if ((CdbPtr->OpFlags != PXE_OPFLAGS_INITIALIZE_DETECT_CABLE) &&
    (CdbPtr->OpFlags != PXE_OPFLAGS_INITIALIZE_DO_NOT_DETECT_CABLE))
  {
    DEBUGPRINT (CRITICAL, ("INVALID CDB\n"));
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
    CdbPtr->StatCode = PXE_STATCODE_INVALID_CDB;
    return;
  }

  // Check if it is already initialized
  if (GigAdapter->State == PXE_STATFLAGS_GET_STATE_INITIALIZED) {
    CdbPtr->StatFlags     = PXE_STATFLAGS_COMMAND_COMPLETE;
    CdbPtr->StatCode      = PXE_STATCODE_SUCCESS;
    DEBUGPRINT (DECODE, ("ALREADY INITIALIZED\n"));
    return;
  }

  CpbPtr  = (PXE_CPB_INITIALIZE *) (UINTN) CdbPtr->CPBaddr;
  DbPtr   = (PXE_DB_INITIALIZE *) (UINTN) CdbPtr->DBaddr;

  // Default behaviour is to detect the cable, if the 3rd param is 1,
  // do not do that
  if (CdbPtr->OpFlags == (UINT16) PXE_OPFLAGS_INITIALIZE_DO_NOT_DETECT_CABLE) {
    GigAdapter->CableDetect = (UINT8) 0;
  } else {
    GigAdapter->CableDetect = (UINT8) 1;
  }
  DEBUGPRINT (DECODE, ("CdbPtr->OpFlags = %X\n", CdbPtr->OpFlags));
  GigAdapter->LinkSpeed     = (UINT16) CpbPtr->LinkSpeed;
  GigAdapter->DuplexMode    = CpbPtr->DuplexMode;
  GigAdapter->LoopBack      = CpbPtr->LoopBackMode;

  DEBUGPRINT (DECODE, ("CpbPtr->TxBufCnt = %X\n", CpbPtr->TxBufCnt));
  DEBUGPRINT (DECODE, ("CpbPtr->TxBufSize = %X\n", CpbPtr->TxBufSize));
  DEBUGPRINT (DECODE, ("CpbPtr->RxBufCnt = %X\n", CpbPtr->RxBufCnt));
  DEBUGPRINT (DECODE, ("CpbPtr->RxBufSize = %X\n", CpbPtr->RxBufSize));

  CdbPtr->StatCode = (PXE_STATCODE) IntelgbeInititialize (GigAdapter);

  // We allocate our own memory for transmit and receive so set MemoryUsed to 0.
  DbPtr->MemoryUsed = 0;
  DbPtr->TxBufCnt   = DEFAULT_TX_DESCRIPTORS;
  DbPtr->TxBufSize  = sizeof (INTELGBE_TRANSMIT_DESCRIPTOR);
  DbPtr->RxBufCnt   = DEFAULT_RX_DESCRIPTORS;
  DbPtr->RxBufSize  = sizeof (INTELGBE_RECEIVE_DESCRIPTOR) +
  sizeof (LOCAL_RX_BUFFER);

  if (CdbPtr->StatCode != PXE_STATCODE_SUCCESS) {
    DEBUGPRINT (CRITICAL,
      ("IntelgbeInititialize failed! Statcode = %X\n", CdbPtr->StatCode));
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
  } else {
    GigAdapter->State = PXE_STATFLAGS_GET_STATE_INITIALIZED;
  }

  // If no link is detected we want to set the driver state back to _GET_STATE_STARTED so
  // that the SNP will not try to restart the driver.
  if (IntelgbeWaitForAutoNeg (GigAdapter)) {
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_COMPLETE;
  } else {
    CdbPtr->StatFlags |= PXE_STATFLAGS_INITIALIZED_NO_MEDIA;
    CdbPtr->StatCode = PXE_STATCODE_NOT_STARTED;
    GigAdapter->State = PXE_STATFLAGS_GET_STATE_STARTED;
  }

  return;
}

/** This routine resets the network adapter and initializes the 1-Gigabit UNDI using the
   parameters supplied in the CPB.

   The transmit and receive queues are emptied and any pending interrupts are cleared.
   If the NIC reset fails, the CdbPtr->StatFlags are updated with PXE_STATFLAGS_COMMAND_FAILED

   @param[in]   CdbPtr         Pointer to the command descriptor block.
   @param[in]   GigAdapter    Pointer to the NIC data structure information which the
                               UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiReset (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  DEBUGPRINT (DECODE, ("IntelgbeUndiReset\n"));
  return;
}

/** This routine resets the network adapter and leaves it in a safe state for another
   driver to initialize.

   Any pending transmits or receives are lost.  Receive filters and external
   interrupt enables are disabled.  Once the UNDI has been shutdown, it can then be stopped
   or initialized again.
   If the NIC reset fails, the CdbPtr->StatFlags are updated with PXE_STATFLAGS_COMMAND_FAILED
   Otherwise, GigAdapter->State is updated with PXE_STATFLAGS_GET_STATE_STARTED showing
   the state of the NIC as being started.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                             UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiShutdown (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  // do the shutdown stuff here
  DEBUGPRINT (DECODE, ("IntelgbeUndiShutdown\n"));
  return;
}

/** This routine can be used to read and/or change the current external interrupt enable
   settings.

   Disabling an external interrupt enable prevents and external (hardware)
   interrupt from being signalled by the network device.  Internally the interrupt events
   can still be polled by using the UNDI_GetState command.
   The resulting information on the interrupt state will be passed back in the CdbPtr->StatFlags.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on.

   @retval      None
**/
VOID
IntelgbeUndiInterrupt (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  DEBUGPRINT (CRITICAL, ("IntelgbeUndiInterrupt\n"));
  return;
}

/** This routine is used to read and change receive filters and, if supported, read
   and change multicast MAC address filter list.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                             UNDI driver is layering on..

   @retval     None
**/
VOID
IntelgbeUndiRecFilter (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  DEBUGPRINT (DECODE, ("INTELGBEUndiRecFilter\n"));
  UINT32 OldFilter;

  if (GigAdapter->DriverBusy) {
    DEBUGPRINT (DECODE,
      ("INTELGBE: IntelgbeUndiRecFilter called when driver busy\n"));
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
    CdbPtr->StatCode = PXE_STATCODE_BUSY;
    return;
  }

  OldFilter = INTELGBE_READ_REG(&GigAdapter->Hw, MAC_PACKET_FILTER);
  if (!(OldFilter & INTELGBE_RCFIL_ALLMCAST)) {
    INTELGBE_WRITE_REG(&GigAdapter->Hw, MAC_PACKET_FILTER, (OldFilter | INTELGBE_RCFIL_ALLMCAST));
    DEBUGPRINT (CRITICAL, ("INTELGBEUndiRecFilter set packet filter to pass all multicast\n"));
  }

  CdbPtr->StatFlags |= PXE_STATFLAGS_RECEIVE_FILTER_UNICAST |
                       PXE_STATFLAGS_RECEIVE_FILTER_BROADCAST |
                       PXE_STATFLAGS_RECEIVE_FILTER_ALL_MULTICAST |
                       PXE_STATFLAGS_COMMAND_COMPLETE;
  CdbPtr->StatCode  = PXE_STATCODE_SUCCESS;
  return;
}

/** This routine is used to get the current station and broadcast MAC addresses,
   and to change the current station MAC address.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on.

   @retval      None
**/
VOID
IntelgbeUndiStnAddr (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  PXE_CPB_STATION_ADDRESS *CpbPtr;
  PXE_DB_STATION_ADDRESS * DbPtr;
  UINT16                   i;

  DbPtr = NULL;
  DEBUGPRINT (DECODE, ("IntelgbeUndiStnAddr\n"));
  if (CdbPtr->OpFlags == PXE_OPFLAGS_STATION_ADDRESS_RESET) {
    // configure the permanent address.
    // change the GigAdapter->CurrentNodeAddress field.
    if (CompareMem (
        GigAdapter->Hw.mac.addr,
        GigAdapter->Hw.mac.perm_addr,
        PXE_HWADDR_LEN_ETHER
        ) != 0) {
      CopyMem (
        GigAdapter->Hw.mac.addr,
        GigAdapter->Hw.mac.perm_addr,
        PXE_HWADDR_LEN_ETHER
      );
    }
  }

  if (CdbPtr->CPBaddr != (UINT64) 0) {
    CpbPtr = (PXE_CPB_STATION_ADDRESS *) (UINTN) (CdbPtr->CPBaddr);
    GigAdapter->MacAddrOverride = TRUE;

    // configure the new address
    CopyMem (
      GigAdapter->Hw.mac.addr,
      CpbPtr->StationAddr,
      PXE_HWADDR_LEN_ETHER
    );
    for (i = 0; i < 6; i++) {
      DEBUGPRINT (DECODE, ("%2x ", CpbPtr->StationAddr[i]));
    }

  }

  if (CdbPtr->DBaddr != (UINT64) 0) {
    DbPtr = (PXE_DB_STATION_ADDRESS *) (UINTN) (CdbPtr->DBaddr);

    // fill it with the new values
    ZeroMem (DbPtr->StationAddr, PXE_MAC_LENGTH);
    ZeroMem (DbPtr->PermanentAddr, PXE_MAC_LENGTH);
    ZeroMem (DbPtr->BroadcastAddr, PXE_MAC_LENGTH);
    CopyMem (DbPtr->StationAddr, GigAdapter->Hw.mac.addr, PXE_HWADDR_LEN_ETHER);
    CopyMem (DbPtr->PermanentAddr,
      GigAdapter->Hw.mac.perm_addr, PXE_HWADDR_LEN_ETHER);
    CopyMem (DbPtr->BroadcastAddr,
      GigAdapter->BroadcastNodeAddress, PXE_MAC_LENGTH);
  }
  DEBUGPRINT (DECODE, ("\nDbPtr->StationAddr ="));
  for (i = 0; i < PXE_MAC_LENGTH; i++) {
    DEBUGPRINT (DECODE, (" %x", DbPtr->StationAddr[i]));
  }
  DEBUGPRINT (DECODE, ("\nDbPtr->PermanentAddr ="));
  for (i = 0; i < PXE_MAC_LENGTH; i++) {
    DEBUGPRINT (DECODE, (" %x", DbPtr->PermanentAddr[i]));
  }

  DEBUGPRINT (DECODE, ("\nDbPtr->BroadcastAddr ="));
  for (i = 0; i < PXE_MAC_LENGTH; i++) {
    DEBUGPRINT (DECODE, (" %x", DbPtr->BroadcastAddr[i]));
  }

  DEBUGPRINT (DECODE, ("\n"));
  DEBUGWAIT (DECODE);
  CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_COMPLETE;
  CdbPtr->StatCode  = PXE_STATCODE_SUCCESS;
  return;
}

/** This routine is used to read and clear the NIC traffic statistics.  This command is supported
   only if the !PXE structure's Implementation flags say so.

   Results will be parsed out in the following manner:
   CdbPtr->DBaddr.Data[0]   R  Total Frames (Including frames with errors and dropped frames)
   CdbPtr->DBaddr.Data[1]   R  Good Frames (All frames copied into receive buffer)
   CdbPtr->DBaddr.Data[2]   R  Undersize Frames (Frames below minimum length for media <64 for ethernet)
   CdbPtr->DBaddr.Data[4]   R  Dropped Frames (Frames that were dropped because receive buffers were full)
   CdbPtr->DBaddr.Data[8]   R  CRC Error Frames (Frames with alignment or CRC errors)
   CdbPtr->DBaddr.Data[A]   T  Total Frames (Including frames with errors and dropped frames)
   CdbPtr->DBaddr.Data[B]   T  Good Frames (All frames copied into transmit buffer)
   CdbPtr->DBaddr.Data[C]   T  Undersize Frames (Frames below minimum length for media <64 for ethernet)
   CdbPtr->DBaddr.Data[E]   T  Dropped Frames (Frames that were dropped because of collisions)
   CdbPtr->DBaddr.Data[14]  T  Total Collision Frames (Total collisions on this subnet)

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiStatistics (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  DEBUGPRINT (DECODE, ("IntelgbeUndiStatistics\n"));
  return;
}

/** This routine is used to translate a multicast IP address to a multicast MAC address.

   This results in a MAC address composed of 25 bits of fixed data with the upper 23 bits of the IP
   address being appended to it.  Results passed back in the equivalent of CdbPtr->DBaddr->MAC[0-5].

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiIp2Mac (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  PXE_CPB_MCAST_IP_TO_MAC *CpbPtr;
  PXE_DB_MCAST_IP_TO_MAC * DbPtr;
  UINT32                   IpAddr;
  UINT8 *                  TmpPtr;

  CpbPtr  = (PXE_CPB_MCAST_IP_TO_MAC *) (UINTN) CdbPtr->CPBaddr;
  DbPtr   = (PXE_DB_MCAST_IP_TO_MAC *) (UINTN) CdbPtr->DBaddr;

  DEBUGPRINT (DECODE, ("IntelgbeUndiIp2Mac\n"));

  if ((CdbPtr->OpFlags & PXE_OPFLAGS_MCAST_IPV6_TO_MAC) != 0) {
    UINT8 *Ipv6Ptr;

    Ipv6Ptr    = (UINT8 *) &CpbPtr->IP.IPv6;

    DbPtr->MAC[0] = 0x33;
    DbPtr->MAC[1] = 0x33;
    DbPtr->MAC[2] = *(Ipv6Ptr + 12);
    DbPtr->MAC[3] = *(Ipv6Ptr + 13);
    DbPtr->MAC[4] = *(Ipv6Ptr + 14);
    DbPtr->MAC[5] = *(Ipv6Ptr + 15);
    return;
  }

  // Take the last 23 bits of IP to generate a multicase IP address.
  IpAddr        = CpbPtr->IP.IPv4;
  TmpPtr        = (UINT8 *) (&IpAddr);

  DbPtr->MAC[0] = 0x01;
  DbPtr->MAC[1] = 0x00;
  DbPtr->MAC[2] = 0x5E;
  DbPtr->MAC[3] = (UINT8) (TmpPtr[1] & 0x7F);
  DbPtr->MAC[4] = (UINT8) TmpPtr[2];
  DbPtr->MAC[5] = (UINT8) TmpPtr[3];
  return;
}

/** This routine is used to read and write non-volatile storage on the NIC (if supported).  The NVRAM
   could be EEPROM, FLASH, or battery backed RAM.

   This is an optional function according to the UNDI specification  (or will be......)

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiNvData (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  DEBUGPRINT (DECODE, ("IntelgbeUndiNvData\n"));
  return;
}

/** This routine returns the current interrupt status and/or the transmitted buffer addresses.

   If the current interrupt status is returned, pending interrupts will be acknowledged by this
   command.  Transmitted buffer addresses that are written to the DB are removed from the transmit
   buffer queue.
   Normally, this command would be polled with interrupts disabled.
   The transmit buffers are returned in CdbPtr->DBaddr->TxBufer[0 - NumEntries].
   The interrupt status is returned in CdbPtr->StatFlags.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the 1-Gigabit
                             UNDI driver is layering on..

   @retval   None
**/
VOID
IntelgbeUndiStatus (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  PXE_DB_GET_STATUS *       DbPtr;
  UINT32                    IntStatus;
  UINT16                    NumEntries;

  DEBUGPRINT (DECODE, ("IntelgbeUndiStatus\n"));

  if (GigAdapter->DriverBusy) {
    DEBUGPRINT (DECODE,
      ("ERROR: IntelgbeUndiStatus called when driver busy\n"));
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
    CdbPtr->StatCode = PXE_STATCODE_BUSY;
    return;
  }

  // If the size of the DB is not large enough to store at least one 64 bit
  // complete transmit buffer address and size of the next available receive
  // packet we will return an error.  Per E.4.16 of the EFI spec the DB should
  // have enough space for at least 1 completed transmit buffer.
  if (CdbPtr->DBsize < (sizeof (UINT64) * 2)) {
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
    CdbPtr->StatCode = PXE_STATCODE_INVALID_CDB;
    DEBUGPRINT (CRITICAL, ("Invalid CDB\n"));
    if ((CdbPtr->OpFlags & PXE_OPFLAGS_GET_TRANSMITTED_BUFFERS) != 0) {
      CdbPtr->StatFlags |= PXE_STATFLAGS_GET_STATUS_NO_TXBUFS_WRITTEN;
    }

    return;
  }

  DbPtr = (PXE_DB_GET_STATUS *) (UINTN) CdbPtr->DBaddr;
  if ((CdbPtr->OpFlags & PXE_OPFLAGS_GET_TRANSMITTED_BUFFERS) != 0) {
    // Calculate the number of entries available in the DB to save the addresses
    // of completed transmit buffers.
    NumEntries = (UINT16)
    ((CdbPtr->DBsize - sizeof (UINT64)) / sizeof (UINT64));
    DEBUGPRINT (DECODE, ("CdbPtr->DBsize = %d\n", CdbPtr->DBsize));
    DEBUGPRINT (DECODE, ("NumEntries in DbPtr = %d\n", NumEntries));

    // On return NumEntries will be the number of TX buffers written into the DB
    NumEntries =
    IntelgbeFreeTxBuffers (GigAdapter, NumEntries, DbPtr->TxBuffer);
    if (NumEntries == 0) {
      CdbPtr->StatFlags |= PXE_STATFLAGS_GET_STATUS_NO_TXBUFS_WRITTEN;
    }

    // The receive buffer size and reserved fields take up the first 64 bits of the DB
    // The completed transmit buffers take up the rest
    CdbPtr->DBsize = (UINT16) (sizeof (UINT64) + NumEntries * sizeof (UINT64));
    DEBUGPRINT (DECODE, ("Return DBsize = %d\n", CdbPtr->DBsize));
  }

  if ((CdbPtr->OpFlags & PXE_OPFLAGS_GET_INTERRUPT_STATUS) != 0) {
    u32 i, j;
    j = MAX(GigAdapter->txqnum, GigAdapter->rxqnum);
    for (i = 0; i < j; i++) {
      struct intelgbe_rx_queue *rx_queue = &GigAdapter->rx_queue[i];
      IntStatus = INTELGBE_READ_REG(&GigAdapter->Hw, DMA_INTR_STATUS_CH(i));
      if (IntStatus & BIT(15)) {
        if (IntStatus & BIT(0)) {
          DEBUGPRINT(DECODE, ("Transmit Interrupt\n"));
          CdbPtr->StatFlags |= PXE_STATFLAGS_GET_STATUS_TRANSMIT;
          INTELGBE_WRITE_REG(&GigAdapter->Hw, DMA_INTR_STATUS_CH(i), BIT(0));
        }
      }
      else if (IntStatus & BIT(14)) {
        DEBUGPRINT(CRITICAL, ("Abnormal Interrupt %x\n", IntStatus));
        INTELGBE_WRITE_REG(&GigAdapter->Hw, DMA_INTR_STATUS_CH(i), IntStatus);
      }
      IntStatus = INTELGBE_READ_REG(&GigAdapter->Hw,
                                    DMA_INTR_STATUS_CH(rx_queue->chan));
      if (IntStatus & BIT(15)) {
        if (IntStatus & BIT(6)) {
          DEBUGPRINT(DECODE, ("Receive Interrupt\n"));
          INTELGBE_WRITE_REG(&GigAdapter->Hw, DMA_INTR_STATUS_CH(rx_queue->chan),
                             BIT(6));
          CdbPtr->StatFlags |= PXE_STATFLAGS_GET_STATUS_RECEIVE;
        }
      } else if (IntStatus & BIT(14)) {
        DEBUGPRINT(CRITICAL, ("Abnormal Interrupt %x\n", IntStatus));
        INTELGBE_WRITE_REG(&GigAdapter->Hw, DMA_INTR_STATUS_CH(rx_queue->chan),
                           IntStatus);
      }
    }
  }
  // Return current media status
  if ((CdbPtr->OpFlags & PXE_OPFLAGS_GET_MEDIA_STATUS) != 0) {
    if (!IsLinkUp (GigAdapter)) {
      CdbPtr->StatFlags |= PXE_STATFLAGS_GET_STATUS_NO_MEDIA;
      DEBUGPRINT (DECODE, ("Media disconnected\n"));
    }
  }

  CdbPtr->StatFlags |= PXE_STATFLAGS_COMMAND_COMPLETE;
  CdbPtr->StatCode = PXE_STATCODE_SUCCESS;

  return;
}

/** This routine is used to fill media header(s) in transmit packet(s).

   Copies the MAC address into the media header whether it is dealing
   with fragmented or non-fragmented packets.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                             UNDI driver is layering on.

   @retval      None
**/
VOID
IntelgbeUndiFillHeader (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  PXE_CPB_FILL_HEADER *           Cpb;
  PXE_CPB_FILL_HEADER_FRAGMENTED *Cpbf;
  ETHER_HEADER *                  MacHeader;
  UINTN                           i;

  DEBUGPRINT (DECODE, ("IntelgbeUndiFillHeader\n"));

  if (CdbPtr->CPBsize == PXE_CPBSIZE_NOT_USED) {
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
    CdbPtr->StatCode = PXE_STATCODE_INVALID_CDB;
    return;
  }

  if ((CdbPtr->OpFlags & PXE_OPFLAGS_FILL_HEADER_FRAGMENTED) != 0) {
    Cpbf = (PXE_CPB_FILL_HEADER_FRAGMENTED *) (UINTN) CdbPtr->CPBaddr;

    // Assume 1st fragment is big enough for the mac header.
    if ((Cpbf->FragCnt == 0)
      || (Cpbf->FragDesc[0].FragLen < PXE_MAC_HEADER_LEN_ETHER)) {
      // No buffers given.
      CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
      CdbPtr->StatCode = PXE_STATCODE_INVALID_CDB;
      return;
    }

    MacHeader = (ETHER_HEADER *) (UINTN) Cpbf->FragDesc[0].FragAddr;

    // We don't swap the protocol bytes.
    MacHeader->Type = Cpbf->Protocol;

    DEBUGPRINT (DECODE, ("MacHeader->SrcAddr = "));
    for (i = 0; i < PXE_HWADDR_LEN_ETHER; i++) {
      MacHeader->DestAddr[i] = Cpbf->DestAddr[i];
      MacHeader->SrcAddr[i] = Cpbf->SrcAddr[i];
      DEBUGPRINT (DECODE, ("%x ", MacHeader->SrcAddr[i]));
    }

    DEBUGPRINT (DECODE, ("\n"));
  } else {
    Cpb       = (PXE_CPB_FILL_HEADER *) (UINTN) CdbPtr->CPBaddr;
    MacHeader = (ETHER_HEADER *) (UINTN) Cpb->MediaHeader;

    // We don't swap the protocol bytes.
    MacHeader->Type = Cpb->Protocol;

    DEBUGPRINT (DECODE, ("MacHeader->SrcAddr = "));
    for (i = 0; i < PXE_HWADDR_LEN_ETHER; i++) {
      MacHeader->DestAddr[i] = Cpb->DestAddr[i];
      MacHeader->SrcAddr[i] = Cpb->SrcAddr[i];
      DEBUGPRINT (DECODE, ("%x ", MacHeader->SrcAddr[i]));
    }

    DEBUGPRINT (DECODE, ("\n"));
  }

  DEBUGWAIT (DECODE);
  return;
}

/** This routine is used to place a packet into the transmit queue.

   The data buffers given to this command are to be considered locked and the application or
   network driver loses ownership of these buffers and must not free or relocate them until
   the ownership returns.
   When the packets are transmitted, a transmit complete interrupt is generated (if interrupts
   are disabled, the transmit interrupt status is still set and can be checked using the UNDI_Status
   command.
   Some implementations and adapters support transmitting multiple packets with one transmit
   command.  If this feature is supported, the transmit CPBs can be linked in one transmit
   command.
   All UNDIs support fragmented frames, now all network devices or protocols do.  If a fragmented
   frame CPB is given to UNDI and the network device does not support fragmented frames
   (see !PXE.Implementation flag), the UNDI will have to copy the fragments into a local buffer
   before transmitting.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiTransmit (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  DEBUGPRINT (DECODE, ("IntelgbeUndiTransmit\n"));

  if (GigAdapter->DriverBusy) {
    DEBUGPRINT (DECODE,
      ("ERROR: IntelgbeUndiTransmit called when driver busy\n"));
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
    CdbPtr->StatCode = PXE_STATCODE_BUSY;
    return;
  }

  if (CdbPtr->CPBsize == PXE_CPBSIZE_NOT_USED) {
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
    CdbPtr->StatCode = PXE_STATCODE_INVALID_CDB;
    return;
  }


  CdbPtr->StatCode = (PXE_STATCODE) IntelgbeTransmit (GigAdapter,
    CdbPtr->CPBaddr, CdbPtr->OpFlags);

  if (CdbPtr->StatCode == PXE_STATCODE_SUCCESS) {
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_COMPLETE;
  } else {
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
  }

  return;
}

/** When the network adapter has received a frame, this command is used to copy the frame
   into the driver/application storage location.

   Once a frame has been copied, it is removed from the receive queue.

   @param[in]   CdbPtr        Pointer to the command descriptor block.
   @param[in]   GigAdapter   Pointer to the NIC data structure information which the
                              UNDI driver is layering on..

   @retval      None
**/
VOID
IntelgbeUndiReceive (
  IN PXE_CDB *        CdbPtr,
  IN GIG_DRIVER_DATA *GigAdapter
  )
{
  DEBUGPRINT (DECODE, ("IntelgbeUndiReceive\n"));

  if (GigAdapter->DriverBusy) {
    DEBUGPRINT (DECODE,
      ("ERROR: IntelgbeUndiReceive called while driver busy\n"));
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
    CdbPtr->StatCode = PXE_STATCODE_BUSY;
    return;
  }

  // Check if RU has started.
  if (!GigAdapter->ReceiveStarted) {
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
    CdbPtr->StatCode = PXE_STATCODE_NOT_INITIALIZED;
    return;
  }

  CdbPtr->StatCode = (UINT16) IntelgbeReceive (GigAdapter,
    CdbPtr->CPBaddr, CdbPtr->DBaddr);

  if (CdbPtr->StatCode == PXE_STATCODE_SUCCESS) {
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_COMPLETE;
  } else {
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
  }
  return;
}

/** This is the main SW UNDI API entry using the newer nii protocol.
   The parameter passed in is a 64 bit flat model virtual
   address of the Cdb.  We then jump into the common routine for both old and
   new nii protocol entries.

   @param[in]   Cdb   Pointer to the command descriptor block.

   @retval   None
**/
EFI_STATUS
EFIAPI
IntelgbeUndiApiEntry (
  IN  UINT64 Cdb
  )
{
  PXE_CDB *        CdbPtr;
  GIG_DRIVER_DATA *GigAdapter;
  UNDI_CALL_TABLE *TabPtr;

  DEBUGPRINT (DECODE, ("IntelgbeUndiApiEntry\n"));

  if (Cdb == (UINT64) 0) {
    return EFI_INVALID_PARAMETER;
  }

  CdbPtr = (PXE_CDB *) (UINTN) Cdb;

  if (CdbPtr->IFnum > mIntelgbePxe31->IFcnt) {
    DEBUGPRINT (DECODE, ("Invalid IFnum %d\n", CdbPtr->IFnum));
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
    CdbPtr->StatCode  = PXE_STATCODE_INVALID_CDB;
    return EFI_INVALID_PARAMETER;
  }

  GigAdapter = &(mIntelgbeUndi32DeviceList[CdbPtr->IFnum]->NicInfo);

  // Check if InitUndiNotifyExitBs was called before
  if (GigAdapter->ExitBootServicesTriggered) {
    DEBUGPRINT (CRITICAL, ("Pci Bus Mastering Disabled !\n"));
    CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
    CdbPtr->StatCode  = PXE_STATCODE_NOT_INITIALIZED;
    return EFI_INVALID_PARAMETER;
  }

  GigAdapter->VersionFlag = 0x31; // entering from new entry point

  DEBUGPRINT (DECODE, ("StatCode, OpCode, or StatFlags.: %x, %x, %x\n",
    CdbPtr->StatCode, CdbPtr->OpCode, CdbPtr->StatFlags));

  // Check the OPCODE range.
  if ((CdbPtr->OpCode > PXE_OPCODE_LAST_VALID) ||
    (CdbPtr->StatCode != PXE_STATCODE_INITIALIZE) ||
    (CdbPtr->StatFlags != PXE_STATFLAGS_INITIALIZE)) {
    DEBUGPRINT (DECODE, ("Invalid StatCode, OpCode, or StatFlags.:%x, %x, %x\n",
      CdbPtr->StatCode, CdbPtr->OpCode, CdbPtr->StatFlags));
    goto BadCdb;
  }

  if (CdbPtr->CPBsize == PXE_CPBSIZE_NOT_USED) {
    if (CdbPtr->CPBaddr != PXE_CPBADDR_NOT_USED) {
      goto BadCdb;
    }
  } else if (CdbPtr->CPBaddr == PXE_CPBADDR_NOT_USED) {
    goto BadCdb;
  }

  if (CdbPtr->DBsize == PXE_DBSIZE_NOT_USED) {
    if (CdbPtr->DBaddr != PXE_DBADDR_NOT_USED) {
      goto BadCdb;
    }
  } else if (CdbPtr->DBaddr == PXE_DBADDR_NOT_USED) {
    goto BadCdb;
  }
  TabPtr = &mIntelgbeApiTable[CdbPtr->OpCode];

  // Check if cpbsize and dbsize are as needed.
  // Check if opflags are as expected.
  if (TabPtr->CpbSize != (UINT16) (DONT_CHECK)
    && TabPtr->CpbSize != CdbPtr->CPBsize)
  {
    goto BadCdb;
  }

  if (TabPtr->DbSize != (UINT16) (DONT_CHECK)
    && TabPtr->DbSize != CdbPtr->DBsize)
  {
    goto BadCdb;
  }

  if (TabPtr->OpFlags != (UINT16) (DONT_CHECK)
    && TabPtr->OpFlags != CdbPtr->OpFlags)
  {
    goto BadCdb;
  }

  GigAdapter = &(mIntelgbeUndi32DeviceList[CdbPtr->IFnum]->NicInfo);

  // Check if UNDI_State is valid for this call.
  if (TabPtr->State != (UINT16) (-1)) {

    // Should atleast be started.
    if (GigAdapter->State == PXE_STATFLAGS_GET_STATE_STOPPED) {
      CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
      CdbPtr->StatCode  = PXE_STATCODE_NOT_STARTED;
      return EFI_NOT_READY;
    }

    // Check if it should be initialized.
    if (TabPtr->State == 2) {
      if (GigAdapter->State != PXE_STATFLAGS_GET_STATE_INITIALIZED) {
        CdbPtr->StatCode  = PXE_STATCODE_NOT_INITIALIZED;
        CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
        return EFI_NOT_READY;
      }
    }
  }

  // Set the return variable for success case here.
  CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_COMPLETE;
  CdbPtr->StatCode  = PXE_STATCODE_SUCCESS;

  TabPtr->ApiPtr (CdbPtr, GigAdapter);
  return EFI_SUCCESS;

BadCdb:
  DEBUGPRINT (DECODE, ("BadCdb\n"));
  CdbPtr->StatFlags = PXE_STATFLAGS_COMMAND_FAILED;
  CdbPtr->StatCode  = PXE_STATCODE_INVALID_CDB;
  return EFI_NOT_READY;
}
