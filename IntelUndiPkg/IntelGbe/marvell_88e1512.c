/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Intelgbe.h"
#include "marvell_88e1512.h"

s32 marvell_88e1512_link_status_change(struct intelgbe_hw *hw, bool *intr_sig)
{
  int retval = 0;
  u32 val;

  /* Change to Page 0 */
  retval = intelgbe_phy_write_c22(hw, REG_PAGE_ADDR, 0);
  if (retval < 0) {
    return retval;
  }
  /* Read interrupt status */
  retval = intelgbe_phy_read_c22(hw, REG_COPPER_SPE_INT_STATUS, &val);
  *intr_sig = (val & REG_COPPER_SPE_INT_LINK) ? true : false;

  return retval;
}

/* NOTE: link, link_speed & full_duplex value are valid only if
 * function call return no error
 */
int marvell_88e1512_read_status(struct intelgbe_hw *hw, bool *link,
                                s32 *link_speed, s8 *full_duplex)
{
  int retval;
  UINT32 val;

  /* Change to Page 0 */
  retval = intelgbe_phy_write_c22(hw, REG_PAGE_ADDR, 0);
  if (retval < 0) {
    return retval;
  }
  /* Read Copper Status */
  retval = intelgbe_phy_read_c22(hw, REG_COPPER_SPE_STATUS_1, &val);
  if (retval < 0) {
    return retval;
  }

  *link = ((val & REG_COPPER_SPE_STATUS_LINK) == 0 ? false : true);
  if (*link) {
    if (val & REG_COPPER_SPE_STATUS_FULL_DUPLEX) {
      *full_duplex = true;
    } else {
      *full_duplex = false;
    }

    val &= REG_COPPER_SPE_STAUS_SPEED_MASK;
    if (val == REG_COPPER_SPE_STAUS_SPEED_1000) {
      *link_speed = 1000;
    } else if (val == REG_COPPER_SPE_STAUS_SPEED_100) {
      *link_speed = 100;
    } else {
      *link_speed = 10;
    }
  }
  return 0;
}

STATIC s32 intelgbe_phy_cfg_link(struct intelgbe_hw *hw)
{
  int retval;

  /* Change to Page 0 */
  retval = intelgbe_phy_write_c22(hw, REG_PAGE_ADDR, 0);
  if (retval < 0) {
    return retval;
  }

  return mii_phy_config_link(hw, false);
}



STATIC s32 intelgbe_phy_initialize(struct intelgbe_hw *hw)
{
  int retval;
  struct intelgbe_phy_info *phy = &hw->phy;

  if (phy->interface == PHY_INTERFACE_SGMII) {
    DEBUGPRINT (INIT, ("Entered SGMII"));
    /* Change to Page 18 */
    retval = intelgbe_phy_write_c22(hw, REG_PAGE_ADDR, 18);
    if (retval < 0) {
      return retval;
    }
    DEBUGPRINT (INIT, ("PHY write"));

    /* Configure to sgmii copper mode */
    retval |= intelgbe_phy_modify_c22(hw, REG_GENERAL_CTRL_1,
                                  REG_GENERAL_CTRL_MODE_MASK,
                                  REG_GENERAL_CTRL_1_SGMII_COPPER);

    DEBUGPRINT (INIT, ("configured copper mode"));
    /* PHY reset */
    retval |= intelgbe_phy_modify_c22(hw, REG_GENERAL_CTRL_1, 0,
                                  REG_GENERAL_CTRL_1_RESET);

    DEBUGPRINT (INIT, ("phy reset done "));
    if (retval < 0) {
      return retval;
    }
    DEBUGPRINT (INIT, ("Success"));
  }

  /* Change to Page 3 */
  retval = intelgbe_phy_write_c22(hw, REG_PAGE_ADDR, 3);
  if (retval < 0) {
    return retval;
  }

  /* PHY LED config: LED[0] .. Link, LED[1] .. Activity */
  retval = intelgbe_phy_write_c22(hw, REG_LED_FUNC_CTRL, REG_LED_FUNC_CTRL_DEF);
  if (retval < 0) {
    return retval;
  }

  /* Change to Page 2 */
  retval = intelgbe_phy_write_c22(hw, REG_PAGE_ADDR, 2);
  if (retval < 0) {
    return retval;
  }
  /* Pass as is and do not pad odd nibble preambles in copper receive
   * packets
   */
  retval = intelgbe_phy_modify_c22(hw, REG_SPE_MAC_CTRL_1, 0,
                               REG_SPE_MAC_CTRL_1_PAD_ODD);
  if (retval < 0) {
    return retval;
  }

  /* Change to Page 0 */
  retval = intelgbe_phy_write_c22(hw, REG_PAGE_ADDR, 0);
  if (retval < 0) {
    return retval;
  }

  mii_phy_soft_reset(hw, false);

  DEBUGPRINT (CRITICAL, ("PHY Init Success\n"));

  return INTELGBE_SUCCESS;
}

s32 intelgbe_init_phy_ops_marvell_88e1512(struct intelgbe_hw *hw)
{
  DEBUGPRINT (PHYFUNC, ("intelgbe_init_phy_ops_marvell_88e1512\n"));
  struct intelgbe_phy_info *phy = &hw->phy;
  /* PHY init */
  phy->ops.init = intelgbe_phy_initialize;
  /* PHY write */
  phy->ops.cfg_link = intelgbe_phy_cfg_link;
  /* Link status */
  phy->ops.link_status_change = marvell_88e1512_link_status_change;
  /* PHY status */
  phy->ops.status = marvell_88e1512_read_status;
  phy->type = PHY_MARVELL_88E1512;

  return INTELGBE_SUCCESS;
}
