/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _INTELGBE_API_H_
#define _INTELGBE_API_H_

#include "intelgbe_stmmac.h"

s32 intelgbe_set_mac_type(struct intelgbe_hw *hw);
s32 intelgbe_setup_init_funcs(struct intelgbe_hw *hw, bool init_device);
s32 intelgbe_read_mac_addr_generic(struct intelgbe_hw *hw);
s32 intelgbe_write_mac_addr_generic(struct intelgbe_hw *hw);
s32 intelgbe_get_id(struct intelgbe_hw *hw);
s32 intelgbe_reset(struct intelgbe_hw *hw);
s32 intelgbe_init_hw(struct intelgbe_hw *hw);
s32 intelgbe_uninit_hw(struct intelgbe_hw *hw);
s32 intelgbe_phy_init(struct intelgbe_hw *hw);

/* MDIO helper functions */
s32 intelgbe_phy_write_c22(struct intelgbe_hw *hw, UINT8 reg, UINT16 val);
s32 intelgbe_phy_read_c22(struct intelgbe_hw *hw, UINT8 reg, UINT32 *val);
s32 intelgbe_phy_modify_c22(struct intelgbe_hw *hw, UINT8 regnum,
                           UINT16 mask, UINT16 val);
s32 intelgbe_phy_modify_c22_if_changed(struct intelgbe_hw *hw, UINT8 regnum,
                           UINT16 mask, UINT16 val);
s32 intelgbe_mdio_c45_modify(struct intelgbe_hw *hw, UINT8 devnum,
                             UINT16 regnum, UINT16 mask, UINT16 val);
s32 intelgbe_mdio_c45_read(struct intelgbe_hw *hw, UINT8 devnum,
                             UINT16 regnum, UINT16 *val);
s32 intelgbe_phy_write_c45(struct intelgbe_hw *hw, UINT8 devnum, UINT16 regnum, 
                            UINT16 val);
s32 intelgbe_phy_read_c45(struct intelgbe_hw *hw, UINT8 devnum, UINT16 regnum, 
                            UINT32* val);
s32 intelgbe_phy_modify_c45(struct intelgbe_hw *hw, UINT8 devnum, UINT16 regnum,
                            UINT16 mask, UINT16 val);
s32 intelgbe_phy_modify_c45_if_changed(struct intelgbe_hw *hw, UINT8 devnum, UINT16 regnum,
                            UINT16 mask, UINT16 val);

s32 intelgbe_init_mac_params(struct intelgbe_hw *hw);
s32 intelgbe_init_phy_params(struct intelgbe_hw *hw);
#endif /* _INTELGBE_API_H_ */
