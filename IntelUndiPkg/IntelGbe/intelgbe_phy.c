/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Intelgbe.h"
#include "mii_regs.h"

/* MII Compatible PHY functions
 * phy reset and auto-nego implementation is from phy_sam_gmac.c
 */

/* Find and store valid Phy ID.
 * Loop through Phy Address 0..32.
 * First assume it's C22 devices and get Phy ID by reading register 2 and 3.
 * If value is all Fs, assume its C45 device and get Phy ID by reading PMA/PMD device register 2 and 3 
 */
s32 mii_phy_id_get(struct intelgbe_hw *hw)
{
  s32 retval;
  u32 phy_reg;
  u32 phy_id = 0;
  u8  phy_addr = 0;
  bool phy_c45;

  DEBUGPRINT (PHYFUNC, ("mii_phy_id_get\n"));

  while (((phy_id == 0xFFFFFFFF) || (phy_id == 0)) && (phy_addr < 32)) {
    hw->phy.addr = phy_addr;
    phy_c45 = false;

    retval = intelgbe_phy_read_c22(hw, MII_STD_PHYID1, &phy_reg);
    if (retval) return retval;
    phy_id = (phy_reg & BIT_MASK(16)) << 16;

    retval = intelgbe_phy_read_c22(hw, MII_STD_PHYID2, &phy_reg);
    if (retval) return retval;
    phy_id |= (phy_reg & BIT_MASK(16));

    DEBUGPRINT (CRITICAL, ("C22 PHY ID %X at addr %X\n", phy_id, phy_addr));

    if ((phy_id == 0xFFFFFFFF) || (phy_id == 0)) {
      phy_c45 = true;

      retval = intelgbe_phy_read_c45(hw, MMD_PMA_PMD, MMD_PMA_PHYID1, &phy_reg);
      if (retval) return retval;
      phy_id = (phy_reg & BIT_MASK(16)) << 16;

      retval = intelgbe_phy_read_c45(hw, MMD_PMA_PMD, MMD_PMA_PHYID2, &phy_reg);
      if (retval) return retval;
      phy_id |= (phy_reg & BIT_MASK(16));
      
      DEBUGPRINT (CRITICAL, ("C45 PHY ID %X at addr %X\n", phy_id, phy_addr));
    }
    phy_addr++;
  }

  hw->phy.id = phy_id;
  hw->phy.c45 = phy_c45;

  return phy_addr < 32 ? INTELGBE_SUCCESS : -INTELGBE_ERR_PHY;
}

/* refer to genphy_read_abilities() in phy_device.c */
int mii_phy_get_supported(struct intelgbe_hw *hw)
{
  struct intelgbe_phy_info *phy = &hw->phy;
  u32 bmsr_val, estat_val;
  int retval;

  DEBUGPRINT (PHYFUNC, ("mii_phy_get_supported\n"));
  if(phy->c45)
    retval = intelgbe_phy_read_c45(hw, MMD_AN, MMD_AN_1G_ABILITY, &bmsr_val);
  else
    retval = intelgbe_phy_read_c22(hw, MII_STD_STATUS, &bmsr_val);
  if (retval < 0) return retval;

  /* 10/100 support */
  if (bmsr_val & MII_STD_STATUS_10_FULL) {
    phy->support |= PHY_SUPPORT_10_FULL;
    DEBUGPRINT (CRITICAL, ("PHY Supported: 10BASE-T, Full Duplex\n"));
  }
  if (bmsr_val & MII_STD_STATUS_10_HALF) {
    phy->support |= PHY_SUPPORT_10_HALF;
    DEBUGPRINT (CRITICAL, ("PHY Supported: 10BASE-T, Half Duplex\n"));
  }
  if (bmsr_val & MII_STD_STATUS_100BASE_X_FULL) {
    phy->support |= PHY_SUPPORT_100_FULL;
    DEBUGPRINT (CRITICAL, ("PHY Supported: 100BASE-X, Full Duplex\n"));
  }
  if (bmsr_val & MII_STD_STATUS_100BASE_X_HALF) {
    phy->support |= PHY_SUPPORT_100_HALF;
    DEBUGPRINT (CRITICAL, ("PHY Supported: 100BASE-X, Half Duplex\n"));
  }

  /* 1000 support */
  if (bmsr_val & MII_STD_STATUS_EXTEND_STATUS) {
    if (phy->c45)
      estat_val = bmsr_val;
    else {
      retval = intelgbe_phy_read_c22(hw, MII_STD_XSTATUS, &estat_val);
      if (retval < 0) return retval;
    }
    if (estat_val & MII_STD_XSTATUS_1000BASE_T_FULL) {
      phy->support |= PHY_SUPPORT_1000_FULL;
      DEBUGPRINT (CRITICAL, ("PHY Supported: 1000BASE-T, Full Duplex\n"));
    }
    if (estat_val & MII_STD_XSTATUS_1000BASE_T_HALF) {
      phy->support |= PHY_SUPPORT_1000_HALF;
      DEBUGPRINT (CRITICAL, ("PHY Supported: 1000BASE-T, Half Duplex\n"));
    }
  }
  return INTELGBE_SUCCESS;
}

int mii_phy_soft_reset(struct intelgbe_hw *hw, bool wait)
{
  int retries = PHY_SOFT_RESET_TIMEOUT_MS / PHY_SOFT_RESET_POLL_MS;
  int retval;
  u32 status;

  DEBUGPRINT (PHYFUNC, ("mii_phy_soft_reset\n"));
  /* Issue a soft reset */
  retval = intelgbe_phy_modify_c22(hw, MII_STD_CTRL, MII_STD_CTRL_RESET,
    MII_STD_CTRL_RESET);
  if (retval < 0) return retval;

  if (wait) {
    do {
      if (retries-- == 0) {
        return -INTELGBE_BUSY;
      }

      msec_delay(PHY_SOFT_RESET_POLL_MS);

      retval = intelgbe_phy_read_c22(hw, MII_STD_CTRL, &status);
      if (retval < 0) continue;
    } while (status & MII_STD_CTRL_RESET);
  }
  retval = intelgbe_phy_read_c22(hw, MII_STD_CTRL, &status);
  DEBUGPRINT (PHYFUNC, ("STD CTRL: %X\n", status));
  return retval;
}

/* To configure PHY link setting
 * refer to __genphy_config_aneg() in phy_device.c
 *
 */
int mii_phy_config_link(struct intelgbe_hw *hw, bool changed)
{
  struct intelgbe_phy_info *phy = &hw->phy;
  int retries = PHY_AUTONEG_1000_TIMEOUT_MS / PHY_AUTONEG_POLL_MS;
  int retval = 0;
  u32 readval = 0;
  u16 mask = 0;
  u16 advertise = 0;
  u16 advertise_1000 = 0;

  DEBUGPRINT (PHYFUNC, ("mii_phy_config_link\n"));

  /* Placeholder for genphy_config_eee_advert */

  /* Placeholder for genphy_setup_forced() in phy_device.c */

  /* refer to genphy_config_advert() in phy_device.c */
  if (!phy->support) {
    retval = mii_phy_get_supported(hw);
    if (retval < 0) return retval;
  }
  /* Obtain PHY supported list and set advertise */
  if (phy->support & PHY_SUPPORT_1000_FULL) {
    advertise_1000 |= MII_STD_GCTRL_1000_FULL;
  }
  if (phy->support & PHY_SUPPORT_1000_HALF) {
    advertise_1000 |= MII_STD_GCTRL_1000_HALF;
  }
  if (phy->support & PHY_SUPPORT_100_FULL) {
    advertise |= MII_AN_ADV_TAF_100_FULL;
  }
  if (phy->support & PHY_SUPPORT_100_HALF) {
    advertise |= MII_AN_ADV_TAF_100_HALF;
  }
  if (phy->support & PHY_SUPPORT_10_FULL) {
    advertise |= MII_AN_ADV_TAF_10_FULL;
  }
  if (phy->support & PHY_SUPPORT_10_HALF) {
    advertise |= MII_AN_ADV_TAF_10_HALF;
  }

  /* Do we need to advertise sym and asym pause? */

  /* Configure 10/100BASE-T advertise */
  mask = (MII_AN_ADV_TAF_10_HALF | MII_AN_ADV_TAF_10_FULL |
          MII_AN_ADV_TAF_100_HALF | MII_AN_ADV_TAF_100_FULL |
          MII_AN_ADV_TAF_100BASE_T4 | MII_AN_ADV_TAF_PAUSE |
          MII_AN_ADV_TAF_ASYM_PAUSE);
  advertise |= MII_AN_ADV_TAF_SEL_IEEE_802_3;
  if(phy->c45)
    retval = intelgbe_phy_modify_c45_if_changed(hw, MMD_AN, MMD_AN_ADV, mask, advertise);
  else
    retval = intelgbe_phy_modify_c22_if_changed(hw, MII_AN_ADV, mask, advertise);
  if (retval < 0) return retval;
  if (retval > 0) changed = true;
  /* Configure 1000BASE-T advertise */
  if(phy->c45)
    retval = intelgbe_phy_read_c45(hw, MMD_AN, MMD_AN_1G_ABILITY, &readval);
  else
    retval = intelgbe_phy_read_c22(hw, MII_STD_STATUS, &readval);
  if (retval < 0) return retval;
  if (readval & MII_STD_STATUS_EXTEND_STATUS) {
    mask = MII_STD_GCTRL_1000_HALF | MII_STD_GCTRL_1000_FULL;
    if(phy->c45)
      retval = intelgbe_phy_modify_c45_if_changed(hw, MMD_AN, MMD_AN_1G_CTRL, mask, advertise_1000);
    else
      retval = intelgbe_phy_modify_c22_if_changed(hw, MII_STD_GCTRL, mask, advertise_1000);
    if (retval < 0) return retval;
    if (retval > 0) changed = true;
  }

  /* Advertisement hasn't changed, but maybe aneg was never on to
   * begin with?  Or maybe phy was isolated?
   */
  if (!changed) {
    if(phy->c45)
      retval = intelgbe_phy_read_c45(hw, MMD_AN, MMD_AN_CTRL, &readval);
    else
      retval = intelgbe_phy_read_c22(hw, MII_STD_CTRL, &readval);
    if (retval < 0) return retval;
    if (!(readval & MII_STD_CTRL_AUTONEG_ENABLE) || (readval & MII_STD_CTRL_ISOLATE)) changed = true;
  }

  /* Only restart aneg if we are advertising something different
   * than we were before.
   * refer to genphy_restart_aneg() in phy_device.c
   */
  if (changed) {
    if(phy->c45) 
      retval |= intelgbe_phy_write_c45(hw, MMD_AN, MMD_AN_CTRL, 
                                        (MMD_AN_CTRL_START_AN | MMD_AN_CTRL_AN_ENABLE | MMD_AN_CTRL_AN_EXT));
    else
      retval = intelgbe_phy_modify_c22(hw, MII_STD_CTRL, MII_STD_CTRL_ISOLATE,
                                        (MII_STD_CTRL_AUTONEG_ENABLE | MII_STD_CTRL_AUTONEG_RESTART));
    if (retval < 0) return retval;

    /* Wait for the auto-negotiation process to complete
     * Let's not fail the autoneg when timeout. Maybe the port is not connected.
     * refer to phy_poll_MMD_AN_done() in phy.c
     */
    do {
      msec_delay(PHY_AUTONEG_POLL_MS);
      if (phy->c45)
        retval = intelgbe_phy_read_c45(hw, MMD_AN, MMD_AN_STATUS, &readval);
      else
        retval = intelgbe_phy_read_c22(hw, MII_STD_STATUS, &readval); 
      if (retval < 0) return retval;
      readval &= MII_STD_STATUS_AUTONEG_COMPLETE;
    } while (!readval && --retries);
    if (!readval)
      DEBUGPRINT (CRITICAL, ("Aneg timeout\n"));
  }

  /* Print Link Partner advertise speed and duplex */
  DEBUGPRINT (CRITICAL, ("Link partner advertised: \n"));
  /* 10/100BASE-T */
  if (phy->c45)
    retval = intelgbe_phy_read_c45(hw, MMD_AN, MMD_AN_LPA, &readval);
  else
    retval = intelgbe_phy_read_c22(hw, MII_AN_LPA, &readval); 
  if (retval < 0) return retval;
  if (readval & MII_AN_ADV_TAF_100_FULL) {
    DEBUGPRINT (CRITICAL, ("Speed: 100Mbps Duplex: Full\n"));
  }
  if (readval & MII_AN_ADV_TAF_100_HALF) {
    DEBUGPRINT (CRITICAL, ("Speed: 100Mbps Duplex: Half\n"));
  }
  if (readval & MII_AN_ADV_TAF_10_FULL) {
    DEBUGPRINT (CRITICAL, ("Speed: 10Mbps Duplex: Full\n"));
  }
  if (readval & MII_AN_ADV_TAF_10_HALF) {
    DEBUGPRINT (CRITICAL, ("Speed: 10Mbps Duplex: Half\n"));
  }
  /* 1000BASE-T */
  if (phy->c45)
    retval = intelgbe_phy_read_c45(hw, MMD_AN, MMD_AN_1G_STATUS, &readval);
  else
    retval = intelgbe_phy_read_c22(hw, MII_STD_GSTAT, &readval);
  if (retval < 0) return retval;
  if (readval & MII_STD_GSTAT_1000_FULL) {
    DEBUGPRINT (CRITICAL, ("Speed: 1000Mbps Duplex: Full\n"));
  }
  if (readval & MII_STD_GSTAT_1000_HALF) {
    DEBUGPRINT (CRITICAL, ("Speed: 1000Mbps Duplex: Half\n"));
  }

  return INTELGBE_SUCCESS;
}