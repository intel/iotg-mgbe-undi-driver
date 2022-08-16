/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Intelgbe.h"
#include "marvell_88e2110.h"

STATIC s32 m88e2110_link_status_change (struct intelgbe_hw *hw, bool *intr_sig)
{
    struct intelgbe_phy_info *phy = &hw->phy;
    u32 phyreg;
    s32 retval;
    bool prevlink, currlink;

    retval = intelgbe_phy_read_c45(hw, MMD_AN, MMD_AN_STATUS, &phyreg);
    if (retval < 0) return retval;
    if (phyreg & MII_STD_STATUS_LINK_STATUS)
        currlink = true;
    else
        currlink = false;

    prevlink = phy->link_up;

    //  prevlink    currlink    intr_sig
    //      0           0           0
    //      1           1           0
    //      0           1           1
    //      1           0           1
    if ((prevlink && currlink) || (!prevlink && !currlink))
        *intr_sig = false;
    else
        *intr_sig = true;
    
    return INTELGBE_SUCCESS;
}

/* NOTE: link, link_speed & full_duplex value are valid only if
 * function call return no error
 */
STATIC s32 m88e2110_read_status (struct intelgbe_hw *hw, bool *link,
                                s32 *link_speed, s8 *full_duplex)
{
    u32 speed;
    u32 phyreg;
    s32 retval;

    retval = intelgbe_phy_read_c45(hw, MMD_PCS, MMD_PCS_CSTATUS1, &phyreg);
    if (retval < 0) return retval;

    if (phyreg & MMD_PCS_CSTATUS1_LINK)
        *link = true;
    else
        *link = false;

    if (phyreg & MMD_PCS_CSTATUS1_DUPLEX) 
        *full_duplex = 1;
    else 
        *full_duplex = 0;

    speed = (phyreg & MMD_PCS_CSTATUS1_SPEED) >> 14;
    switch (speed) {
        case MMD_PCS_CSTATUS1_SPEED_1G:
            *link_speed = 1000;
            break;
        case MMD_PCS_CSTATUS1_SPEED_100M:
            *link_speed = 100;
            break;
        default:
            *link_speed = 10;
            break;
    }
    return INTELGBE_SUCCESS;
}

STATIC s32 m88e2110_phy_cfg_link (struct intelgbe_hw *hw)
{
    s32 retval;

    retval = intelgbe_phy_write_c45 (hw, MMD_AN, MMD_AN_MULTIG_CTRL, MMD_AN_MULTIG_CTRL_1G);
    if (retval < 0) return retval;
    return mii_phy_config_link (hw, false);
}

STATIC s32 m88e2110_phy_initialize (struct intelgbe_hw *hw)
{
    s32 retval;
    u32 phyreg;
    u32 fwver = 0xFFFFFFFF;

    retval = intelgbe_phy_read_c45 (hw, MMD_PMA_PMD, MMD_PMA_BOOT, &phyreg);
    if (phyreg & MMD_PMA_BOOT_FATAL) {
        DEBUGPRINT(CRITICAL, ("m88e2110 PHY Boot fatal error has occured\n"));
        return -INTELGBE_ERR_PHY;
    }

    retval |= intelgbe_phy_read_c45 (hw, MMD_PMA_PMD, MMD_PMA_FW_VER1, &phyreg);
    fwver = (phyreg & BIT_MASK(16)) << 16;
    retval |= intelgbe_phy_read_c45 (hw, MMD_PMA_PMD, MMD_PMA_FW_VER0, &phyreg);
    fwver |= phyreg & BIT_MASK(16);
    DEBUGPRINT (PHYFUNC, ("M88E2110 FW Ver %X\n", fwver));

    return retval;
}

s32 intelgbe_init_phy_ops_marvell_88e2110 (struct intelgbe_hw *hw)
{
    DEBUGPRINT (PHYFUNC, ("intelgbe_init_phy_ops_marvell_88e2110\n"));
    struct intelgbe_phy_info *phy = &hw->phy;
    /* PHY init */
    phy->ops.init = m88e2110_phy_initialize;
    /* PHY write */
    phy->ops.cfg_link = m88e2110_phy_cfg_link;
    /* Link status */
    phy->ops.link_status_change = m88e2110_link_status_change;
    /* PHY status */
    phy->ops.status = m88e2110_read_status;
    phy->type = PHY_MARVELL_88E2110;

    return INTELGBE_SUCCESS;
}
