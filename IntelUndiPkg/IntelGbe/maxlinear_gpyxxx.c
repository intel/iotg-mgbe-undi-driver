/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Intelgbe.h"
#include "maxlinear_gpyxxx.h"

/* Poll for Link Status change
 * TODO: check other events also e.g. speed change. See GPY215 Datasheet 3.4.6.
 */
s32 maxlinear_gpyxxx_link_status_change(struct intelgbe_hw *hw, bool *intr_sig)
{
  int retval = 0;
  u32 val;

  retval = intelgbe_phy_read_c22(hw, GPY_PHY_ISTAT, &val);
  *intr_sig = (val & GPY_PHY_ISTAT_LSTC) ? true : false;

  return retval;
}

/* NOTE: link, link_speed & full_duplex value are valid only if
 * function call return no error
 * TODO: Check VSPEC1_SGMII_STAT. See GPY215 datasheet 3.4.1
 */
int maxlinear_gpyxxx_read_status(struct intelgbe_hw *hw, bool *link,
                                s32 *link_speed, s8 *full_duplex)
{
  int retval;
  UINT32 val;

  DEBUGPRINT (PHYFUNC, ("maxlinear_gpyxxx_read_status\n"));

  retval = intelgbe_phy_read_c22(hw, GPY_PHY_MIISTAT, &val);
  if (retval < 0) {
    return retval;
  }

  *link = ((val & GPY_PHY_MIISTAT_LS) == 0 ? false : true);
  if (*link) {
    if (val & GPY_PHY_MIISTAT_DPX) {
      *full_duplex = true;
    } else {
      *full_duplex = false;
    }

    val &= GPY_PHY_MIISTAT_SPEED;
    if (val == GPY_PHY_MIISTAT_SPEED_BZ2G5) {
      *link_speed = 2500;
    } else if (val == GPY_PHY_MIISTAT_SPEED_GIGA) {
      *link_speed = 1000;
    } else if (val == GPY_PHY_MIISTAT_SPEED_FAST) {
      *link_speed = 100;
    } else {
      *link_speed = 10;
    }
  }
  return 0;
}

STATIC s32 intelgbe_phy_cfg_link(struct intelgbe_hw *hw)
{

  DEBUGPRINT (PHYFUNC, ("intelgbe_phy_cfg_link\n"));

  return mii_phy_config_link(hw, false);
}

/* Ported from gpy_config_init in intel-gpy.c */
STATIC s32 intelgbe_phy_initialize(struct intelgbe_hw *hw)
{
  DEBUGPRINT (PHYFUNC, ("intelgbe_phy_initialize\n"));

  /* TODO: Show GPY PHY FW version in log */

  /* TODO: In GPY PHY FW, by default EEE mode is enabled. So, disable EEE mode
   * during power up. See Linux implementation.
   */

  /* TODO: Keep LED settings to default after discussing with CK. Revisit LED later.
   * Not debug printing LED settings to reduce log file entries. To enable, you can refer to previous commits.
   */

  DEBUGPRINT (CRITICAL, ("PHY Init Success\n"));

  return INTELGBE_SUCCESS;
}

s32 intelgbe_init_phy_ops_maxlinear_gpyxxx(struct intelgbe_hw *hw)
{
  DEBUGPRINT (PHYFUNC, ("intelgbe_init_phy_ops_maxlinear_gpyxxx\n"));
  struct intelgbe_phy_info *phy = &hw->phy;
  /* PHY init */
  phy->ops.init = intelgbe_phy_initialize;
  /* PHY write */
  phy->ops.cfg_link = intelgbe_phy_cfg_link;
  /* Link status */
  phy->ops.link_status_change = maxlinear_gpyxxx_link_status_change;
  /* PHY status */
  phy->ops.status = maxlinear_gpyxxx_read_status;
  phy->type = PHY_MAXLINEAR_GPY211;

  return INTELGBE_SUCCESS;
}
