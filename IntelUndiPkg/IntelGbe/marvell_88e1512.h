/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _MARVELL_88E1512_DEFINES_H_
#define _MARVELL_88E1512_DEFINES_H_

#include "mii_regs.h"


/* PHY */

#define MARVELL_PHY_ID_88E1512                  0x01410DD1
#define MARVELL_PHY_VENDOR                      MII_PHY_VENDOR(MARVELL_PHY_ID_88E1512)
#define MARVELL_PHY_MODEL_88E1510               MII_PHY_MODEL(MARVELL_PHY_ID_88E1512)

/* Refer Datasheet Section 3.2 PHY MDIO Register Description
 * Register is access through Page 0 to Page 18
 * Page 0 is MII compatible register
 * For e.g: To access Page 18 General Control Register 1
 * Need to update Page Address Register(Page Any, Register 22)
 * Page Number to 18
 */
#define REG_PAGE_ADDR                           0x16

/* General Control Register 1 - Page 18, Reg 20 */
#define REG_GENERAL_CTRL_1                      0x14
#define REG_GENERAL_CTRL_MODE_MASK              BIT_MASK(3)
#define REG_GENERAL_CTRL_1_SGMII_COPPER         BIT(0)
#define REG_GENERAL_CTRL_1_RESET                BIT(15)

/* Copper Specific Interrupt Enable Register - Page 0, Reg 18 */
#define REG_COPPER_SPE_INT_ENABLE               0x12

/* Copper Specific Interrupt Status Register - Page 0, Reg 19 */
#define REG_COPPER_SPE_INT_STATUS               0x13
#define REG_COPPER_SPE_INT_LINK                 BIT(10)
#define REG_COPPER_SPE_INT_AUTONEG              BIT(11)
#define REG_COPPER_SPE_INT_DUPLEX               BIT(13)
#define REG_COPPER_SPE_INT_SPEED                BIT(14)
#define REG_COPPER_SPE_INT_DEF                  (REG_COPPER_SPE_INT_LINK | \
                                                 REG_COPPER_SPE_INT_AUTONEG | \
                                                 REG_COPPER_SPE_INT_DUPLEX | \
                                                 REG_COPPER_SPE_INT_SPEED)

/* LED[2:0] Function Control Register - Page 3, Reg 16 */
#define REG_LED_FUNC_CTRL                       0x10
/* LED[1] Control Bit 7:4 0011 = On - Activity, Off - No Activity */
#define REG_LED_FUNC_CTRL_DEF                   (BIT(4) | BIT(5))

/* MAC Specific Control Register 1 - Page 2, Reg 16 */
#define REG_SPE_MAC_CTRL_1                      0x10

#define REG_SPE_MAC_CTRL_1_PAD_ODD              BIT(6)

/* Copper Specific Status Register 1 - Page 0, Reg 17 */
#define REG_COPPER_SPE_STATUS_1                 0x11
#define REG_COPPER_SPE_STATUS_LINK              BIT(10)
#define REG_COPPER_SPE_STATUS_FULL_DUPLEX       BIT(13)
#define REG_COPPER_SPE_STAUS_SPEED_MASK         (BIT(15) | BIT(14))
#define REG_COPPER_SPE_STAUS_SPEED_1000         BIT(15)
#define REG_COPPER_SPE_STAUS_SPEED_100          BIT(14)

/* MDIO Manageable Device (MMD) Auto-Negotiation */
#define MDIO_AN_DEVICE                          7

/* PHY EEE Advertisement Register - Device 7, Reg 60 */
#define MDIO_AN_EEE_ADV                         60
/* 100M & 1000M link speed */
#define MDIO_AN_EEE_ADV_SPEED                   (BIT(1) | BIT(2))

#define PHY_AUTONEG_TIMEOUT_MS                  5500
#define PHY_AUTONEG_POLL_MS                     100

#define MII_M1111_PHY_EXT_SR                    0x1b
#define MII_M1111_HWCFG_MODE_MASK               0xf

#define MII_M1111_HWCFG_FIBER_COPPER_RES        BIT(13)
#define MII_M1111_HWCFG_FIBER_COPPER_AUTO       BIT(15)


#endif /* _MARVELL_88E1512_DEFINES_H_ */
