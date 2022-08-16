/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Intelgbe.h"
#include "Intelgbe_osdep.h"
#include "intelgbe_defines.h"
#include "intelgbe_api.h"
#include "intelgbe_stmmac.h"

s32 intelgbe_mdio_c45_modify(struct intelgbe_hw *hw, UINT8 devnum, UINT16 regnum,
                         UINT16 mask, UINT16 val)
{
  UINT16 data;
  int retval = 0;
  bool C45 = 1;

  retval = intelgbe_mdio_read(hw, XPCS_ADDR, devnum, regnum, (UINT32 *)&data, C45);
  if (retval < 0) {
    return retval;
  }

  data &= ~(mask);
  data |= val;

  retval = intelgbe_mdio_write(hw, XPCS_ADDR, devnum, regnum, data, C45);
  if (retval < 0) {
    return retval;
  }

  return INTELGBE_SUCCESS;
}

s32 intelgbe_mdio_c45_read(struct intelgbe_hw *hw, UINT8 devnum, UINT16 regnum,
                       UINT16 *val)
{
  int retval = 0;

  retval = intelgbe_mdio_read(hw, XPCS_ADDR, devnum, regnum, (UINT32 *)val, 1);

  return retval;
}

s32 intelgbe_phy_write_c22(struct intelgbe_hw *hw, UINT8 reg, UINT16 val)
{
  if (hw->phy.ops.mdio_write)
    return hw->phy.ops.mdio_write(hw, hw->phy.addr, 0, reg, val, false);

  return -INTELGBE_NOT_IMPLEMENTED;
}

s32 intelgbe_phy_read_c22(struct intelgbe_hw *hw, UINT8 reg, UINT32* val)
{
  if (hw->phy.ops.mdio_read)
    return hw->phy.ops.mdio_read(hw, hw->phy.addr, 0, reg, val, false);

  return -INTELGBE_NOT_IMPLEMENTED;
}

s32 intelgbe_phy_modify_c22(struct intelgbe_hw *hw, UINT8 regnum,
                   UINT16 mask, UINT16 val)
{
  int retval;
  UINT32 data;

  retval = intelgbe_phy_read_c22(hw, regnum, &data);
  if (retval < 0) {
    return retval;
  }

  return intelgbe_phy_write_c22(hw, regnum, ((data & ~mask) | val));
}

/* Inspired by __phy_modify_changed from phy_core.c */
/* Returns negative errno, 0 if there was no change, and 1 in case of change */
s32 intelgbe_phy_modify_c22_if_changed(struct intelgbe_hw *hw, UINT8 regnum,
                   UINT16 mask, UINT16 val)
{
  int retval;
  u32 oldval;
  u16 newval;

  retval = intelgbe_phy_read_c22(hw, regnum, &oldval);
  if (retval < 0) {
    return retval;
  }

  newval =  (oldval & ~mask) | val;
  if (newval == oldval) {
    return 0;
  }

  retval = intelgbe_phy_write_c22(hw, regnum, newval);
  if (retval < 0) {
    return retval;
  }
  return 1;
}

s32 intelgbe_phy_write_c45(struct intelgbe_hw *hw, UINT8 devnum, UINT16 regnum, UINT16 val)
{
  if (hw->phy.ops.mdio_write)
    return hw->phy.ops.mdio_write(hw, hw->phy.addr, devnum, regnum, val, true);

  return -INTELGBE_NOT_IMPLEMENTED;
}

s32 intelgbe_phy_read_c45(struct intelgbe_hw *hw, UINT8 devnum, UINT16 regnum, UINT32* val)
{
  if (hw->phy.ops.mdio_read)
    return hw->phy.ops.mdio_read(hw, hw->phy.addr, devnum, regnum, val, true);

  return -INTELGBE_NOT_IMPLEMENTED;
}

s32 intelgbe_phy_modify_c45(struct intelgbe_hw *hw, UINT8 devnum, UINT16 regnum,
                   UINT16 mask, UINT16 val)
{
  int retval;
  UINT32 data;

  retval = intelgbe_phy_read_c45(hw, devnum, regnum, &data);
  if (retval < 0) {
    return retval;
  }

  return intelgbe_phy_write_c45(hw, devnum, regnum, ((data & ~mask) | val));
}

/* Returns negative errno, 0 if there was no change, and 1 in case of change */
s32 intelgbe_phy_modify_c45_if_changed(struct intelgbe_hw *hw, UINT8 devnum, UINT16 regnum,
                   UINT16 mask, UINT16 val)
{
  int retval;
  u32 oldval;
  u16 newval;

  retval = intelgbe_phy_read_c45(hw, devnum, regnum, &oldval);
  if (retval < 0) {
    return retval;
  }

  newval =  (oldval & ~mask) | val;
  if (newval == oldval) {
    return 0;
  }

  retval = intelgbe_phy_write_c45(hw, devnum, regnum, newval);
  if (retval < 0) {
    return retval;
  }
  
  return 1;
}


s32 intelgbe_phy_init(struct intelgbe_hw *hw)
{
  int retval = 0;
  if (hw->phy.ops.init)
    retval |= hw->phy.ops.init(hw);
  if (hw->phy.ops.cfg_link)
    retval |= hw->phy.ops.cfg_link(hw);
  return retval;
}

/* MAC operations */
s32 intelgbe_reset(struct intelgbe_hw *hw)
{
  if (hw->mac.ops.reset_hw)
    return hw->mac.ops.reset_hw(hw);

  return INTELGBE_NOT_IMPLEMENTED;
}

s32 intelgbe_get_id(struct intelgbe_hw *hw)
{
  if (hw->mac.ops.get_id)
    return hw->mac.ops.get_id(hw);

  return INTELGBE_NOT_IMPLEMENTED;
}

s32 intelgbe_init_hw(struct intelgbe_hw *hw)
{
  if (hw->mac.ops.init_hw)
    return hw->mac.ops.init_hw(hw);

  return INTELGBE_NOT_IMPLEMENTED;
}

s32 intelgbe_uninit_hw(struct intelgbe_hw *hw)
{
  if (hw->mac.ops.uninit_hw)
    return hw->mac.ops.uninit_hw(hw);

  return INTELGBE_NOT_IMPLEMENTED;
}

/**
 *  intelgbe_read_mac_addr - Reads MAC address
 *  @hw: pointer to the HW structure
 *
 *  Reads the MAC address out of the adapter and stores it in the HW structure.
 *  Currently no func pointer exists and all implementations are handled in the
 *  generic version of this function.
 **/
s32 intelgbe_read_mac_addr_generic(struct intelgbe_hw *hw)
{
  if (hw->mac.ops.read_mac_addr)
    return hw->mac.ops.read_mac_addr(hw);

  return INTELGBE_NOT_IMPLEMENTED;
}

/**
 *  intelgbe_write_mac_addr - Writes MAC address
 *  @hw: pointer to the HW structure
 *
 **/
s32 intelgbe_write_mac_addr_generic(struct intelgbe_hw *hw)
{
  if (hw->mac.ops.write_mac_addr)
    return hw->mac.ops.write_mac_addr(hw);

  return INTELGBE_NOT_IMPLEMENTED;
}

/**
 *  intelgbe_init_mac_params - Initialize MAC function pointers
 *  Ported from e1000_init_mac_params
 *  @hw: pointer to the HW structure
 *
 *  This function initializes the function pointers for the MAC
 *  set of functions.  Called by drivers or by intelgbe_setup_init_funcs.
 **/
s32 intelgbe_init_mac_params(struct intelgbe_hw *hw)
{
  s32 ret_val;

  if (hw->mac.ops.init_params) {
    ret_val = hw->mac.ops.init_params(hw);
    if (ret_val) {
      DEBUGPRINT(CRITICAL, ("MAC Initialization Error\n"));
    }
  } else {
    DEBUGPRINT(CRITICAL, ("mac.init_mac_params was NULL\n"));
    ret_val = -INTELGBE_ERR_CONFIG;
  }

  return ret_val;
}

/**
 *  intelgbe_init_phy_params - Initialize PHY function pointers
 *  ported from e1000_init_phy_params
 *  @hw: pointer to the HW structure
 *
 *  This function initializes the function pointers for the PHY
 *  set of functions.  Called by drivers or by e1000_setup_init_funcs.
 **/
s32 intelgbe_init_phy_params(struct intelgbe_hw *hw)
{
  s32 ret_val = INTELGBE_SUCCESS;

  if (hw->phy.ops.init_params) {
    ret_val = hw->phy.ops.init_params(hw);
    if (ret_val) {
      DEBUGPRINT(CRITICAL, ("PHY Initialization Error\n"));
      goto out;
    }
  } else {
    DEBUGPRINT(CRITICAL, ("phy.init_phy_params was NULL\n"));
    ret_val = -INTELGBE_ERR_CONFIG;
  }

out:
  return ret_val;
}

/**
 *  intelgbe_set_mac_type- Sets MAC type
 *  Ported from e1000_set_mac_type
 *  @hw: pointer to the HW structure
 *
 *  This function sets the mac type of the adapter based on the
 *  device ID stored in the hw structure.
 *  MUST BE FIRST FUNCTION CALLED (explicitly or through
 *  intelgbe_setup_init_funcs()).
 **/
s32 intelgbe_set_mac_type(struct intelgbe_hw *hw)
{
  struct intelgbe_mac_info *mac = &hw->mac;
  s32 ret_val = INTELGBE_SUCCESS;

  DEBUGPRINT(INTELGBE, ("intelgbe_set_mac_type\n"));

  switch (hw->device_id) {

  case EHL_PSE0_STMMAC_RGMII1G_DID:
  case EHL_PSE1_STMMAC_RGMII1G_DID:
    mac->type = intelgbe_stmmac_rgmii;
    break;

  case EHL_PCH_STMMAC_SGMII_DID:
    mac->type = intelgbe_stmmac_sgmii;
    mac->pse_gbe = false;
    break;
  case EHL_PSE0_STMMAC_SGMII1G_DID:
  case EHL_PSE1_STMMAC_SGMII1G_DID:
  case EHL_PSE0_STMMAC_SGMII2G5_DID:
  case EHL_PSE1_STMMAC_SGMII2G5_DID:
    mac->type = intelgbe_stmmac_sgmii;
    mac->pse_gbe = true;
    break;
  case TGLH_PCH1_STMMAC_SGMII_DID:
  case TGLH_PCH2_STMMAC_SGMII_DID:
  case TGLU_PCH1_STMMAC_SGMII_DID:
    mac->type = intelgbe_stmmac_sgmii;
    break;

  default:
    /* Should never have loaded on this device */
    DEBUGPRINT(CRITICAL, ("Unknown device id: %X\n", hw->device_id));
    ret_val = -INTELGBE_ERR_MAC_INIT;
    break;
  }

  return ret_val;
}

/**
 *  intelgbe_setup_init_funcs - Initializes function pointers
 *  Ported from e1000_setup_init_funcs
 *  @hw: pointer to the HW structure
 *  @init_device: true will initialize the rest of the function pointers
 *                getting the device ready for use.  false will only set
 *                MAC type and the function pointers for the other init
 *                functions.  Passing false will not generate any hardware
 *                reads or writes.
 *
 *  This function must be called by a driver in order to use the rest
 *  of the 'shared' code files. Called by drivers only.
 *  TODO: Init function pointers to generic implementations. See
 *        e1000_setup_init_funcs
 *        Do we need something similar to e1000_init_nvm_params?
 **/
s32 intelgbe_setup_init_funcs(struct intelgbe_hw *hw, bool init_device)
{
  s32 ret_val;

  /* Can't do much good without knowing the MAC type. */
  ret_val = intelgbe_set_mac_type(hw);
  if (ret_val < 0) {
    DEBUGPRINT(CRITICAL, ("ERROR: MAC type could not be set properly.\n"));
    return ret_val;
  }

  if (!hw->hw_addr) {
    DEBUGPRINT(CRITICAL, ("ERROR: Registers not mapped\n"));
    return -INTELGBE_ERR_CONFIG;
  }

  /*
   * Placeholder for Init function pointers to generic implementations. We do this first
   * allowing a driver module to override it afterward.
  */

  /*
   * Set up the init function pointers. These are functions within the
   * adapter family file that sets up function pointers for the rest of
   * the functions in that family.
  */
  switch (hw->mac.type) {

  case intelgbe_stmmac_rgmii:
  case intelgbe_stmmac_sgmii:
    intelgbe_init_function_pointers_stmmac(hw);
    break;

  default:
    DEBUGPRINT(CRITICAL, ("MAC device not supported\n"));
    return -INTELGBE_ERR_CONFIG;
    break;
  }

  /*
   * Initialize the rest of the function pointers. These require some
   * register reads/writes in some cases.
   */
  if (init_device) {
    ret_val = intelgbe_init_mac_params(hw);
    if (ret_val)
      return ret_val;

    ret_val = intelgbe_init_phy_params(hw);
    if (ret_val)
      return ret_val;
  }

  return ret_val;
}
