// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __bcd_version_h__
#define __bcd_version_h__

#define BCD_VERSION(major, minor, patch) \
  ((((major) & 0xFF) << 8) | (((minor) & 0xF) << 4) | ((patch) & 0xF))

#endif
