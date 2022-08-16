/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef INTELGBE_H_
#define INTELGBE_H_

#include <Uefi.h>

#include <Base.h>
#include <Guid/EventGroup.h>
#include <Protocol/PciIo.h>
#include <Protocol/NetworkInterfaceIdentifier.h>
#include <Protocol/DevicePath.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/DriverDiagnostics.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/DriverSupportedEfiVersion.h>
#include <Protocol/PlatformToDriverConfiguration.h>
#include <Protocol/FirmwareManagement.h>
#include <Protocol/DriverHealth.h>

#include <Protocol/HiiConfigRouting.h>
#include <Protocol/FormBrowser2.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiString.h>

#include <Guid/MdeModuleHii.h>

#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PrintLib.h>

#include <IndustryStandard/Pci.h>

#include "AdapterInformation.h"
#include "Dma.h"
#include "Intelgbe_osdep.h"
#include "intelgbe_api.h"
#include "intelgbe_stmmac.h"
#include "intelgbe_defines.h"
#include "Version.h"
#include "ComponentName.h"
#include "StartStop.h"

// Debug levels for driver DEBUG_PRINT statements
#define NONE        0
#define INIT        (1 << 0)
#define DECODE      (1 << 1)
#define INTELGBE    (1 << 2)
#define SHARED      (1 << 3)
#define DIAG        (1 << 4)
#define CFG         (1 << 5)
#define IO          (1 << 6)
#define VLAN        (1 << 7)
#define CRITICAL    (1 << 8)
#define CLP         (1 << 9)
#define TX          (1 << 10)
#define RX          (1 << 11)
#define HW          (1 << 12)
#define HII         (1 << 13)
#define IMAGE       (1 << 14)
#define WAIT        (1 << 15)
#define FLASH       (1 << 16)
#define HEALTH      (1 << 17)
#define ADAPTERINFO (1 << 18)
#define DMA         (1 << 19)
#define PHYFUNC     (1 << 20)

#define DBG_LVL None

// This is the Macro Section
#if DBG_LVL
/** When specific debug level is currently set this macro
   prints debug message.

   @param[in]   Lvl   Debug level
   @param[in]   Msg   Debug message

   @param[in]   Msg printed or not according to Lvl
**/
#define DEBUGPRINT(Lvl, Msg) \
  if ((DBG_LVL & Lvl) != 0) { AsciiPrint Msg;}

/** When specific debug level is currently set this macro
   stops execution and waits until user presses ENTER.

   @param[in]   Lvl   Debug level

   @return   Execution of code resumed after ENTER is pressed
**/
#define DEBUGWAIT(Lvl)
#if 0
  if ((DBG_LVL & Lvl) != 0) { \
    WaitForEnter (); \
  }
#endif
/** When specific debug level is currently set this macro
   DEBUGPRINTS current timestamp

   @param[in]   Lvl   Debug level

   @return   Timestamp printed or not according to Lvl
**/
#define DEBUGPRINTTIME(Lvl) \
  if ((DBG_LVL & Lvl) != 0) \
    { gSystemTable->RuntimeServices->GetTime (&gTime, NULL);}; \
  DEBUGPRINT (Lvl, ("Timestamp - %dH:%dM:%dS:%dNS\n", \
      gTime.Hour, gTime.Minute, gTime.Second, gTime.Nanosecond));
#else /* NOT DBG_LVL */

// Comment out the debug stuff
/** When DBG_LVL is not defined leave occurences of DEBUGPRINT blank

   @param[in]   Lvl   Debug level
   @param[in]   Msg   Debug message

   @return   None
**/
#define DEBUGPRINT(Lvl, Msg)

/** When DBG_LVL is not defined leave occurences of DEBUGWAIT blank

   @param[in]   Lvl   Debug level

   @return   None
**/
#define DEBUGWAIT(Lvl)
#endif /* DBG_LVL */

#define GIG_UNDI_DEV_SIGNATURE   SIGNATURE_32 ('P', 'R', '0', 'g')

#define MAX_NIC_INTERFACES  256

// PCI Base Address Register Bits
#define PCI_BAR_IO_MASK             0x00000003
#define PCI_BAR_IO_MODE             0x00000001

#define PCI_BAR_MEM_MASK            0x0000000F
#define PCI_BAR_MEM_MODE            0x00000000
#define PCI_BAR_MEM_64BIT           0x00000004

// Device and Vendor IDs
#define INTEL_VENDOR_ID         0x8086

#define EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL_REVISION_31 0x00010001
#define PXE_ROMID_MINORVER_31 0x10

#define PCI_CLASS_MASK          0xFF00
#define PCI_SUBCLASS_MASK       0x00FF

#define EHL_PCH_STMMAC_SGMII_DID       0x4B32
#define EHL_PSE0_STMMAC_RGMII1G_DID    0x4BA0
#define EHL_PSE0_STMMAC_SGMII1G_DID    0x4BA1
#define EHL_PSE0_STMMAC_SGMII2G5_DID   0x4BA2
#define EHL_PSE1_STMMAC_RGMII1G_DID    0x4BB0
#define EHL_PSE1_STMMAC_SGMII1G_DID    0x4BB1
#define EHL_PSE1_STMMAC_SGMII2G5_DID   0x4BB2
#define TGLH_PCH1_STMMAC_SGMII_DID     0x43AC   // BDF: 0.1E.4
#define TGLH_PCH2_STMMAC_SGMII_DID     0x43A2   // BDF: 0.1E.5
#define TGLU_PCH1_STMMAC_SGMII_DID     0xA0AC

/* If the surprise removal has been detected,
 *  Device Status Register returns 0xFFFFFFFF */
#define INVALID_STATUS_REGISTER_VALUE  0xFFFFFFFF

// UNDI_CALL_TABLE.state can have the following values
#define DONT_CHECK -1
#define ANY_STATE -1
#define MUST_BE_STARTED 1
#define MUST_BE_INITIALIZED 2

/** Retrieves UNDI_PRIVATE_DATA structure using AIP protocol instance

   @param[in]   a   Current protocol instance

   @return    UNDI_PRIVATE_DATA structure instance
**/
#define UNDI_PRIVATE_DATA_FROM_AIP(a) \
  CR (a, UNDI_PRIVATE_DATA, AdapterInformation, GIG_UNDI_DEV_SIGNATURE)

/** Retrieves UNDI_PRIVATE_DATA structure using NII Protocol 3.1 instance

   @param[in]   a   Current protocol instance

   @return    UNDI_PRIVATE_DATA structure instance
**/
#define UNDI_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, UNDI_PRIVATE_DATA, NiiProtocol31, GIG_UNDI_DEV_SIGNATURE)

/** Retrieves UNDI_PRIVATE_DATA structure using DriverStop protocol instance

   @param[in]   a   Current protocol instance

   @return    UNDI_PRIVATE_DATA structure instance
**/
#define UNDI_PRIVATE_DATA_FROM_DRIVER_STOP(a) \
  CR (a, UNDI_PRIVATE_DATA, DriverStop, GIG_UNDI_DEV_SIGNATURE)

/** Test bit mask against a value.
 *
 *    @param[in]   v   Value
 *    @param[in]   m   Mask
 *
 *    @return    TRUE when value contains whole bit mask, otherwise - FALSE.
 *  */
#define BIT_TEST(v, m) (((v) & (m)) == (m))

/** Macro to compare MAC addresses.  Returns true if the MAC addresses match.
   a and b must be UINT8 pointers to the first byte of MAC address.

   @param[in]   a   Pointer to MAC address to compare with b
   @param[in]   b   Pointer to MAC address to compare with a

   @retval  TRUE     if MAC addresses match
   @retval  FALSE    MAC addresses don't match
**/
#ifdef EFI64
#define INTELGBE_COMPARE_MAC(a, b) \
  ( (a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]) && (a[3] == b[3]) &&
    (a[4] == b[4]) && (a[5] == b[5]))
#else /* NOT EFI64 */
#define INTELGBE_COMPARE_MAC(a, b) \
  ( *((UINT32 *) a) == *((UINT32 *) b) ) && ( *((UINT16 *) (a + 4)) == \
    *((UINT16 *) (b + 4)) )
#endif /* EFI64 */

/** Macro to copy MAC address b to a.
   a and b must be UINT8 pointers to the first byte of MAC address.

   @param[in]   a   Pointer to MAC address to copy to
   @param[in]   b   Pointer to MAC address to copy from

   @return   MAC b copied to a
**/
#ifdef EFI64
#define INTELGBE_COPY_MAC(a, b) \
  a[0] = b[0];a[1] = b[1];a[2] = b[2];a[3] = b[3];a[4] = b[4];a[5] = b[5];
#else /* NOT EFI64 */
#define INTELGBE_COPY_MAC(a, b) \
  *((UINT32 *) a) = *((UINT32 *) b);*((UINT16 *) (a + 4)) = \
                                                          *((UINT16 *) (b + 4))
#endif /* EFI64 */

struct intelgbe_hw;

typedef
VOID
(* BS_PTR) (
  UINT64  UnqId,
  UINTN   MicroSeconds
  );


/* Function pointers for the MAC. */
struct intelgbe_mac_operations {
  s32  (*init_params)(struct intelgbe_hw *);
  s32  (*check_for_link)(struct intelgbe_hw *, bool *);
  s32  (*get_bus_info)(struct intelgbe_hw *);
  s32  (*get_link_up_info)(struct intelgbe_hw *, u16 *, u16 *);
  void (*update_mc_addr_list)(struct intelgbe_hw *, u8 *, u32);
  s32  (*reset_hw)(struct intelgbe_hw *);
  s32  (*init_hw)(struct intelgbe_hw *);
  s32  (*uninit_hw)(struct intelgbe_hw *);
  s32  (*setup_link)(struct intelgbe_hw *);
  s32  (*setup_physical_interface)(struct intelgbe_hw *);
  s32  (*read_mac_addr)(struct intelgbe_hw *);
  s32  (*write_mac_addr)(struct intelgbe_hw *);
  s32  (*get_id)(struct intelgbe_hw *);
};

enum intelgbe_mac_type {
  intelgbe_undefined = 0,
  intelgbe_stmmac_rgmii,
  intelgbe_stmmac_sgmii,
};

struct intelgbe_mac_info {
  struct intelgbe_mac_operations ops;
  u8 addr[ETH_ADDR_LEN];
  u8 perm_addr[ETH_ADDR_LEN];

  enum intelgbe_mac_type type;

  u32 txfifosz;
  u32 rxfifosz;
  u32 link_speed;
  u32 full_duplex;
  bool speed_2500_en;
  bool pse_gbe;
};

struct intelgbe_phy_operations {
  s32  (*init_params)(struct intelgbe_hw *);
  s32  (*init)(struct intelgbe_hw *);
  s32  (*mdio_write)(struct intelgbe_hw *, UINT32, UINT8, UINT16, UINT16, bool);
  s32  (*mdio_read)(struct intelgbe_hw *, UINT32, UINT8, UINT16, UINT32*, bool);
  s32  (*cfg_link)(struct intelgbe_hw *);
  s32  (*link_status_change)(struct intelgbe_hw *, bool *);
  s32  (*status)(struct intelgbe_hw *, bool *, s32 *, s8 *);
};

enum phy_type {
  PHY_MARVELL_88E1512,
  PHY_MAXLINEAR_GPY211,
  PHY_MARVELL_88E2110,
};

enum phy_interface {
  PHY_INTERFACE_MII,
  PHY_INTERFACE_GMII,
  PHY_INTERFACE_SGMII,
  PHY_INTERFACE_RGMII,
};

enum phy_support {
  PHY_SUPPORT_10_HALF    = BIT(0),
  PHY_SUPPORT_10_FULL    = BIT(1),
  PHY_SUPPORT_100_HALF   = BIT(2),
  PHY_SUPPORT_100_FULL   = BIT(3),
  PHY_SUPPORT_1000_HALF  = BIT(4),
  PHY_SUPPORT_1000_FULL  = BIT(5),
};

struct intelgbe_phy_info {
  struct intelgbe_phy_operations ops;
  u32 addr;
  u32 mdio_csr_clk;
  u32 id;
  enum phy_type type;
  enum phy_interface interface;
  enum phy_support support;
  bool autoneg;
  u32 link_speed;
  u32 full_duplex;
  bool link_up;
  u32 reset_delay_us; /* in usec */
  u32 revision;
  bool c45;
};

struct intelgbe_hw {
  void *back;
  u8 *hw_addr;
  unsigned long io_base;
  struct intelgbe_mac_info  mac;
  struct intelgbe_phy_info  phy;
  u16 device_id;
  u16 subsystem_vendor_id;
  u16 subsystem_device_id;
  u16 vendor_id;
  u8  revision_id;
};

#pragma pack(1)
typedef struct {
  UINT8  DestAddr[PXE_HWADDR_LEN_ETHER];
  UINT8  SrcAddr[PXE_HWADDR_LEN_ETHER];
  UINT16 Type;
} ETHER_HEADER;

#pragma pack(1)
typedef struct {
  UINT16 VendorId;
  UINT16 DeviceId;
  UINT16 Command;
  UINT16 Status;
  UINT16 RevId;
  UINT16 ClassId;
  UINT8  CacheLineSize;
  UINT8  LatencyTimer;
  UINT8  HeaderType;
  UINT8  Bist;
  UINT32 BaseAddressReg0;
  UINT32 BaseAddressReg1;
  UINT32 BaseAddressReg2;
  UINT32 BaseAddressReg3;
  UINT32 BaseAddressReg4;
  UINT32 BaseAddressReg5;
  UINT32 CardBusCisPtr;
  UINT16 SubVendorId;
  UINT16 SubSystemId;
  UINT32 ExpansionRomBaseAddr;
  UINT8  CapabilitiesPtr;
  UINT8  Reserved1;
  UINT16 Reserved2;
  UINT32 Reserved3;
  UINT8  IntLine;
  UINT8  IntPin;
  UINT8  MinGnt;
  UINT8  MaxLat;
} PCI_CONFIG_HEADER;
#pragma pack()

/** Retrieves RX descriptor from RX ring structure

   @param[in]   R   RX ring
   @param[in]   i   Number of descriptor

   @return   Descriptor retrieved
**/
#define INTELGBE_RX_DESC(R, i)          \
          (&(((struct intelgbe_rx_desc *) ((R)->UnmappedAddress))[i]))

typedef struct intelgbe_rx_desc INTELGBE_RECEIVE_DESCRIPTOR;


// TX Buffer size including crc and padding
#define RX_BUFFER_SIZE 2048

/*
  Following YOCTO implementation
*/
#define DEFAULT_RX_DESCRIPTORS 512
#define DEFAULT_TX_DESCRIPTORS 512

#pragma pack(1)
typedef struct {
  UINT8  RxBuffer[RX_BUFFER_SIZE - (sizeof (UINT64))];
  UINT64 BufferUsed;
} LOCAL_RX_BUFFER, *PLOCAL_RX_BUFFER;
#pragma pack()

typedef struct intelgbe_tx_desc INTELGBE_TRANSMIT_DESCRIPTOR;

/* UNDI callback functions typedefs */
typedef
VOID
(* BS_PTR) (
  UINT64  UnqId,
  UINTN   MicroSeconds
  );

typedef
VOID
(* VIRT_PHYS) (
  UINT64  UnqId,
  UINT64  VirtualAddr,
  UINT64  PhysicalPtr
  );

typedef
VOID
(* BLOCK) (
  UINT64  UnqId,
  UINT32  Enable
  );

typedef
VOID
(* MEM_IO) (
  UINT64  UnqId,
  UINT8   ReadWrite,
  UINT8   Len,
  UINT64  Port,
  UINT64  BufAddr
  );

typedef
VOID
(* MAP_MEM) (
  UINT64  UnqId,
  UINT64  VirtualAddr,
  UINT32  Size,
  UINT32  Direction,
  UINT64  MappedAddr
  );

typedef
VOID
(* UNMAP_MEM) (
  UINT64  UnqId,
  UINT64  VirtualAddr,
  UINT32  Size,
  UINT32  Direction,
  UINT64  MappedAddr
  );

typedef
VOID
(* SYNC_MEM) (
  UINT64  UnqId,
  UINT64  VirtualAddr,
  UINT32  Size,
  UINT32  Direction,
  UINT64  MappedAddr
  );

struct intelgbe_tx_queue {
  u32 queue_index;
  INTELGBE_TRANSMIT_DESCRIPTOR *tx_desc;
  INTELGBE_TRANSMIT_DESCRIPTOR *dma_tx;
  unsigned int cur_tx;
  unsigned int dirty_tx;
  u32 tx_tail_addr;
};

struct intelgbe_rx_queue {
  u32 queue_index;
  u32 chan;
  INTELGBE_RECEIVE_DESCRIPTOR *rx_desc;
  INTELGBE_RECEIVE_DESCRIPTOR *dma_rx;
  LOCAL_RX_BUFFER           *rx_buff;
  LOCAL_RX_BUFFER           *dma_rx_buff;
  unsigned int cur_rx;
  unsigned int dirty_rx;
  u32 rx_tail_addr;
};

typedef struct DRIVER_DATA_S {
  UINT16                State; // stopped, started or initialized
  struct intelgbe_hw      Hw;
  UINTN                 Segment;
  UINTN                 Bus;
  UINTN                 Device;
  UINTN                 Function;
  UINT8                 PciClass;
  UINT8                 PciSubClass;
  UINTN                 LanFunction;
  UINT32                PciConfig[MAX_PCI_CONFIG_LEN];
  UINTN                 HwInitialized;
  UINTN                 DriverBusy;
  UINT16                LinkSpeed; // requested (forced) link speed
  UINT8                 DuplexMode; // requested duplex
  UINT8                 CableDetect; // 1 to detect and 0 not to detect the cable
  UINT8                 LoopBack;
  UINT8                 UndiEnabled; // When 0 only HII and FMP are avaliable,
                                     // NII is not installed on ControllerHandle
                                     // (e.g. in case iSCSI driver loaded on port)

  UINT64               UniqueId;
  EFI_PCI_IO_PROTOCOL *PciIo;
  UINT64               OriginalPciAttributes;
  // UNDI callbacks
  BS_PTR               Delay;
  VIRT_PHYS            Virt2Phys;
  BLOCK                Block;
  MEM_IO               MemIo;
  MAP_MEM              MapMem;
  UNMAP_MEM            UnMapMem;
  SYNC_MEM             SyncMem;
  UINT8                IoBarIndex;
  UINT16               RxFilter;
  UINT8                IntMask;
  UINT8                txqnum;
  UINT8                rxqnum;
  UINT8                ReceiveStarted;
  UINT16               XmitDoneHead;
  UNDI_DMA_MAPPING     TxRing;
  UNDI_DMA_MAPPING     RxRing;
  UNDI_DMA_MAPPING     RxBufferMapping;
  UNDI_DMA_MAPPING     TxBufferMappings[DEFAULT_TX_DESCRIPTORS];
  /* RX Queue */
  struct intelgbe_rx_queue rx_queue[INTELGBE_MAX_RX_QUEUES];
  /* TX Queue */
  struct intelgbe_tx_queue tx_queue[INTELGBE_MAX_TX_QUEUES];

  UINT8                BroadcastNodeAddress[PXE_MAC_LENGTH];
  UINT8                DeviceId;
  BOOLEAN              MacAddrOverride;
  BOOLEAN              SurpriseRemoval;
  BOOLEAN              ExitBootServicesTriggered;
  UINTN                VersionFlag; // Indicates UNDI version 3.0 or 3.1
} GIG_DRIVER_DATA, *PADAPTER_STRUCT;

typedef struct {
  EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL *NiiProtocol31;
} EFI_NII_POINTER_PROTOCOL;


typedef struct UNDI_PRIVATE_DATA_S {
  UINTN                                     Signature;
  EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL NiiProtocol31;
  EFI_NII_POINTER_PROTOCOL                  NIIPointerProtocol;
  EFI_HANDLE                                ControllerHandle;
  EFI_HANDLE                                DeviceHandle;
  EFI_DEVICE_PATH_PROTOCOL *                Undi32BaseDevPath;
  EFI_DEVICE_PATH_PROTOCOL *                Undi32DevPath;
  EFI_ADAPTER_INFORMATION_PROTOCOL          AdapterInformation;
  GIG_DRIVER_DATA                           NicInfo;
  UINT8 AltMacAddrSupported;
  BOOLEAN                                   IsChildInitialized;
  EFI_DRIVER_STOP_PROTOCOL                  DriverStop;
  EFI_UNICODE_STRING_TABLE *                ControllerNameTable;
  CHAR16 *                                  Brand;
} UNDI_PRIVATE_DATA;

typedef struct {
  INTELGBE_RECEIVE_DESCRIPTOR
  RxRing[INTELGBE_MAX_RX_QUEUES][DEFAULT_RX_DESCRIPTORS];
  INTELGBE_TRANSMIT_DESCRIPTOR
  TxRing[INTELGBE_MAX_TX_QUEUES][DEFAULT_TX_DESCRIPTORS];
  LOCAL_RX_BUFFER
  RxBuffer[INTELGBE_MAX_RX_QUEUES][DEFAULT_RX_DESCRIPTORS];
} GIG_UNDI_DMA_RESOURCES;

typedef struct {
  UINT16 CpbSize;
  UINT16 DbSize;
  UINT16 OpFlags;
  UINT16 State;
  VOID (*ApiPtr)();
} UNDI_CALL_TABLE;

/* External variables declarations */
extern PXE_SW_UNDI *               mIntelgbePxe31;
extern UNDI_PRIVATE_DATA *        mIntelgbeUndi32DeviceList[MAX_NIC_INTERFACES];

/* External Global Variables */
extern UNDI_CALL_TABLE             mIntelgbeApiTable[];

#define BYTE_ALIGN_64    0x7F

/* We need enough space to store TX descriptors, RX descriptors,
 RX buffers, and enough left over to do a 64 byte alignment. */
#define RX_RING_SIZE    sizeof (((GIG_UNDI_DMA_RESOURCES*) 0)->RxRing)
#define TX_RING_SIZE    sizeof (((GIG_UNDI_DMA_RESOURCES*) 0)->TxRing)
#define RX_BUFFERS_SIZE sizeof (((GIG_UNDI_DMA_RESOURCES*) 0)->RxBuffer)

extern EFI_COMPONENT_NAME_PROTOCOL gUndiComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL gUndiComponentName2;
extern EFI_DRIVER_CONFIGURATION_PROTOCOL gGigUndiDriverConfiguration;
extern EFI_DRIVER_SUPPORTED_EFI_VERSION_PROTOCOL gUndiSupportedEfiVersion;
extern EFI_DRIVER_BINDING_PROTOCOL gUndiDriverBinding;
extern EFI_GUID gEfiNiiPointerGuid;
extern EFI_DRIVER_STOP_PROTOCOL  gUndiDriverStop;
extern EFI_GUID                  gEfiStartStopProtocolGuid;

/** This function performs PCI-E initialization for the device.
 *
 *  @param[in]   GigAdapter   Pointer to adapter structure
 *
 *  @retval   EFI_SUCCESS            PCI-E initialized successfully
 *  @retval   EFI_UNSUPPORTED        Failed to get supported PCI command options
 *  @retval   EFI_UNSUPPORTED        Failed to set PCI command options
 *  @retval   EFI_OUT_OF_RESOURCES   The memory pages for transmit and receive resources could
 *                                   not be allocated
 **/
EFI_STATUS
IntelgbePciInit (
  GIG_DRIVER_DATA *GigAdapter
  );

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
  GIG_DRIVER_DATA *GigAdapterInfo
  );

/** This routine blocks until auto-negotiation completes or times out (after 4.5 seconds).

   @param[in]   GigAdapter   Pointer to the NIC data structure information
                             which the UNDI driver is layering on..

   @retval   TRUE   Auto-negotiation completed successfully,
   @retval   FALSE  Auto-negotiation did not complete (i.e., timed out)
**/
BOOLEAN
IntelgbeWaitForAutoNeg (
  IN GIG_DRIVER_DATA *GigAdapter
  );

/** Detects surprise removal device status in PCI controller register

   @param[in]   Adapter   Pointer to the device instance

   @retval   TRUE    Surprise removal has been detected
   @retval   FALSE   Surprise removal has not been detected
**/
BOOLEAN
IsSurpriseRemoval (
  IN  GIG_DRIVER_DATA *Adapter
  );

/** Delay a specified number of microseconds

   @param[in]   Adapter        Pointer to the NIC data structure information
                               which the UNDI driver is layering on..
   @param[in]   MicroSeconds   Time to delay in Microseconds.

   @return   Execution of code delayed
**/
VOID
DelayInMicroseconds (
  IN GIG_DRIVER_DATA *Adapter,
  IN UINTN            MicroSeconds
  );

/** Checks if link is up

   @param[in]   GigAdapter   Pointer to the NIC data structure information
                             which the UNDI driver is layering on.

   @retval   TRUE   Link is up
   @retval   FALSE  Link is down
**/
BOOLEAN
IsLinkUp (
  GIG_DRIVER_DATA *GigAdapter
  );

/** Initializes the gigabit adapter, setting up memory addresses, MAC Addresses,
   Type of card, etc.

   @param[in]   GigAdapter   Pointer to adapter structure

   @retval   PXE_STATCODE_SUCCESS       Initialization succeeded
   @retval   PXE_STATCODE_NOT_STARTED   Hardware Init failed
**/
PXE_STATCODE
IntelgbeInititialize (
  GIG_DRIVER_DATA *GigAdapterInfo
  );

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
  GIG_DRIVER_DATA *GigAdapterInfo,
  UINT64           Cpb,
  UINT16           OpFlags
  );

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
  );

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
  GIG_DRIVER_DATA *GigAdapterInfo,
  UINT64           Cpb,
  UINT64           Db
  );

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
  );

/** Stop the hardware and put it all (including the PHY) into a known good state.

   @param[in]   GigAdapter   Pointer to the driver structure

   @retval   PXE_STATCODE_SUCCESS    Hardware stopped
**/
UINTN
IntelgbeShutdown (
  GIG_DRIVER_DATA *GigAdapter
  );

#endif /* INTELGBE_H_ */
