/*
 * sbdi_block.h
 *
 *  Created on: May 18, 2014
 *      Author: dhein
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SBDI_BLOCK_H_
#define SBDI_BLOCK_H_

#include "sbdi_config.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t sbdi_db_t[SBDI_BLOCK_SIZE];

typedef struct sbdi_block {
  uint32_t idx;
  sbdi_db_t *data;
} sbdi_block_t;

static inline sbdi_error_t sbdi_block_init(sbdi_block_t *blk, uint32_t blk_idx,
    sbdi_db_t *blk_data)
{
  if (!blk) {
    return SBDI_ERR_ILLEGAL_PARAM;
  }
  blk->idx = blk_idx;
  blk->data = blk_data;
  return SBDI_SUCCESS;
}

static inline sbdi_error_t sbdi_block_invalidate(sbdi_block_t *blk)
{
  if (!blk) {
    return SBDI_ERR_ILLEGAL_PARAM;
  }
  blk->idx = UINT32_MAX;
  blk->data = NULL;
  return SBDI_SUCCESS;
}

static inline uint32_t sbdi_get_mngt_block_number(uint32_t idx)
{
  return idx / SBDI_MNGT_BLOCK_ENTRIES;
}

static inline uint32_t sbdi_get_mngt_block_index(uint32_t idx)
{
  return (sbdi_get_mngt_block_number(idx) * SBDI_MNGT_BLOCK_ENTRIES)
      + sbdi_get_mngt_block_number(idx) + 1;
}

static inline uint32_t sbdi_get_data_block_index(uint32_t idx)
{
  return idx + sbdi_get_mngt_block_number(idx) + 2;
}

static inline uint32_t sbdi_get_mngt_tag_index(uint32_t idx)
{
  return idx % SBDI_MNGT_BLOCK_ENTRIES;
}

static inline uint32_t sbdi_bl_idx_phy_to_log(uint32_t phy)
{
  if (phy < 2) {
    // TODO Error handling?
    return UINT32_MAX;
  }
  return (phy - 2) - (phy - 2) / (SBDI_MNGT_BLOCK_ENTRIES + 1);
}

static inline uint32_t sbdi_bl_idx_phy_to_mng(uint32_t phy)
{
  assert(phy > 1);
  uint32_t tmp = (phy - 2) / (SBDI_MNGT_BLOCK_ENTRIES + 1);
  return tmp * (SBDI_MNGT_BLOCK_ENTRIES + 1) + 1;
}

static inline int sbdi_bl_is_phy_mng(uint32_t phy) {
  assert(phy > 0);
  return ((phy - 1) % (SBDI_MNGT_BLOCK_ENTRIES + 1)) == 0;
}

static inline uint32_t sbdi_bl_mng_phy_to_mng_log(uint32_t mng_phy)
{
  assert(mng_phy > 0);
  assert(sbdi_bl_is_phy_mng(mng_phy));
  return (mng_phy - 1) / (SBDI_MNGT_BLOCK_ENTRIES + 1);
}

#endif /* SBDI_BLOCK_H_ */

#ifdef __cplusplus
}
#endif
