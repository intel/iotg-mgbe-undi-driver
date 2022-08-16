/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _INTELGBE_DEFINES_H_
#define _INTELGBE_DEFINES_H_

#ifndef ETH_ADDR_LEN
#define ETH_ADDR_LEN                            6
#endif

#define BIT(n)                                  (1UL << (n))
#define BIT_MASK(n)                             (BIT(n) - 1)

/* Helper to pass a int as a pointer or vice-versa.
 * Those are available for 32 bits architectures:
 */
#define POINTER_TO_UINT(x)                      ((u64) (x))
#define UINT_TO_POINTER(x)                      ((void *) (x))
#define POINTER_TO_INT(x)                       ((s64) (x))
#define INT_TO_POINTER(x)                       ((void *) (x))

#define INTELGBE_MAX_RX_QUEUES                    1
#define INTELGBE_MAX_TX_QUEUES                    1
#define INTELGBE_ADDR_HIGH(reg)                   (0x300 + reg * 8)
#define INTELGBE_ADDR_LOW(reg)                    (0x304 + reg * 8)

#define INTELGBE_ADDR_LOW_LEN                     4
#define INTELGBE_ADDR_HIGH_LEN                    2

#define INTELGBE_VERSION                          0x0110
#define INTELGBE_VERSION_MASK                     0x00ff

#define INTELGBE_FIFO_SZ_PER_QUEUE                0x1000

/*
 *  * DMA Registers
 *   */
#define INTELGBE_DMA_MODE                         0x1000
#define INTELGBE_INV_DMA_MD_INTM                  0xFFFCFFFF
#define INTELGBE_DMA_MD_INTM_MODE1                BIT(16)
#define INTELGBE_DMA_MD_INTM_MODE2                BIT(17)
#define INTELGBE_INV_DMA_MD_TAA                   0xFFFFFFE3
#define INTELGBE_DMA_MD_TAA_FP                    0x00000000
#define INTELGBE_DMA_MD_TAA_WSP                   0x00000004
#define INTELGBE_DMA_MD_TAA_WRP                   0x00000008
#define INTELGBE_DMA_MD_SWR                       BIT(0)

#define DMA_SYSBUS_MODE                         0x1004
#define DMA_SYSBUS_MD_EN_LPI                    BIT(31)
#define INV_DMA_SYSBUS_MD_WR_OSR_LMT            0xF8FFFFFF
#define INV_DMA_SYSBUS_MD_RD_OSR_LMT            0xFFF8FFFF
#define DMA_SYSBUS_MD_MB                        BIT(14)
#define DMA_SYSBUS_MD_AAL                       BIT(12)
#define DMA_SYSBUS_MD_AALE                      BIT(10)
#define DMA_SYSBUS_MD_BLEN32                    BIT(4)
#define DMA_SYSBUS_MD_BLEN16                    BIT(3)
#define DMA_SYSBUS_MD_BLEN8                     BIT(2)
#define DMA_SYSBUS_MD_BLEN4                     BIT(1)
#define DMA_SYSBUS_MD_FB                        BIT(0)

#define DMA_INTERRUPT_STATUS                    0x1008
#define DMA_INTR_STS_CHNL_MAX                   8
#define DMA_INTR_STS_DCIS(x)                    (BIT(0) << x)

#define DMA_CONTROL_CH(x)                       (0x1100 + (x * 0x80))
#define DMA_CH_CTRL_DSL_MASK                    0x001C0000
#define DMA_CH_CTRL_DSL_SHIFT                   18
#define DMA_CH_CTRL_PBLX8                       BIT(16)
#define DMA_CH_CTRL_MSS_MASK                    0x00003FFF

#define DMA_TX_CONTROL_CH(x)                    (0x1104 + (x * 0x80))
#define DMA_CH_TX_CTRL_TXPBL_MASK               0x003F0000
#define DMA_CH_TX_CTRL_TXPBL_SHIFT              16
#define DMA_CH_TX_CTRL_OSF                      BIT(4)
#define DMA_CH_TX_CTRL_ST                       BIT(0)

#define DMA_RX_CONTROL_CH(x)                    (0x1108 + (x * 0x80))
#define DMA_CH_RX_CTRL_RXPBL_MASK               0x003F0000
#define DMA_CH_RX_CTRL_RXPBL_SHIFT              16
#define DMA_CH_RX_CTRL_RBSZ_MASK                0x00007FFE
#define DMA_CH_RX_CTRL_RBSZ_SHIFT               1
#define DMA_CH_RX_CTRL_SR                       BIT(0)

#define DMA_TXDESC_LIST_ADDR_CH(x)              (0x1114 + (x * 0x80))
#define DMA_RXDESC_LIST_ADDR_CH(x)              (0x111C + (x * 0x80))
#define DMA_TXDESC_TAIL_PTR_CH(x)               (0x1120 + (x * 0x80))
#define DMA_RXDESC_TAIL_PTR_CH(x)               (0x1128 + (x * 0x80))
#define DMA_TXDESC_RING_LENGTH_CH(x)            (0x112C + (x * 0x80))
#define DMA_RXDESC_RING_LENGTH_CH(x)            (0x1130 + (x * 0x80))

#define DMA_INTR_EN_CH(x)                       (0x1134 + (x * 0x80))
#define DMA_CH_INTR_EN_NIE                      BIT(15)
#define DMA_CH_INTR_EN_RWTE                     BIT(9)
#define DMA_CH_INTR_EN_RIE                      BIT(6)
#define DMA_CH_INTR_EN_TIE                      BIT(0)
#define DMA_INTR_STATUS_CH(x)                   (0x1160 + (x * 0x80))
#define DMA_CH_INTR_STS_NIS                     BIT(15)
#define DMA_CH_INTR_STS_RWT                     BIT(9)
#define DMA_CH_INTR_STS_RI                      BIT(6)
#define DMA_CH_INTR_STS_TI                      BIT(0)

#define DMA_RX_INTR_WDT_CH(x)                   (0x1138 + (x * 0x80))
#define DMA_CH_RX_INTR_WDT_RWTU_MASK            0x00030000
#define DMA_CH_RX_INTR_WDT_RWTU_SHIFT           16
#define DMA_CH_RX_INTR_WDT_RWTU(x)              (256 * POW2(x))
#define DMA_CH_RX_INTR_WDT_RWT_MASK             0x000000FF

/*
 * MTL Registers
 */
#define MTL_OPERATION_MODE                      0x0C00
#define MTL_OPR_MD_FRPE                         BIT(15)
#define INV_MTL_OPR_MD_SCHALG                   0xFFFFFF9F
#define MTL_OPR_MD_SCHALG_WRR                   0x00000000
#define MTL_OPR_MD_SCHALG_WFQ                   0x00000020
#define MTL_OPR_MD_SCHALG_DWRR                  0x00000040
#define MTL_OPR_MD_SCHALG_SP                    0x00000060
#define INV_MTL_OPR_MD_RAA_SP                   0xFFFFFFFB
#define MTL_OPR_MD_RAA_WSP                      BIT(2)

#define MTL_INTERRUPT_STATUS                    0x0C20
#define MTL_INTR_STS_Q7IS                       BIT(7)
#define MTL_INTR_STS_Q6IS                       BIT(6)
#define MTL_INTR_STS_Q5IS                       BIT(5)
#define MTL_INTR_STS_Q4IS                       BIT(4)
#define MTL_INTR_STS_Q3IS                       BIT(3)
#define MTL_INTR_STS_Q2IS                       BIT(2)
#define MTL_INTR_STS_Q1IS                       BIT(1)
#define MTL_INTR_STS_Q0IS                       BIT(0)
#define MTL_INTR_CTRL_STATUS_Q(x)               (0x0D2C + (x * 0x40))
#define MTL_QX_INTR_CTRL_STS_RXOIE              BIT(24)
#define MTL_QX_INTR_CTRL_STS_RXOVFIS            BIT(16)

#define MTL_RXQ_DMA_MAP0                        0x0C30
#define MTL_RXQ_DMA_MAP1                        0x0C34
#define INV_MTL_RXQ_DMA_Q0_Q4MDMACH             0xFFFFFFF8
#define INV_MTL_RXQ_DMA_Q1_Q5MDMACH             0xFFFFF8FF
#define INV_MTL_RXQ_DMA_Q2_Q6MDMACH             0xFFF8FFFF
#define INV_MTL_RXQ_DMA_Q3_Q7MDMACH             0xF8FFFFFF
#define MTL_RXQ_DMA_Q0_Q4MDMACH_0               0x00000000
#define MTL_RXQ_DMA_Q0_Q4MDMACH_1               0x00000001
#define MTL_RXQ_DMA_Q0_Q4MDMACH_2               0x00000002
#define MTL_RXQ_DMA_Q0_Q4MDMACH_3               0x00000003
#define MTL_RXQ_DMA_Q0_Q4MDMACH_4               0x00000004
#define MTL_RXQ_DMA_Q0_Q4MDMACH_5               0x00000005
#define MTL_RXQ_DMA_Q0_Q4MDMACH_6               0x00000006
#define MTL_RXQ_DMA_Q0_Q4MDMACH_7               0x00000007
#define MTL_RXQ_DMA_Q1_Q5MDMACH_0               0x00000000
#define MTL_RXQ_DMA_Q1_Q5MDMACH_1               0x00000100
#define MTL_RXQ_DMA_Q1_Q5MDMACH_2               0x00000200
#define MTL_RXQ_DMA_Q1_Q5MDMACH_3               0x00000300
#define MTL_RXQ_DMA_Q1_Q5MDMACH_4               0x00000400
#define MTL_RXQ_DMA_Q1_Q5MDMACH_5               0x00000500
#define MTL_RXQ_DMA_Q1_Q5MDMACH_6               0x00000600
#define MTL_RXQ_DMA_Q1_Q5MDMACH_7               0x00000700
#define MTL_RXQ_DMA_Q2_Q6MDMACH_0               0x00000000
#define MTL_RXQ_DMA_Q2_Q6MDMACH_1               0x00010000
#define MTL_RXQ_DMA_Q2_Q6MDMACH_2               0x00020000
#define MTL_RXQ_DMA_Q2_Q6MDMACH_3               0x00030000
#define MTL_RXQ_DMA_Q2_Q6MDMACH_4               0x00040000
#define MTL_RXQ_DMA_Q2_Q6MDMACH_5               0x00050000
#define MTL_RXQ_DMA_Q2_Q6MDMACH_6               0x00060000
#define MTL_RXQ_DMA_Q2_Q6MDMACH_7               0x00070000
#define MTL_RXQ_DMA_Q3_Q7MDMACH_0               0x00000000
#define MTL_RXQ_DMA_Q3_Q7MDMACH_1               0x01000000
#define MTL_RXQ_DMA_Q3_Q7MDMACH_2               0x02000000
#define MTL_RXQ_DMA_Q3_Q7MDMACH_3               0x03000000
#define MTL_RXQ_DMA_Q3_Q7MDMACH_4               0x04000000
#define MTL_RXQ_DMA_Q3_Q7MDMACH_5               0x05000000
#define MTL_RXQ_DMA_Q3_Q7MDMACH_6               0x06000000
#define MTL_RXQ_DMA_Q3_Q7MDMACH_7               0x07000000

#define MTL_RXP_CTRL_STATS                      0x0CA0
#define MTL_RXP_CTRL_STATS_RXPI                 BIT(31)
#define MTL_RXP_INDRT_ACC_CTRL_STATS            0x0CB0
#define MTL_RXP_INDACC_CTRLSTS_STARTBUSY        BIT(31)
#define MTL_RXP_INDACC_CTRLSTS_WRRDN            BIT(16)
#define MTL_RXP_INDACC_CTRLSTS_ADDR_MASK        0x000003FF
#define MTL_RXP_INDRT_ACC_DATA                  0x0CB4
#define MTL_RXP_INSTR_ACCPT                     BIT(0)
#define MTL_RXP_INSTR_REJT                      BIT(1)
#define MTL_RXP_INSTR_NEXT                      BIT(3)
#define MTL_RXP_INSTR_FRM_OFF_UNIT              4
#define MTL_RXP_DWORD_PER_INSTR                 4

#define MTL_TXQ_OPERATION_MODE(x)               (0x0D00 + (x * 0x40))
#define MTL_TXQ_OPR_TQS_MASK                    0x007F0000
#define MTL_TXQ_OPR_TQS_SHIFT                   16
#define MTL_TXQSZ_BLOCK                         256
#define MTL_TXQ_OPR_TSF                         BIT(1)
#define INV_MTL_TXQ_OPR_TXQEN                   0xFFFFFFF3
#define MTL_TXQ_OPR_TXQEN_EN                    0x00000008
#define MTL_TXQ_OPR_TXQEN_EN_IF_AV              0x00000004

#define MTL_RXQ_OPERATION_MODE(x)               (0x0D30 + (x * 0x40))
#define MTL_RXQ_OPR_RQS_MASK                    0x07F00000
#define MTL_RXQ_OPR_RQS_SHIFT                   20
#define MTL_RXQSZ_BLOCK                         256
#define MTL_RXQ_OPR_RSF                         BIT(5)


#define MAC_MDIO_ADDRESS_REG                    0x0200
#define MAC_MDIO_PA_MASK                        0x03E00000
#define MAC_MDIO_PA_SHIFT                       21
#define MAC_MDIO_RDA_MASK                       0x001F0000
#define MAC_MDIO_RDA_SHIFT                      16
#define MAC_MDIO_DATA_REG                       0x0204
#define MAC_MDIO_RA_MASK                        0xFFFF0000
#define MAC_MDIO_RA_SHIFT                       16
#define MAC_MDIO_GMII_BUSY                      BIT(0)
#define MAC_MDIO_CLAUSE_45_PHY_EN               BIT(1)
#define MAC_MDIO_GMII_OPR_CMD_READ              0x0000000C
#define MAC_MDIO_GMII_OPR_CMD_WRITE             0x00000004
#define INV_MAC_MDIO_CSR_CLOCK                  0xFFFFF0FF
#define MAC_MDIO_CSR_CLOCK_60_100MHZ            0x00000000
#define MAC_MDIO_CSR_CLOCK_100_150MHZ           0x00000100
#define MAC_MDIO_CSR_CLOCK_20_35MHZ             0x00000200
#define MAC_MDIO_CSR_CLOCK_35_60MHZ             0x00000300
#define MAC_MDIO_CSR_CLOCK_150_250MHZ           0x00000400
#define MAC_MDIO_CSR_CLOCK_250_300MHZ           0x00000500
#define MAC_MDIO_CSR_CLOCK_300_500MHZ           0x00000600
#define MAC_MDIO_CSR_CLOCK_500_800MHZ           0x00000700


#define XPCS_ADDR                               0x16
#define SR_MII_CTRL_RST                         BIT(15)

#define VENDOR_SPECIFIC_MII_MMD                 0x1F

#define SR_MII_MMD_CTRL_REG                     0x0000
#define SR_MII_CTRL_RESTART_AN                  BIT(9)
#define SR_MII_CTRL_AN_EN                       BIT(12)
#define SR_MII_CTRL_2500                        (BIT(13) & ~BIT(6))

#define VR_MII_MMD_DIG_CTRL1_REG                0x8000
#define VR_MII_DIG_CTRL1_25G_EN                 BIT(2)
#define VR_MII_DIG_CTRL1_PRE_EMP                BIT(6)
#define VR_MII_DIG_CTRL1_MAC_AUTO_SW            BIT(9)

#define VR_MII_MMD_AN_CTRL_REG                  0x8001
#define VR_MII_AN_CTRL_TX_CFG                   BIT(3)
#define VR_MII_AN_CTRL_PCS_MODE_MASK            0x00000006
#define VR_MII_AN_CTRL_AN_INTR_EN               BIT(0)
#define VR_MII_AN_CTRL_TX_CFG_MAC_SIDE_SGMII    0x00000000
#define VR_MII_AN_CTRL_PCS_MODE_SGMII           0x00000004
#define SR_MII_CTRL_AN_EN                       BIT(12)
#define SR_MII_CTRL_RESTART_AN                  BIT(9)

#define MODPHY_ADDR                             0x15

#define GENMASK(h, l) (((~0) << (l)) & (~0 >> (4 * 8 - 1 - (h))))
#define POLL_DELAY_US 8

/* SERDES Register */
#define SERDES_GCR      0x0     /* Global Conguration */
#define SERDES_GSR0     0x5     /* Global Status Reg0 */
#define SERDES_GCR0     0xB     /* Global Configuration Reg0 */

/* SERDES defines */
#define SERDES_PLL_CLK          BIT(0)          /* PLL clk valid signal */
#define SERDES_PHY_RX_CLK       BIT(1)          /* Rx clock ungate enable */
#define SERDES_RST              BIT(2)          /* Serdes Reset */
#define SERDES_PWR_ST_MASK      0x70            //GENMASK(6, 4)   /* Serdes Power state*/
#define SERDES_RATE_MASK        0x300           //GENMASK(9, 8)
#define SERDES_PCLK_MASK        0x7000          //GENMASK(14, 12) /* PCLK rate to PHY */
#define SERDES_LINK_MODE_MASK   0x6             //GENMASK(2, 1)
#define SERDES_LINK_MODE_SHIFT  1
#define SERDES_PWR_ST_SHIFT     4
#define SERDES_PWR_ST_P0        0x0
#define SERDES_PWR_ST_P3        0x3
#define SERDES_LINK_MODE_2G5    0x3
#define SERDES_LINK_MODE_1G     0x2
#define SERDES_PCLK_37p5MHZ     0x0
#define SERDES_PCLK_70MHZ       0x1
#define SERDES_RATE_PCIE_GEN1   0x0
#define SERDES_RATE_PCIE_GEN2   0x1
#define SERDES_RATE_PCIE_SHIFT  8
#define SERDES_PCLK_SHIFT       12

#define MAC_HW_FEATURE0                         0x011C
#define MAC_HW_FEAT0_TSSEL                      BIT(12)
#define MAC_HW_FEAT0_TXCOESEL                   BIT(14)
#define MAC_HW_FEAT0_RXCOESEL                   BIT(16)
#define MAC_HW_FEAT0_ADDMACADRSEL_MASK          0x007C0000
#define MAC_HW_FEAT0_ADDMACADRSEL_SHIFT         18
#define MAC_HW_FEATURE1                         0x0120
#define MAC_HW_FEAT1_HASHTBLSZ_MASK             0x03000000
#define MAC_HW_FEAT1_HASHTBLSZ_SHIFT            24
#define MAC_HW_FEAT1_HASHTBLSZ_64               0x01
#define MAC_HW_FEAT1_HASHTBLSZ_128              0x02
#define MAC_HW_FEAT1_HASHTBLSZ_256              0x03
#define MAC_HW_FEAT1_TXFIFOSZ_MASK              0x000007C0
#define MAC_HW_FEAT1_TXFIFOSZ_SHIFT             6
#define MAC_HW_FEAT1_RXFIFOSZ_MASK              0x0000001F
#define MAC_HW_FEATURE2                         0x0124
#define MAC_HW_FEAT2_TXCHCNT_MASK               0x003C0000
#define MAC_HW_FEAT2_TXCHCNT_SHIFT              18
#define MAC_HW_FEAT2_RXCHCNT_MASK               0x0000F000
#define MAC_HW_FEAT2_RXCHCNT_SHIFT              12
#define MAC_HW_FEAT2_TXQCNT_MASK                0x000003C0
#define MAC_HW_FEAT2_TXQCNT_SHIFT               6
#define MAC_HW_FEAT2_RXQCNT_MASK                0x0000000F
#define MAC_HW_FEATURE3                         0x0128
#define MAC_HW_FEAT3_FRPES_MASK                 0x00006000
#define MAC_HW_FEAT3_FRPES_SHIFT                13
#define MAC_HW_FEAT3_FRPES_64ENTR               0
#define MAC_HW_FEAT3_FRPES_128ENTR              1
#define MAC_HW_FEAT3_FRPES_256ENTR              2
#define MAC_HW_FEAT3_FRPBS_MASK                 0x00001800
#define MAC_HW_FEAT3_FRPBS_SHIFT                11
#define MAC_HW_FEAT3_FRPBS_64BYTS               0
#define MAC_HW_FEAT3_FRPBS_128BYTS              1
#define MAC_HW_FEAT3_FRPBS_256BYTS              2
#define MAC_HW_FEAT3_FRPSEL                     BIT(10)
#define MAC_HW_FEAT3_NRVF_MASK                  0x00000007
#define MAC_HW_FEAT3_NRVF_4                     0x1
#define MAC_HW_FEAT3_NRVF_8                     0x2
#define MAC_HW_FEAT3_NRVF_16                    0x3
#define MAC_HW_FEAT3_NRVF_24                    0x4
#define MAC_HW_FEAT3_NRVF_32                    0x5

/*
 * MAC Registers
 */

#define MAC_CONFIGURATION                       0x0000
#define MAC_PACKET_FILTER                       0x0008
#define MAC_CONF_IPC                            BIT(27)
#define MAC_CONF_CST                            BIT(21)
#define MAC_CONF_ACS                            BIT(20)
#define INV_MAC_CONF_SPD                        0xFFFF3FFF
#define MAC_CONF_SPD_10MHZ                      0x00008000
#define MAC_CONF_SPD_100MHZ                     0x0000C000
#define MAC_CONF_SPD_1000MHZ                    0x00000000
#define MAC_CONF_SPD_2500MHZ                    0x00004000
#define MAC_CONF_DM                             BIT(13)
#define MAC_CONF_LM                             BIT(12)
#define MAC_CONF_DO                             BIT(10)
#define MAC_CONF_TE                             BIT(1)
#define MAC_CONF_RE                             BIT(0)

#define MAC_RXQ_CTRL0                           0x00A0
#define MAC_RXQ_CTRL2                           0x00A8
#define MAC_RXQ_CTRL3                           0x00AC
#define INV_MAC_DISABLE_RXQ(x)                  ~((BIT(0) | BIT(1)) << (x * 2))
#define MAC_EN_AV_RXQ(x)                        (BIT(0) << (x * 2))
#define MAC_EN_DCB_GEN_RXQ(x)                   (BIT(1) << (x * 2))

/* Error Codes */
#define INTELGBE_BUSY                             100
#define INTELGBE_SUCCESS                          0
#define INTELGBE_ERR_NVM                          1
#define INTELGBE_ERR_PHY                          2
#define INTELGBE_ERR_CONFIG                       3
#define INTELGBE_ERR_PARAM                        4
#define INTELGBE_ERR_MAC_INIT                     5
#define INTELGBE_ERR_PHY_TYPE                     6
#define INTELGBE_ERR_RESET                        9
#define INTELGBE_ERR_MASTER_REQUESTS_PENDING      10
#define INTELGBE_ERR_HOST_INTERFACE_COMMAND       11
#define INTELGBE_BLK_PHY_RESET                    12
#define INTELGBE_ERR_SWFW_SYNC                    13
#define INTELGBE_NOT_IMPLEMENTED                  14
#define INTELGBE_ERR_MBX                          15
#define INTELGBE_ERR_INVALID_ARGUMENT             16
#define INTELGBE_ERR_NO_SPACE                     17
#define INTELGBE_ERR_NVM_PBA_SECTION              18
#define INTELGBE_ERR_TIMEOUT                      19

/* Receive Descriptor bit definitions */
#define INTELGBE_RXD_STAT_DD                      0x01    /* Descriptor Done */
#define INTELGBE_RXD_STAT_EOP                     0x02    /* End of Packet */

#define INTELGBE_RCFIL_ALLMCAST                   (1 << 4)

#endif /* _INTELGBE_DEFINES_H_ */
