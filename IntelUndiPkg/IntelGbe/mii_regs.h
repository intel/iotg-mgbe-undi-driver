/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _MII_REGS_H_
#define _MII_REGS_H_

/* Standard management registers supported by IEEE 802.3 Clause 22 */
#define MII_STD_CTRL      0x00    /* Basic control register */
#define MII_STD_STATUS    0x01    /* Basic status register */
#define MII_STD_PHYID1    0x02    /* PHY ID 1 */
#define MII_STD_PHYID2    0x03    /* PHY ID 2 */
#define MII_AN_ADV        0x04    /* Auto-Negotiation advertisement */
#define MII_AN_LPA        0x05    /* Auto-Negotiation link partner ability register */
#define MII_AN_EXP        0x06    /* Auto-Negotiation Expansion register */
#define MII_AN_NPTX       0x07    /* Auto-Negotiation next page transit register */
#define MII_AN_NPRX       0x08    /* Auto-Negotiation link partner received next page */
#define MII_STD_GCTRL     0x09    /* 1000BASE-T control register */
#define MII_STD_GSTAT     0x0a    /* 1000BASE-T status register */
#define MII_STD_MMD_CTRL  0x0d    /* MMD Access Control register */
#define MII_STD_MMD_DATA  0x0e    /* MMD Access Data register */
#define MII_STD_XSTATUS   0x0f    /* Extended status register */

/* Registers 16 through 31 are Vendor Specific */


/* Bit defines */
#define MII_STD_CTRL_RESET                BIT(15)
#define MII_STD_CTRL_LOOPBACK             BIT(14)
#define MII_STD_CTRL_AUTONEG_ENABLE       BIT(12)
#define MII_STD_CTRL_POWERDOWN            BIT(11)
#define MII_STD_CTRL_ISOLATE              BIT(10)
#define MII_STD_CTRL_AUTONEG_RESTART      BIT(9)
#define MII_STD_CTRL_DUPLEX_MODE          BIT(8)
#define MII_STD_CTRL_SPEED_MASK           (BIT(6) | BIT(13))
#define MII_STD_CTRL_SPEED_10             (0 << 6 | 0 << 13)
#define MII_STD_CTRL_SPEED_100            (0 << 6 | BIT(13))
#define MII_STD_CTRL_SPEED_1000           (BIT(6) | 0 << 13)

#define MII_STD_STATUS_100BASE_X_FULL     BIT(14)
#define MII_STD_STATUS_100BASE_X_HALF     BIT(13)
#define MII_STD_STATUS_10_FULL            BIT(12)
#define MII_STD_STATUS_10_HALF            BIT(11)
#define MII_STD_STATUS_EXTEND_STATUS      BIT(8)
#define MII_STD_STATUS_AUTONEG_COMPLETE   BIT(5)
#define MII_STD_STATUS_LINK_STATUS        BIT(2)

/* Bit ops for combined PHYID1 and PHYID2 Regs to get OUI */
#define MII_PHY_VENDOR(i)                (((i) & GENMASK(31, 10)) >> 10)
/* Bit ops for combined PHYID1 and PHYID2 Regs to get device number */
#define MII_PHY_MODEL(i)                 (((i) & GENMASK(9, 4)) >> 4)
/* Bit ops for combined PHYID1 and PHYID2 Regs to get device revision */
#define MII_PHY_REV(i)                   (((i) & GENMASK(3, 0)) >> 0)

#define MII_AN_ADV_TAF_ASYM_PAUSE         BIT(11)
#define MII_AN_ADV_TAF_PAUSE              BIT(10)
#define MII_AN_ADV_TAF_100BASE_T4         BIT(9)
#define MII_AN_ADV_TAF_100_FULL           BIT(8)
#define MII_AN_ADV_TAF_100_HALF           BIT(7)
#define MII_AN_ADV_TAF_10_FULL            BIT(6)
#define MII_AN_ADV_TAF_10_HALF            BIT(5)
#define MII_AN_ADV_TAF_SEL_IEEE_802_3     BIT(0)

#define MII_STD_GCTRL_1000_FULL           BIT(9)
#define MII_STD_GCTRL_1000_HALF           BIT(8)

#define MII_STD_GSTAT_1000_FULL           BIT(11)
#define MII_STD_GSTAT_1000_HALF           BIT(10)

#define MII_STD_XSTATUS_1000BASE_T_FULL   BIT(13)
#define MII_STD_XSTATUS_1000BASE_T_HALF   BIT(12)

/* Clause 45 MMD Registers */
#define MMD_PMA_PMD                       0x01
#define MMD_PMA_PHYID1                    0x02    /* PHY ID 1 */
#define MMD_PMA_PHYID2                    0x03    /* PHY ID 2 */
#define MMD_PMA_FW_VER0                   0xC011
#define MMD_PMA_FW_VER1                   0xC012
#define MMD_PMA_BOOT                      0xC050
#define MMD_PMA_BOOT_FATAL                BIT(0)

#define MMD_AN                            0x07
#define MMD_AN_CTRL                       0x00
#define MMD_AN_CTRL_START_AN              BIT(9)
#define MMD_AN_CTRL_AN_ENABLE             BIT(12)
#define MMD_AN_CTRL_AN_EXT                BIT(13)
#define MMD_AN_STATUS                     0x01
#define MMD_AN_STATUS_CAPABLE             BIT(3)
#define MMD_AN_ADV                        0x10
#define MMD_AN_LPA                        0x13
#define MMD_AN_MULTIG_CTRL                0x20
#define MMD_AN_MULTIG_CTRL_1G             BIT(0)
#define MMD_AN_1G_ABILITY                 0x8002
#define MMD_AN_1G_STATUS                  0x8001
#define MMD_AN_1G_CTRL                    0x8000

#define MMD_PCS                           0x03
#define MMD_PCS_CSTATUS1                  0x8008
#define MMD_PCS_CSTATUS1_LINK             BIT(10)
#define MMD_PCS_CSTATUS1_SPDDONE          BIT(11)
#define MMD_PCS_CSTATUS1_DUPLEX           BIT(13)
#define MMD_PCS_CSTATUS1_SPEED            (BIT(15) | BIT(14))
#define MMD_PCS_CSTATUS1_SPEED_1G         0x2
#define MMD_PCS_CSTATUS1_SPEED_100M       0x1

// TODO: don't belong here
#define PHY_SOFT_RESET_TIMEOUT_MS               600
#define PHY_SOFT_RESET_POLL_MS                  50

#define PHY_AUTONEG_1000_TIMEOUT_MS             5500
#define PHY_AUTONEG_100_TIMEOUT_MS              4000
#define PHY_AUTONEG_POLL_MS                     100

#endif /* _MDII_REGS_H_ */