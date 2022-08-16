/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _MARVELL_88E2110_DEFINES_H_
#define _MARVELL_88E2110_DEFINES_H_

#include "mii_regs.h"


/* PHY */

#define MARVELL_PHY_ID_88E2110                  0x002B09B9

#define MARVELL10G_PHY_VENDOR                   MII_PHY_VENDOR(MARVELL_PHY_ID_88E2110)
#define MARVELL10G_PHY_MODEL_88E2110            MII_PHY_MODEL(MARVELL_PHY_ID_88E2110)

#endif /* _MARVELL_88E2110_DEFINES_H_ */