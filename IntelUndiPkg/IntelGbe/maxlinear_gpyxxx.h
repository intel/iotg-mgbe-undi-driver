/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _MAXLINEAR_GPYXXX_DEFINES_H_
#define _MAXLINEAR_GPYXXX_DEFINES_H_

#include "mii_regs.h"


/** MMD Access Control Register - Device 0, Reg 13 and MMD Access Data Register - Device 0, Reg 14
 * The MMD access control register and MMD Access Data Register is used to access the MMD register space.
 * This uses address directing as specified in IEEE802.3 Clause 22 Extended.
 **/
#define STD_MMDCTRL                             0xD
#define STD_MMDDATA                             0xE

/* PHY ID*/
#define MAXLINEAR_PHY_ID_GPY                    0x67C9DC00
#define MAXLINEAR_PHY_VENDOR                    MII_PHY_VENDOR(MAXLINEAR_PHY_ID_GPY)
#define MAXLINEAR_PHY_MODEL                     MII_PHY_MODEL(MAXLINEAR_PHY_ID_GPY)

/* GPY Interrupt Status Register - Device 0, Reg 26 */
#define GPY_PHY_ISTAT                           0x1A
#define GPY_PHY_ISTAT_LSTC                      BIT(0)

/* GPY Media-Independent Interface Status - Device 0, Reg 24 */
#define GPY_PHY_MIISTAT                         0x18
#define GPY_PHY_MIISTAT_LS                      BIT(10)
#define GPY_PHY_MIISTAT_DPX                     BIT(3)
#define GPY_PHY_MIISTAT_SPEED                   (BIT(2) | BIT(1) | BIT(0))
#define GPY_PHY_MIISTAT_SPEED_TEN               0
#define GPY_PHY_MIISTAT_SPEED_FAST              0x1
#define GPY_PHY_MIISTAT_SPEED_GIGA              0x2
#define GPY_PHY_MIISTAT_SPEED_ANEG              0x3
#define GPY_PHY_MIISTAT_SPEED_BZ2G5             0x4

/* GPY LED Control Register - Device 0, Reg 27 */
#define GPY_PHY_LED                             0x1B
/* GPY Configuration for LED Pin 0 - Device 30, Reg 1 */
#define VSPEC1_LED0                             0x1
/* GPY Configuration for LED Pin 1 - Device 30, Reg 2 */
#define VSPEC1_LED1                             0x2
/* GPY Configuration for LED Pin 2 - Device 30, Reg 3 */
#define VSPEC1_LED2                             0x3
/* GPY Configuration for LED Pin 3 - Device 30, Reg 4 */
#define VSPEC1_LED3                             0x4

/* Copied from marvell_88e1512.h */

#define PHY_AUTONEG_1000_TIMEOUT_MS             5500
#define PHY_AUTONEG_100_TIMEOUT_MS              4000
#define PHY_AUTONEG_POLL_MS                     100

#define PHY_SOFT_RESET_TIMEOUT_MS               600
#define PHY_SOFT_RESET_POLL_MS                  50

/* PHY */

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


#endif /* _MAXLINEAR_GPYXXX_DEFINES_H_ */
