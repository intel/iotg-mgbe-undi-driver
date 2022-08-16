/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _INTELGBE_HW_H_
#define _INTELGBE_HW_H_

#ifndef __le16
#define __le16 u16
#endif
#ifndef __le32
#define __le32 u32
#endif
#ifndef __le64
#define __le64 u64
#endif

/* Transmit Descriptor */
struct intelgbe_tx_desc {
  __le32 des0;
  __le32 des1;
  __le32 des2;
  __le32 des3;
};

/* Receive Descriptor */
struct intelgbe_rx_desc {
  __le32 des0;
  __le32 des1;
  __le32 des2;
  __le32 des3;
};

struct intelgbe_hw;
struct intelgbe_phy_info;

/**
 *  Intelgbe_init_mac_ops_generic- Init MAC func ptrs.
 *  @hw: pointer to the HW structure
 **/

s32 intelgbe_mdio_write(struct intelgbe_hw *hw, UINT32 addr, UINT8 dev, UINT16 reg, UINT16 data,
  bool c45);
s32 intelgbe_mdio_read(struct intelgbe_hw *hw, UINT32 addr, UINT8 dev, UINT16 reg, UINT32 *data,
  bool c45);
s32 intelgbe_init_mac_ops(struct intelgbe_hw *hw);
s32 intelgbe_init_phy_ops_maxlinear_gpyxxx(struct intelgbe_hw *);
s32 intelgbe_init_phy_ops_marvell_88e1512(struct intelgbe_hw *);
s32 intelgbe_init_phy_ops_marvell_88e2110(struct intelgbe_hw *);
s32 intelgbe_init_phy_params_stmmac(struct intelgbe_hw *hw);
void intelgbe_init_function_pointers_stmmac(struct intelgbe_hw *hw);
s32 intelgbe_xpcs_init(struct intelgbe_hw *hw);
s32 intelgbe_modphy_init(struct intelgbe_hw *hw);

s32 mii_phy_id_get(struct intelgbe_hw *hw);
int mii_phy_soft_reset(struct intelgbe_hw *hw, bool wait);
int mii_phy_config_link(struct intelgbe_hw *hw, bool changed);
int mii_phy_get_supported(struct intelgbe_hw *hw);

#endif
