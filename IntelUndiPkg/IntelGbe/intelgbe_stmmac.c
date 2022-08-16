/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Intelgbe.h"
#include "marvell_88e1512.h"
#include "maxlinear_gpyxxx.h"
#include "marvell_88e2110.h"

/* MAC operations */
s32 intelgbe_mdio_write(struct intelgbe_hw *hw, UINT32 addr, UINT8 dev, UINT16 reg, UINT16 data,
  bool c45)
{
  u32 mdio_data = 0;
  u32 mdio_address = 0;
  s32 limit = 100;
  u32 val;

  if (c45) {
    mdio_address = MAC_MDIO_CLAUSE_45_PHY_EN;
    mdio_address |= (addr << MAC_MDIO_PA_SHIFT) & MAC_MDIO_PA_MASK;
    mdio_address |= (dev << MAC_MDIO_RDA_SHIFT) & MAC_MDIO_RDA_MASK;
    mdio_data = (reg << MAC_MDIO_RA_SHIFT) & MAC_MDIO_RA_MASK;
  } else {
    mdio_address = (addr << MAC_MDIO_PA_SHIFT) & MAC_MDIO_PA_MASK;
    mdio_address |= (reg << MAC_MDIO_RDA_SHIFT) & MAC_MDIO_RDA_MASK;
  }
  mdio_address |= MAC_MDIO_GMII_BUSY | MAC_MDIO_GMII_OPR_CMD_WRITE |
                  hw->phy.mdio_csr_clk;
  mdio_data |= (u32) data;
  while (limit--) {
    val = INTELGBE_READ_REG(hw, MAC_MDIO_ADDRESS_REG);
    if (!((val) & MAC_MDIO_GMII_BUSY)) {
      break;
    }
    msec_delay(1);
  }
  if (limit < 0) {
    return -INTELGBE_BUSY;
  }
  INTELGBE_WRITE_REG(hw, MAC_MDIO_DATA_REG, mdio_data);
  INTELGBE_WRITE_REG(hw, MAC_MDIO_ADDRESS_REG, mdio_address);
  limit = 100;
  while (limit--) {
    val = INTELGBE_READ_REG(hw, MAC_MDIO_ADDRESS_REG);
    if (!((val) & MAC_MDIO_GMII_BUSY)) {
      break;
    }
    msec_delay(1);
  }
  if (limit < 0) {
    return -INTELGBE_BUSY;
  }
  return INTELGBE_SUCCESS;
}

s32 intelgbe_mdio_read(struct intelgbe_hw *hw, UINT32 addr, UINT8 dev, UINT16 reg, UINT32* data,
  bool c45)
{
  u32 mdio_data = 0;
  u32 mdio_address = 0;
  s32 limit = 100;
  u32 val;

  if (c45) {
    mdio_address = MAC_MDIO_CLAUSE_45_PHY_EN;
    mdio_address |= (addr << MAC_MDIO_PA_SHIFT) & MAC_MDIO_PA_MASK;
    mdio_address |= (dev << MAC_MDIO_RDA_SHIFT) & MAC_MDIO_RDA_MASK;
    mdio_data    = (reg << MAC_MDIO_RA_SHIFT) & MAC_MDIO_RA_MASK;
  } else {
    mdio_address = (addr << MAC_MDIO_PA_SHIFT) & MAC_MDIO_PA_MASK;
    mdio_address |= (reg << MAC_MDIO_RDA_SHIFT) & MAC_MDIO_RDA_MASK;
  }
  mdio_address |= MAC_MDIO_GMII_BUSY | MAC_MDIO_GMII_OPR_CMD_READ |
                   hw->phy.mdio_csr_clk;
  while (limit--) {
    val = INTELGBE_READ_REG(hw, MAC_MDIO_ADDRESS_REG);
    if (!((val) & MAC_MDIO_GMII_BUSY)) {
      break;
    }
    msec_delay(1);
  }
  if (limit < 0) {
    return -INTELGBE_BUSY;
  }
  INTELGBE_WRITE_REG(hw, MAC_MDIO_DATA_REG, mdio_data);
  INTELGBE_WRITE_REG(hw, MAC_MDIO_ADDRESS_REG, mdio_address);
  limit = 100;
  while (limit--) {
    val = INTELGBE_READ_REG(hw, MAC_MDIO_ADDRESS_REG);
    if (!((val) & MAC_MDIO_GMII_BUSY)) {
      break;
    }
    msec_delay(1);
  }
  if (limit < 0) {
    return -INTELGBE_BUSY;
  }
  *data = INTELGBE_READ_REG(hw, MAC_MDIO_DATA_REG);
  return INTELGBE_SUCCESS;
}

STATIC s32 intelgbe_reset_controller(struct intelgbe_hw *hw)
{
  volatile u32 val;
  s32 limit = 10;

  DEBUGPRINT (INTELGBE, ("Entered reset controller \n"));
  val = INTELGBE_READ_REG(hw, INTELGBE_DMA_MODE);
  val |= INTELGBE_DMA_MD_SWR;
  INTELGBE_WRITE_REG(hw, INTELGBE_DMA_MODE, val);
  msec_delay(10);
  while (limit--) {
    val = INTELGBE_READ_REG(hw, INTELGBE_DMA_MODE);
    if (!((val) & INTELGBE_DMA_MD_SWR)) {
      break;
    }
    msec_delay(10);
  }
  if (limit < 0) {
    DEBUGPRINT (INTELGBE, (" reset controller failed\n"));
    return INTELGBE_ERR_RESET;
  }
  return INTELGBE_SUCCESS;
}

/**
 *  intelgbe_get_mac_version - Read MAC version
 *  @hw: pointer to the HW structure
 **/
STATIC s32 intelgbe_get_mac_version(struct intelgbe_hw *hw)
{
  u32 id;

  DEBUGFUNC("intelgbe_get_mac_version");
  id = INTELGBE_READ_REG(hw, INTELGBE_VERSION) & INTELGBE_VERSION_MASK;
  return id;
}

/**
 *  intelgbe_read_mac_addr - Read device MAC address
 *  @hw: pointer to the HW structure
 **/
STATIC s32 intelgbe_read_mac_addr(struct intelgbe_hw *hw)
{
  u32 rar_high;
  u32 rar_low;
  u16 i;

  DEBUGFUNC("intelgbe_read_mac_addr");
  rar_high = INTELGBE_READ_REG(hw, INTELGBE_ADDR_HIGH(0));
  rar_low = INTELGBE_READ_REG(hw, INTELGBE_ADDR_LOW(0));

  DEBUGPRINT (CRITICAL, ("MAC rar_low %x rar_high %x\n", rar_low, rar_high));
  for (i = 0; i < INTELGBE_ADDR_LOW_LEN; i++)
    hw->mac.perm_addr[i] = (u8)(rar_low >> (i*8));

  for (i = 0; i < INTELGBE_ADDR_HIGH_LEN; i++)
    hw->mac.perm_addr[i+4] = (u8)(rar_high >> (i*8));

  DEBUGPRINT (CRITICAL, ("MAC address (%d) :", ETH_ADDR_LEN));
  for (i = 0; i < ETH_ADDR_LEN; i++) {
    hw->mac.addr[i] = hw->mac.perm_addr[i];
    DEBUGPRINT (CRITICAL, ("%x ", hw->mac.addr[i]));
  }
  DEBUGPRINT (CRITICAL, ("\n"));

  return INTELGBE_SUCCESS;
}

/**
 *  intelgbe_write_mac_addr - Write back device MAC address
 *  @hw: pointer to the HW structure
 **/
STATIC s32 intelgbe_write_mac_addr(struct intelgbe_hw *hw)
{
  u32 data;
  u16 i;
  u8 addr[ETH_ADDR_LEN] = {0x00,0x00,0x00,0x00,0x00,0x00};

  DEBUGPRINT (CRITICAL, ("Saved MAC address: ", ETH_ADDR_LEN));
  for (i = 0; i < ETH_ADDR_LEN; i++) {
    addr[i] = hw->mac.addr[i];
    DEBUGPRINT (CRITICAL, ("%x ", addr[i]));
  }
  DEBUGPRINT (CRITICAL, ("\n"));

  data = (addr[5] << 8) | addr[4];
  /* For MAC Addr registers se have to set the Address Enable (AE)
  * bit that has no effect on the High Reg 0 where the bit 31 (MO)
  * is RO.
  */
  data |= (0 << 16);
  INTELGBE_WRITE_REG(hw, INTELGBE_ADDR_HIGH(0), data | BIT(31));
  data = (addr[3] << 24) | (addr[2] << 16) | (addr[1] << 8) | addr[0];
  INTELGBE_WRITE_REG(hw, INTELGBE_ADDR_LOW(0), data);

  return INTELGBE_SUCCESS;
}

static void intelgbe_dma_rx_desc_init(struct intelgbe_rx_queue *rx_queue)
{
  int i;

  for (i = 0; i < (DEFAULT_RX_DESCRIPTORS); i++) {
    INTELGBE_RECEIVE_DESCRIPTOR *desc = &rx_queue->rx_desc[i];

    DEBUGPRINT (DECODE,
      ("RX descriptor address VA: 0x%08llx PA: 0x%08llx BUFF VA: \
                            0x%08llx PA: 0x%08llx\n",
                            POINTER_TO_UINT(desc),
                            POINTER_TO_UINT(&rx_queue->dma_rx[i]),
                            POINTER_TO_UINT(&rx_queue->rx_buff[i]),
                            POINTER_TO_UINT(&rx_queue->dma_rx_buff[i])));
    desc->des0 = (u32)(u64) &rx_queue->dma_rx_buff[i];
    desc->des1 = 0;
    desc->des2 = 0;
    desc->des3 = (BIT(30) | BIT(24));
    MemoryFence();
    desc->des3 |= (BIT(31));
  }
  return;
}

static inline int intelgbe_mac_enable_interrupts(struct intelgbe_hw *hw)
{
  GIG_DRIVER_DATA *GigAdapterInfo = (GIG_DRIVER_DATA *)hw->back;
  u32 reg_val;
  int i;

  for (i = 0; i < MAX(GigAdapterInfo->txqnum, GigAdapterInfo->rxqnum); i++) {
    reg_val = DMA_CH_INTR_EN_NIE | DMA_CH_INTR_EN_RIE |
              DMA_CH_INTR_EN_TIE | BIT(14) | BIT(12);
    INTELGBE_WRITE_REG(hw, DMA_INTR_EN_CH(i), reg_val);
  }
  return 0;
}

static inline int intelgbe_mac_start_transaction(struct intelgbe_hw *hw)
{
  GIG_DRIVER_DATA *GigAdapterInfo = (GIG_DRIVER_DATA *)hw->back;
  u32 reg_val;
  int i;
  for (i = 0; i < GigAdapterInfo->rxqnum; i++) {
    struct intelgbe_rx_queue *rx_queue = &GigAdapterInfo->rx_queue[i];
    /* Enable DMA layer receive channels */
    reg_val = INTELGBE_READ_REG(hw, DMA_RX_CONTROL_CH(rx_queue->chan));
    reg_val |= DMA_CH_RX_CTRL_SR;
    INTELGBE_WRITE_REG(hw, DMA_RX_CONTROL_CH(rx_queue->chan), reg_val);
  }
  for (i = 0; i < GigAdapterInfo->txqnum; i++) {
    /* Enable MTL layer transmit queues */
    reg_val = INTELGBE_READ_REG(hw, MTL_TXQ_OPERATION_MODE(i));
    reg_val &= INV_MTL_TXQ_OPR_TXQEN;
    reg_val |= MTL_TXQ_OPR_TXQEN_EN;
    INTELGBE_WRITE_REG(hw, MTL_TXQ_OPERATION_MODE(i), reg_val);

    /* Enable DMA layer transmit channels */
    reg_val = (u32) INTELGBE_READ_REG(hw, DMA_TX_CONTROL_CH(i));
    reg_val |= DMA_CH_TX_CTRL_ST;
    INTELGBE_WRITE_REG(hw, DMA_TX_CONTROL_CH(i), reg_val);
  }
  /* Enable MAC layer receive & transmit */
  reg_val = INTELGBE_READ_REG(hw, MAC_CONFIGURATION);
  reg_val |= MAC_CONF_RE;
  reg_val |= MAC_CONF_TE;
  INTELGBE_WRITE_REG(hw, MAC_CONFIGURATION, reg_val);
  GigAdapterInfo->ReceiveStarted = TRUE;
  return 0;
}

static inline int intelgbe_mac_init(struct intelgbe_hw *hw)
{
  struct intelgbe_mac_info *mac = &hw->mac;
  struct intelgbe_phy_info *phy = &hw->phy;
  GIG_DRIVER_DATA *GigAdapterInfo = (GIG_DRIVER_DATA *)hw->back;
  u32 reg_val;
  int i;
  s32 link_speed;
  s8 duplex;
  bool link;

  /* Enable MAC auto pad/crc stripping for IEEE802.3 length packet,
   * crc stripping for EtherType (Ethernet II) type packet,
   * and RX checksum offload
   *
   * Note: Setting bit IPC would not take effect if HW IP has not
   *       enabled the checksum offload feature
   */
  reg_val = MAC_CONF_CST | MAC_CONF_ACS | MAC_CONF_IPC;
  INTELGBE_WRITE_REG(hw, MAC_CONFIGURATION, reg_val);
  if (phy->ops.status(hw, &link, &link_speed, &duplex) != 0) {
    DEBUGPRINT (CRITICAL, ("PHY not initialized \n"));
  }
  if (link) {
    mac->link_speed = link_speed;
    mac->full_duplex = duplex;
    phy->link_up = true;
  } else {
    phy->link_up = false;
  }
  DEBUGPRINT (CRITICAL, ("MAC configured for speed %dMbps ", mac->link_speed));
  reg_val = INTELGBE_READ_REG(hw, MAC_CONFIGURATION);
  reg_val &= INV_MAC_CONF_SPD;
  switch (mac->link_speed) {
  case 100:
    reg_val |= MAC_CONF_SPD_100MHZ;
    break;
  case 1000:
    reg_val |= MAC_CONF_SPD_1000MHZ;
    break;
  case 2500:
    reg_val |= MAC_CONF_SPD_2500MHZ;
    break;
  default:
    reg_val |= MAC_CONF_SPD_10MHZ;
    break;
  }

  if (mac->full_duplex) {
    DEBUGPRINT (CRITICAL, ("full duplex\n"));
    reg_val |= MAC_CONF_DM;
  } else {
    DEBUGPRINT (CRITICAL, ("half duplex\n"));
    reg_val &= ~MAC_CONF_DM;
  }
  INTELGBE_WRITE_REG(hw, MAC_CONFIGURATION, reg_val);

  /* Enable MAC RX queues to DCB/General mode */
  reg_val = 0;
  for (i = 0; i < GigAdapterInfo->rxqnum; i++) {
    reg_val |= MAC_EN_DCB_GEN_RXQ(i);
  }
  INTELGBE_WRITE_REG(hw, MAC_RXQ_CTRL0, reg_val);

  return 0;
}

static inline int intelgbe_mtl_init(struct intelgbe_hw *hw)
{
  struct intelgbe_mac_info *mac = &hw->mac;
  GIG_DRIVER_DATA *GigAdapterInfo = (GIG_DRIVER_DATA *)hw->back;
  u32 reg_val, txqsz, rxqsz;
  int i;

  /* Set MTL TX scheduling algo & RX arbitration algo to
   * strict priority
   */
  reg_val = MTL_OPR_MD_SCHALG_SP;
  INTELGBE_WRITE_REG(hw, MTL_OPERATION_MODE, reg_val);

  for (i = 0; i < GigAdapterInfo->rxqnum; i++) {
    struct intelgbe_rx_queue *rx_queue = &GigAdapterInfo->rx_queue[i];
    if (i < 4)
      reg_val = INTELGBE_READ_REG(hw, MTL_RXQ_DMA_MAP0);
    else
      reg_val = INTELGBE_READ_REG(hw, MTL_RXQ_DMA_MAP1);
    if ((i == 0) || (i == 4)) {
      reg_val |= rx_queue->chan << 0;
    } else {
      reg_val |= (rx_queue->chan << (8*i));
    }
    if (i < 4)
      INTELGBE_WRITE_REG(hw, MTL_RXQ_DMA_MAP0, reg_val);
    else
      INTELGBE_WRITE_REG(hw, MTL_RXQ_DMA_MAP1, reg_val);
  }
  txqsz = (mac->txfifosz / (GigAdapterInfo->txqnum * MTL_TXQSZ_BLOCK)) - 1;
  rxqsz = (mac->rxfifosz / (GigAdapterInfo->rxqnum * MTL_RXQSZ_BLOCK)) - 1;

  for (i = 0; i < GigAdapterInfo->txqnum; i++) {
    /* Enable TX store forward and configure TX queue size */
    reg_val = MTL_TXQ_OPR_TSF;
    reg_val |= ((txqsz << MTL_TXQ_OPR_TQS_SHIFT) &
                            MTL_TXQ_OPR_TQS_MASK);
    INTELGBE_WRITE_REG(hw, MTL_TXQ_OPERATION_MODE(i), reg_val);
  }
  for (i = 0; i < GigAdapterInfo->rxqnum; i++) {
    /* Enable RX store forward and configure RX queue size */
    reg_val = MTL_RXQ_OPR_RSF;
    reg_val |= ((rxqsz << MTL_RXQ_OPR_RQS_SHIFT) &
                            MTL_RXQ_OPR_RQS_MASK);
    INTELGBE_WRITE_REG(hw, MTL_RXQ_OPERATION_MODE(i), reg_val);
  }
  return 0;
}

static inline int intelgbe_dma_init(struct intelgbe_hw *hw)
{
  GIG_DRIVER_DATA *GigAdapterInfo = (GIG_DRIVER_DATA *)hw->back;
  u32 reg_val;
  int i;

  /* Enable Low Power Interface and DMA AXI Burst Length 4, 8 & 16 mode */
  reg_val = DMA_SYSBUS_MD_EN_LPI | DMA_SYSBUS_MD_BLEN16 |
            DMA_SYSBUS_MD_BLEN8 | DMA_SYSBUS_MD_BLEN4;
  INTELGBE_WRITE_REG(hw, DMA_SYSBUS_MODE, reg_val);

  for (i = 0; i < GigAdapterInfo->txqnum; i++) {
    struct intelgbe_tx_queue *tx_queue = &GigAdapterInfo->tx_queue[i];

    tx_queue->queue_index = i;
    tx_queue->tx_desc = (INTELGBE_TRANSMIT_DESCRIPTOR *)
                        (GigAdapterInfo->TxRing.UnmappedAddress +
                         i*sizeof(INTELGBE_TRANSMIT_DESCRIPTOR) *
                         DEFAULT_TX_DESCRIPTORS);

    tx_queue->dma_tx = (INTELGBE_TRANSMIT_DESCRIPTOR *)
                       (GigAdapterInfo->TxRing.PhysicalAddress +
                        i*sizeof(INTELGBE_TRANSMIT_DESCRIPTOR) *
                        DEFAULT_TX_DESCRIPTORS);

    /* TODO: descriptor address alignment */
    if (POINTER_TO_UINT(&tx_queue->tx_desc[0]) & 0x0F) {
      DEBUGPRINT (CRITICAL, ("TX descriptor address alignment error 0x%08X\n",
                              POINTER_TO_UINT(&tx_queue->tx_desc[0])));
    }
    /* Initialize TX descriptor ring length */
    INTELGBE_WRITE_REG(hw, DMA_TXDESC_RING_LENGTH_CH(i),
                          DEFAULT_TX_DESCRIPTORS - 1);

    /* Initialize TX descriptor ring list address */
    INTELGBE_WRITE_REG(hw, DMA_TXDESC_LIST_ADDR_CH(i),
                           (u32)POINTER_TO_UINT(&tx_queue->tx_desc[0]));
    DEBUGPRINT (CRITICAL, ("TX descriptor address VA: 0x%16llx PA: 0x%16llx\n",
                            POINTER_TO_UINT(&tx_queue->tx_desc[0]),
                            POINTER_TO_UINT(&tx_queue->dma_tx[0])));
    DEBUG ((DEBUG_ERROR, "TX descriptor address VA: 0x%16llx PA: 0x%16llx\n",
                            POINTER_TO_UINT(&tx_queue->tx_desc[0]),
                            POINTER_TO_UINT(&tx_queue->dma_tx[0])));

    /* Initialize TX descriptor ring tail pointer */
    INTELGBE_WRITE_REG(hw, DMA_TXDESC_TAIL_PTR_CH(i),
                          (u32)POINTER_TO_UINT(&tx_queue->tx_desc[0]));
    tx_queue->tx_tail_addr = (u32)POINTER_TO_UINT(&tx_queue->tx_desc[0]);
    /* Set TX PBL to 32x8 */
    reg_val = 32 << DMA_CH_TX_CTRL_TXPBL_SHIFT;
    reg_val &= DMA_CH_TX_CTRL_TXPBL_MASK;
    INTELGBE_WRITE_REG(hw, DMA_TX_CONTROL_CH(i), reg_val);

    memset((void *)tx_queue->tx_desc, 0, sizeof(INTELGBE_TRANSMIT_DESCRIPTOR)
                                        * DEFAULT_TX_DESCRIPTORS);
  }
  for (i = 0; i < GigAdapterInfo->rxqnum; i++) {
    struct intelgbe_rx_queue *rx_queue = &GigAdapterInfo->rx_queue[i];

    rx_queue->queue_index = i;
    rx_queue->rx_desc = (INTELGBE_RECEIVE_DESCRIPTOR *)
                        (GigAdapterInfo->RxRing.UnmappedAddress +
                         i*sizeof(INTELGBE_RECEIVE_DESCRIPTOR) *
                         DEFAULT_RX_DESCRIPTORS);

    rx_queue->dma_rx = (INTELGBE_RECEIVE_DESCRIPTOR *)
                       (GigAdapterInfo->RxRing.PhysicalAddress +
                        i*sizeof(INTELGBE_RECEIVE_DESCRIPTOR) *
                        DEFAULT_RX_DESCRIPTORS);
    rx_queue->rx_buff = (LOCAL_RX_BUFFER *)
                        (GigAdapterInfo->RxBufferMapping.UnmappedAddress +
                         i*sizeof(LOCAL_RX_BUFFER) * DEFAULT_RX_DESCRIPTORS);

    rx_queue->dma_rx_buff = (LOCAL_RX_BUFFER *)
                            (GigAdapterInfo->RxBufferMapping.PhysicalAddress +
                             i*sizeof(LOCAL_RX_BUFFER) *
                             DEFAULT_RX_DESCRIPTORS);

  /* TODO: descriptor address alignment */
    if (POINTER_TO_UINT(&rx_queue->dma_rx[0]) & 0x0F) {
      DEBUGPRINT (CRITICAL, ("RX descriptor address alignment error 0x%08X\n",
                            POINTER_TO_UINT(&rx_queue->dma_rx[0])));
    }
    intelgbe_dma_rx_desc_init(rx_queue);

    /* Initialize RX descriptor ring length */
    INTELGBE_WRITE_REG(hw, DMA_RXDESC_RING_LENGTH_CH(rx_queue->chan),
                          DEFAULT_RX_DESCRIPTORS - 1);

    /* Set RX PBL to 32x8 */
    reg_val = 32 << DMA_CH_RX_CTRL_RXPBL_SHIFT;
    reg_val |= ((1536 << DMA_CH_RX_CTRL_RBSZ_SHIFT));
    reg_val &= DMA_CH_RX_CTRL_RXPBL_MASK | DMA_CH_RX_CTRL_RBSZ_MASK;
    INTELGBE_WRITE_REG(hw, DMA_RX_CONTROL_CH(rx_queue->chan), reg_val);

    /* Initialize RX descriptor ring list address */
    INTELGBE_WRITE_REG(hw, DMA_RXDESC_LIST_ADDR_CH(rx_queue->chan),
                           (u32)POINTER_TO_UINT(&rx_queue->dma_rx[0]));
    DEBUGPRINT (CRITICAL, ("RX descriptor head address 0x%08llx\n",
                            POINTER_TO_UINT(&rx_queue->dma_rx[0])));

    /* Initialize RX descriptor ring tail pointer */
    rx_queue->rx_tail_addr = (u32)(u64)rx_queue->dma_rx +
    (sizeof(INTELGBE_RECEIVE_DESCRIPTOR) * DEFAULT_RX_DESCRIPTORS);
    DEBUGPRINT (CRITICAL, ("RX descriptor tail address 0x%08llx\n",
                            POINTER_TO_UINT(rx_queue->rx_tail_addr)));
    INTELGBE_WRITE_REG(hw, DMA_RXDESC_TAIL_PTR_CH(rx_queue->chan),
                                rx_queue->rx_tail_addr);
    /* Enable 8x Programmable Burst Length mode */
    reg_val = DMA_CH_CTRL_PBLX8;
    INTELGBE_WRITE_REG(hw, DMA_CONTROL_CH(rx_queue->chan), reg_val);
  }

  for (i = 0; i < GigAdapterInfo->txqnum; i++) {
    /* Enable 8x Programmable Burst Length mode */
    reg_val = DMA_CH_CTRL_PBLX8;
    INTELGBE_WRITE_REG(hw, DMA_CONTROL_CH(i), reg_val);
  }
  return 0;
}

s32 intelgbe_link_status (struct intelgbe_hw *hw, bool *link)
{
  s32 retval;
  u32 reg_val;
  struct intelgbe_mac_info *mac = &hw->mac;
  struct intelgbe_phy_info *phy = &hw->phy;
  bool link_sts_chg = false;
  s32 link_speed;
  s8 duplex;

  retval = phy->ops.link_status_change(hw, &link_sts_chg);
  if (retval < 0)
    return retval;
  if (!link_sts_chg) {
    *link = phy->link_up;
    return 0;
  }
  retval = phy->ops.status(hw, link, &link_speed, &duplex);
  if (retval < 0)
    return retval;
  if (link_sts_chg) {
    if (link) {
      mac->link_speed = link_speed;
      mac->full_duplex = duplex;
      DEBUGPRINT (CRITICAL, ("MAC configured for speed %dMbps ",
                              mac->link_speed));
      reg_val = INTELGBE_READ_REG(hw, MAC_CONFIGURATION);

      reg_val &= INV_MAC_CONF_SPD;
      switch (mac->link_speed) {
      case 100:
        reg_val |= MAC_CONF_SPD_100MHZ;
        break;
      case 1000:
        reg_val |= MAC_CONF_SPD_1000MHZ;
        break;
      case 2500:
        reg_val |= MAC_CONF_SPD_2500MHZ;
        break;
      default:
        reg_val |= MAC_CONF_SPD_10MHZ;
        break;
      }

      if (mac->full_duplex) {
        DEBUGPRINT (CRITICAL, ("full duplex\n"));
        reg_val |= MAC_CONF_DM;
      } else {
        DEBUGPRINT (CRITICAL, ("half duplex\n"));
        reg_val &= ~MAC_CONF_DM;
      }
      INTELGBE_WRITE_REG(hw, MAC_CONFIGURATION, reg_val);
      phy->link_up = true;
    } else {
      phy->link_up = false;
    }
  }
  return 0;
}

s32 intelgbe_uninit_controller(struct intelgbe_hw *hw)
{
  GIG_DRIVER_DATA *GigAdapterInfo = (GIG_DRIVER_DATA *)hw->back;
  u32 reg_val;
  int i;

  for (i = 0; i < GigAdapterInfo->rxqnum; i++) {
    struct intelgbe_rx_queue *rx_queue = &GigAdapterInfo->rx_queue[i];
    /* Enable DMA layer receive channels */
    reg_val = INTELGBE_READ_REG(hw, DMA_RX_CONTROL_CH(rx_queue->chan));
    reg_val &= ~DMA_CH_RX_CTRL_SR;
    INTELGBE_WRITE_REG(hw, DMA_RX_CONTROL_CH(rx_queue->chan), reg_val);
  }

  for (i = 0; i < GigAdapterInfo->txqnum; i++) {
    /* Enable MTL layer transmit queues */
    reg_val = INTELGBE_READ_REG(hw, MTL_TXQ_OPERATION_MODE(i));
    reg_val &= INV_MTL_TXQ_OPR_TXQEN;
    INTELGBE_WRITE_REG(hw, MTL_TXQ_OPERATION_MODE(i), reg_val);

    /* Enable DMA layer transmit channels */
    reg_val = INTELGBE_READ_REG(hw, DMA_TX_CONTROL_CH(i));
    reg_val &= ~DMA_CH_TX_CTRL_ST;
    INTELGBE_WRITE_REG(hw, DMA_TX_CONTROL_CH(i), reg_val);
  }
  /* Enable MAC layer receive & transmit */
  reg_val = INTELGBE_READ_REG(hw, MAC_CONFIGURATION);
  reg_val &= ~MAC_CONF_RE;
  reg_val &= ~MAC_CONF_TE;
  INTELGBE_WRITE_REG(hw, MAC_CONFIGURATION, reg_val);
  return 0;
}

s32 intelgbe_xpcs_init(struct intelgbe_hw *hw)
{
  int retval = 0;
  int xpcs_retry = 0;
  UINT16 phy_reg;
  UINT16 mask;

  /* Reset XPCS before initialize the registers */
  mask = SR_MII_CTRL_RST;
  phy_reg = SR_MII_CTRL_RST;
  retval |= intelgbe_mdio_c45_modify(hw, VENDOR_SPECIFIC_MII_MMD,
                                 SR_MII_MMD_CTRL_REG, mask, phy_reg);

  /* Check if the XPCS reset has completed */
  phy_reg = 0;
  xpcs_retry = 10;
  do {
    retval |= intelgbe_mdio_c45_read(hw, VENDOR_SPECIFIC_MII_MMD,
                                 SR_MII_MMD_CTRL_REG, &phy_reg);

    if (!(phy_reg & SR_MII_CTRL_RST)) {
      break;
    }

    msec_delay(1);
  } while (--xpcs_retry);

  /* Return error timedout if retry count reach zero */
  if (!xpcs_retry) {
    return -INTELGBE_ERR_TIMEOUT;
  }

  if (hw->mac.speed_2500_en) {
    /* Enable 2.5G mode and disable MAC auto speed change
     * See GPY datasheet 2.6
     */
    DEBUGPRINT(INTELGBE, ("XPCS: 2.5Gbps mode\n"));
    retval |= intelgbe_mdio_c45_modify(hw, VENDOR_SPECIFIC_MII_MMD,
                                       VR_MII_MMD_DIG_CTRL1_REG,
                                       VR_MII_DIG_CTRL1_MAC_AUTO_SW,
                                       (VR_MII_DIG_CTRL1_25G_EN| \
                                        VR_MII_DIG_CTRL1_PRE_EMP));
    /* Disable SGMII AN */
    mask = SR_MII_CTRL_AN_EN;
    phy_reg = SR_MII_CTRL_2500;
    retval |= intelgbe_mdio_c45_modify(hw, VENDOR_SPECIFIC_MII_MMD,
                                   SR_MII_MMD_CTRL_REG, mask, phy_reg);

  } else {
    DEBUGPRINT(INTELGBE, ("XPCS: 1Gbps mode\n"));
    /* Enable Pre-emption packet & auto speed mode change after AN */
    mask = VR_MII_DIG_CTRL1_MAC_AUTO_SW | VR_MII_DIG_CTRL1_PRE_EMP;
    phy_reg = VR_MII_DIG_CTRL1_MAC_AUTO_SW | VR_MII_DIG_CTRL1_PRE_EMP;
    retval |= intelgbe_mdio_c45_modify(hw, VENDOR_SPECIFIC_MII_MMD,
                                       VR_MII_MMD_DIG_CTRL1_REG, mask, phy_reg);

    /* Enable AN interrupt, SGMII PCS Mode & MAC side SGMII */
    mask = VR_MII_AN_CTRL_TX_CFG | VR_MII_AN_CTRL_PCS_MODE_MASK |
           VR_MII_AN_CTRL_AN_INTR_EN;
    phy_reg = VR_MII_AN_CTRL_TX_CFG_MAC_SIDE_SGMII |
              VR_MII_AN_CTRL_PCS_MODE_SGMII | VR_MII_AN_CTRL_AN_INTR_EN;
    retval |= intelgbe_mdio_c45_modify(hw, VENDOR_SPECIFIC_MII_MMD,
                                 VR_MII_MMD_AN_CTRL_REG, mask, phy_reg);

    /* Enable AN & restart AN */
    mask = SR_MII_CTRL_AN_EN | SR_MII_CTRL_RESTART_AN;
    phy_reg = SR_MII_CTRL_AN_EN | SR_MII_CTRL_RESTART_AN;
    retval |= intelgbe_mdio_c45_modify(hw, VENDOR_SPECIFIC_MII_MMD,
                                   SR_MII_MMD_CTRL_REG, mask, phy_reg);
  }

  return INTELGBE_SUCCESS;
}

s32 intelgbe_modphy_init(struct intelgbe_hw *hw)
{
  int retval = 0;
  u32 data_addr = 0;
  u16 data = 0;
  int retries = 10;
  u8 link_mode = 0;

  /* Determine link speed mode: 2.5Gbps or 1Gbps */
  retval = intelgbe_mdio_read(hw, MODPHY_ADDR, 0, SERDES_GCR, &data_addr, 0);
  if (retval < 0) {
    return retval;
  }
  link_mode = (data_addr & SERDES_LINK_MODE_MASK) >> SERDES_LINK_MODE_SHIFT;

  data_addr = 0;
  retval = intelgbe_mdio_read(hw, MODPHY_ADDR, 0, SERDES_GCR0, &data_addr, 0);
  if (retval < 0) {
    return retval;
  }

  data = (u16) data_addr;
  data &= ~SERDES_RATE_MASK;
  data &= ~SERDES_PCLK_MASK;

  if (link_mode == SERDES_LINK_MODE_2G5) {
    hw->mac.speed_2500_en = true;

    DEBUGPRINT(INTELGBE, ("SERDES: PCLK set to 37.5Mhz\n"));
    data |= SERDES_RATE_PCIE_GEN2 << SERDES_RATE_PCIE_SHIFT |
              SERDES_PCLK_37p5MHZ << SERDES_PCLK_SHIFT;
  } else {
    hw->mac.speed_2500_en = false;

    data |= SERDES_RATE_PCIE_GEN1 << SERDES_RATE_PCIE_SHIFT |
              SERDES_PCLK_70MHZ << SERDES_PCLK_SHIFT;
  }
  retval = intelgbe_mdio_write(hw, MODPHY_ADDR, 0, SERDES_GCR0, data, 0);
  if (retval < 0) {
    return retval;
  }

  /* assert clk_req */
  retval = intelgbe_mdio_read(hw, MODPHY_ADDR, 0, SERDES_GCR0, &data_addr, 0);
  if (retval < 0) {
    return retval;
  }

  data = (u16) data_addr;
  data |= SERDES_PLL_CLK;

  retval = intelgbe_mdio_write(hw, MODPHY_ADDR, 0, SERDES_GCR0, data, 0);
  if (retval < 0) {
    return retval;
  }
  retries = 10;
  do {
    retval = intelgbe_mdio_read(hw, MODPHY_ADDR, 0, SERDES_GSR0, &data_addr, 0);
    if (retval < 0) {
      return retval;
    }
    if ((data_addr & SERDES_PLL_CLK) == (SERDES_PLL_CLK & SERDES_PLL_CLK))
            break;
    msec_delay(1);
  } while (--retries);

  if (retries < 0) {
    DEBUGPRINT (INTELGBE, ("Serdes PLL clk request timeout %x\n", data_addr));
    return -1;
  }

  /* assert lane reset */
  retval = intelgbe_mdio_read(hw, MODPHY_ADDR, 0, SERDES_GCR0, &data_addr, 0);
  if (retval < 0) {
    return retval;
  }

  data = (u16) data_addr;
  data |= SERDES_RST;

  retval = intelgbe_mdio_write(hw, MODPHY_ADDR, 0, SERDES_GCR0, data, 0);
  if (retval < 0) {
          return retval;
  }

  retries = 10;
  do {
    retval = intelgbe_mdio_read(hw, MODPHY_ADDR, 0, SERDES_GSR0, &data_addr, 0);
    if (retval < 0) {
      return retval;
    }
    if ((data_addr & SERDES_RST) == (SERDES_RST & SERDES_RST))
       break;
    msec_delay(1);
  } while (--retries);

  if (retries < 0) {
    DEBUGPRINT(INTELGBE, ("Serdes assert lane reset timeout %x\n", data_addr));
    return -1;
  }

  /*  move power state to P0 */
  retval = intelgbe_mdio_read(hw, MODPHY_ADDR, 0, SERDES_GCR0, &data_addr, 0);
  if (retval < 0) {
    return retval;
  }

  data = (u16) data_addr;
  data &= ~SERDES_PWR_ST_MASK;
  data |= SERDES_PWR_ST_P0 << SERDES_PWR_ST_SHIFT;
  retval = intelgbe_mdio_write(hw, MODPHY_ADDR, 0, SERDES_GCR0, data, 0);
  if (retval < 0) {
    return retval;
  }

  retries = 10;
  do {
    retval = intelgbe_mdio_read(hw, MODPHY_ADDR, 0, SERDES_GSR0, &data_addr, 0);
    if (retval < 0) {
      return retval;
    }
    if ((data_addr & SERDES_PWR_ST_MASK) == ((SERDES_PWR_ST_P0 << SERDES_PWR_ST_SHIFT) & SERDES_PWR_ST_MASK))
      break;
    msec_delay(1);
  } while (--retries);

  if (retries < 0)
  {
    DEBUGPRINT(INTELGBE, ("Serdes power state P0 timeout %x\n", data_addr));
    return -1;
  }

  /* PSE specific SERDES power-up programming to ungate SGMII PHY Rx Clock
   * See https://elixir.bootlin.com/linux/v5.13.2/source/drivers/net/ethernet/stmicro/stmmac/dwmac-intel.c#L156
   */
  if (hw->mac.pse_gbe) {
    DEBUGPRINT(INTELGBE, ("PSE specific programming\n"));
    retval = intelgbe_mdio_read(hw, MODPHY_ADDR, 0, SERDES_GCR0, &data_addr, 0);
    if (retval < 0) {
      return retval;
    }
    data |= SERDES_PHY_RX_CLK;
    retval = intelgbe_mdio_write(hw, MODPHY_ADDR, 0, SERDES_GCR0, data, 0);
    if (retval < 0) {
      return retval;
    }
  }

  return INTELGBE_SUCCESS;
}

s32 intelgbe_init_controller(struct intelgbe_hw *hw)
{
  s32 retval;

  retval = intelgbe_dma_init(hw);
  if (retval < 0)
    return retval;
  retval = intelgbe_mtl_init(hw);
  if (retval < 0)
    return retval;
  retval = intelgbe_mac_init(hw);
  if (retval < 0)
    return retval;
  retval = intelgbe_mac_enable_interrupts(hw);
  if (retval < 0)
    return retval;
  retval = intelgbe_mac_start_transaction(hw);
  if (retval < 0)
    return retval;

  return INTELGBE_SUCCESS;
}

/**
 *  Intelgbe_init_mac_ops- Init MAC func ptrs.
 *  @hw: pointer to the HW structure
 **/
s32 intelgbe_init_mac_ops(struct intelgbe_hw *hw)
{
  struct intelgbe_mac_info *mac = &hw->mac;
  u32 tx_queues, rx_queues;

  DEBUGPRINT (INTELGBE, ("entered init mac ops funcs\n"));

  /* read mac address */
  mac->ops.read_mac_addr = intelgbe_read_mac_addr;
  /* write mac address */
  mac->ops.write_mac_addr = intelgbe_write_mac_addr;
  /* read ID reg */
  mac->ops.get_id = intelgbe_get_mac_version;
  /* reset MAC controller */
  mac->ops.reset_hw = intelgbe_reset_controller;
  /* initialize MAC controller */
  mac->ops.init_hw = intelgbe_init_controller;
  /* deactivate MAC controller */
  mac->ops.uninit_hw = intelgbe_uninit_controller;
  /* Link status change */
  mac->ops.check_for_link = intelgbe_link_status;
  /* Obtain HW TX & RX fifo size */
  mac->link_speed = 100;
  mac->full_duplex = 1;
  /*
    Refer EHL sighting report EHL-84 1507102816
    TXFIFOSIZE & RXFIFOSIZE Register Fields Incorrectly Report MTL TX & RX FIFO Sizes
    for PSE GbE Controllers

    Software must implement workaround below.

    A workaround for this sighting is available in the Beta Yocto Project* and
    PSE Beta firmware. Other operating systems should hardcode the MTL TX &
    RX FIFO size to 32kB, instead of reading from the
    MAC_HW_FEATURE1.TXFIFOSIZE & MAC_HW_FEATURE1.RXFIFOSIZE registers
  */

  switch (hw->device_id) {
    case EHL_PSE0_STMMAC_RGMII1G_DID:
    case EHL_PSE1_STMMAC_RGMII1G_DID:
    case EHL_PCH_STMMAC_SGMII_DID:
    case EHL_PSE0_STMMAC_SGMII1G_DID:
    case EHL_PSE1_STMMAC_SGMII1G_DID:
    case EHL_PSE0_STMMAC_SGMII2G5_DID:
    case EHL_PSE1_STMMAC_SGMII2G5_DID:
      tx_queues = 8;
      rx_queues = 8;
      break;
    case TGLH_PCH1_STMMAC_SGMII_DID:
    case TGLH_PCH2_STMMAC_SGMII_DID:
    case TGLU_PCH1_STMMAC_SGMII_DID:
      /*
          TGL FIFO size with (4 TxQ and 6RxQ):
          RXFIFOSIZE = 24KB
          TXFIFOSIZE = 16KB
      */
      tx_queues = 4;
      rx_queues = 6;
      break;
    default:
      /* Should never have loaded on this device */
      DEBUGPRINT(CRITICAL, ("Unknown device id: %X\n", hw->device_id));
      return -INTELGBE_ERR_MAC_INIT;
  }
  /*
    There is an errata on this RX/TX FIFO size. 
    So, we will not follow the size read from the MAC_HW_FEATURE1 register. 
    It is 4KB per queue. 
  */
  mac->txfifosz = tx_queues * INTELGBE_FIFO_SZ_PER_QUEUE;
  mac->rxfifosz = rx_queues * INTELGBE_FIFO_SZ_PER_QUEUE;

  return INTELGBE_SUCCESS;
}

/**
 *  intelgbe_init_phy_params_stmmac - Init PHY func ptrs.
 *  ported from e1000_init_phy_params_82571
 *  @hw: pointer to the HW structure
 **/
s32 intelgbe_init_phy_params_stmmac(struct intelgbe_hw *hw)
{
  struct intelgbe_phy_info *phy = &hw->phy;
  u32 vendor, model;
  s32 ret_val;

  DEBUGPRINT (INTELGBE, ("intelgbe_init_phy_ops_generic\n"));

  phy->mdio_csr_clk = MAC_MDIO_CSR_CLOCK_250_300MHZ;
  /* PHY write */
  phy->ops.mdio_write = intelgbe_mdio_write;
  /* PHY read */
  phy->ops.mdio_read = intelgbe_mdio_read;

  switch (hw->mac.type) {
    case intelgbe_stmmac_rgmii:
      phy->interface = PHY_INTERFACE_RGMII;
      break;
    case intelgbe_stmmac_sgmii:
      phy->interface = PHY_INTERFACE_SGMII;
      break;
    default:
      DEBUGPRINT (CRITICAL, ("Unknown MAC.\n"));
      return -INTELGBE_ERR_PHY;
      break;
}

  /* Get PHY ID */
  ret_val = mii_phy_id_get(hw);
  if (ret_val < 0) {
    return -INTELGBE_ERR_PHY;
  }

  /* PHY func ptrs are set by PHY specific "init func pointers" function.
   * At the time of writing, the "init func pointers" to be called is select based on:
   * - vendor ID/OUI. e.g all PHYs from GPY has a single function only. Please refer to
       "include/linux/intel_phy.h" from Intel Yocto repo.
   * - model/device ID. e.g. Marvell has multiple models of PHYs and each models
       will have their own "init func pointers".
   * To cater this different criterias, we have nested switches where we will
   * select vendor-->select model(if needed)-->select revision(if needed)-->
   * check phy specific rules(if needed)-->call init function.
   * Vendor ID(Bits 31:10), Device ID(Bits 9:4) and Device Rev(Bits 3:0) makes
   * up the 32-bits PHY ID
   */
  vendor = MII_PHY_VENDOR(phy->id);
  model = MII_PHY_MODEL(phy->id);

  switch (vendor) {

    /* PHYs from Marvell. */
    case MARVELL_PHY_VENDOR:

      /* Marvell has multiple models. Select the correct model*/
      switch (model) {

        /* Marvel PHY family 88E1510*/
        case MARVELL_PHY_MODEL_88E1510:
          intelgbe_init_phy_ops_marvell_88e1512(hw);
          break;

        default:
          DEBUGPRINT (CRITICAL, ("This Marvell PHY family is not supported by driver.\n"));
          return -INTELGBE_ERR_PHY;
          break;
      }
      break;

    /* PHYs from Maxlinear uses the same function. */
    case MAXLINEAR_PHY_VENDOR:
      if (phy->interface == PHY_INTERFACE_SGMII){
        intelgbe_init_phy_ops_maxlinear_gpyxxx(hw);
      } else {
        DEBUGPRINT (CRITICAL, ("GPY PHY only supports SGMII interface.\n"));
        return -INTELGBE_ERR_PHY;
      }
      break;
    
    case MARVELL10G_PHY_VENDOR:
      if (phy->interface == PHY_INTERFACE_SGMII){
        intelgbe_init_phy_ops_marvell_88e2110(hw);
      } else {
        DEBUGPRINT (CRITICAL, ("M88E2110 only supports SGMII interface.\n"));
        return -INTELGBE_ERR_PHY;
      }
      break;


    default:
      DEBUGPRINT (CRITICAL, ("PHY is not supported by driver.\n"));
      return -INTELGBE_ERR_PHY;
      break;
  }

  return INTELGBE_SUCCESS;
}

/**
 *  intelgbe_init_function_pointers_stmmac - Init func ptrs.
 *  ported from e1000_init_function_pointers_82571
 *  @hw: pointer to the HW structure
 *
 *  Called to initialize all function pointers and parameters.
 *
 *  TODO: Do we need something similar to e1000_init_nvm_params_82571?
 **/
void intelgbe_init_function_pointers_stmmac(struct intelgbe_hw *hw)
{
  DEBUGFUNC("intelgbe_init_function_pointers_stmmac");

  hw->mac.ops.init_params = intelgbe_init_mac_ops;
  hw->phy.ops.init_params = intelgbe_init_phy_params_stmmac;
}
