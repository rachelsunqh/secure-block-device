/* Copyright (c) IAIK, Graz University of Technology, 2015.
 * All rights reserved.
 * Contact: http://opensource.iaik.tugraz.at
 * 
 * This file is part of the Secure Block Device Library.
 * 
 * Commercial License Usage
 * Licensees holding valid commercial licenses may use this file in
 * accordance with the commercial license agreement provided with the
 * Software or, alternatively, in accordance with the terms contained in
 * a written agreement between you and SIC. For further information
 * contact us at http://opensource.iaik.tugraz.at.
 * 
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License as published by the Free Software Foundation version 2.
 * 
 * The Secure Block Device Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with the Secure Block Device Library. If not, see <http://www.gnu.org/licenses/>.
 */
///
/// \file
/// \brief Tests the Secure Block Device Library's block layer.
///
/// The block layer handles all data operations on block granularity. Together
/// with the cache it implements the "plumbing" of the SBD: reading/writing and
/// protecting/checking data blocks.
///

#ifndef UINT32_MAX
#include <limits>
#define UINT32_MAX std::numeric_limits<uint32_t>::max()
#endif
#ifndef UINT32_C
#define UINT32_C(c) c ## u
#endif
#ifndef UINT16_MAX
#define UINT16_MAX 65535u
#endif
#ifndef UINT8_MAX
#define UINT8_MAX 255u
#endif

#include "SbdiTest.h"
#include "SecureBlockDeviceInterface.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <cppunit/extensions/HelperMacros.h>

class SbdiBLockLayerTest: public CppUnit::TestFixture {
CPPUNIT_TEST_SUITE( SbdiBLockLayerTest );
  CPPUNIT_TEST(testIndexComp);
  CPPUNIT_TEST(testSimpleReadWrite);
  CPPUNIT_TEST(testSimpleIntegrityCheck);
  CPPUNIT_TEST(testExtendedReadWrite);
  CPPUNIT_TEST(testLinearReadWrite);CPPUNIT_TEST_SUITE_END()
  ;

private:
  static unsigned char SIV_KEYS[32];
  sbdi_t *sbdi;
  unsigned char b[SBDI_BLOCK_SIZE];
  mt_hash_t root;
  int fd;
  sbdi_pio_t *pio;

  void loadStore()
  {
    fd = open(FILE_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    CPPUNIT_ASSERT(fd != -1);
    struct stat s;
    CPPUNIT_ASSERT(fstat(fd, &s) == 0);
    pio = sbdi_pio_create(&fd, s.st_size);
    CPPUNIT_ASSERT(sbdi_open(&sbdi, pio, SBDI_CRYPTO_NONE, SIV_KEYS, root) == SBDI_SUCCESS);
  }

  void closeStore()
  {
    CPPUNIT_ASSERT(sbdi_close(sbdi, SIV_KEYS, root) == SBDI_SUCCESS);
    int fd = *(int *) pio->iod;
    CPPUNIT_ASSERT(close(fd) != -1);
    sbdi_pio_delete (pio);
  }

  void deleteStore()
  {
    memset(root, 0, sizeof(mt_hash_t));
    CPPUNIT_ASSERT(unlink(FILE_NAME) != -1);
  }

  void read(uint32_t i)
  {
    sbdi_error_t r = sbdi_bl_read_data_block(sbdi, b, i, 0, SBDI_BLOCK_SIZE);
    if (r != SBDI_SUCCESS) {
      std::cout << "Reading file @ block " << i << ". Error: "
          << err_to_string(r) << std::endl;
    }
    CPPUNIT_ASSERT(r == SBDI_SUCCESS);
  }

  void write(uint32_t i)
  {
    sbdi_error_t r = sbdi_bl_write_data_block(sbdi, b, i, 0, SBDI_BLOCK_SIZE);
    if (r != SBDI_SUCCESS) {
      std::cout << "Writing file @ block " << i << ". Error: "
          << err_to_string(r) << std::endl;

    }
    CPPUNIT_ASSERT(r == SBDI_SUCCESS);
  }

  void fill(uint32_t c)
  {
    CPPUNIT_ASSERT(c <= UINT8_MAX);
    memset(b, c, SBDI_BLOCK_SIZE);
  }

  void f_write(uint32_t i, uint32_t v)
  {
    fill(v);
    write(i);
  }

  void cmp(uint32_t c)
  {
    CPPUNIT_ASSERT(c <= UINT8_MAX);
    CPPUNIT_ASSERT(memchrcmp(b, c, SBDI_BLOCK_SIZE));
  }

  void c_read(uint32_t i, uint32_t v)
  {
    fill(0xFF);
    read(i);
    cmp(v);
  }

public:
  void setUp()
  {
    unlink(FILE_NAME);
    memset(b, 0, SBDI_BLOCK_SIZE);
    memset(root, 0, sizeof(mt_hash_t));
  }

  void tearDown()
  {

  }

  void testIndexComp()
  {
    for (uint32_t log_idx = 0; log_idx < SBDI_BLK_MAX_LOG; ++log_idx) {
      uint32_t phy_idx = sbdi_blic_log_to_phy_dat_blk(log_idx);
      if (log_idx != sbdi_blic_phy_dat_to_log(phy_idx)) {
        std::cout << "log: " << log_idx << " phy: " << phy_idx << " phy(log): "
            << sbdi_blic_phy_dat_to_log(phy_idx) << std::endl;
        CPPUNIT_ASSERT(0);
      }
      uint32_t mng_log_idx = sbdi_blic_log_to_phy_mng_blk(log_idx);
      uint32_t mng_phy_idx = sbdi_blic_phy_dat_to_phy_mng_blk(phy_idx);
      if (mng_log_idx != mng_phy_idx) {
        std::cout << "log: " << log_idx << " phy: " << phy_idx << " mng(log): "
            << mng_log_idx << " mng(phy) " << mng_phy_idx << std::endl;
        CPPUNIT_ASSERT(0);
      }
      uint32_t mng_log_blk_nbr = sbdi_blic_log_to_mng_blk_nbr(log_idx);
      uint32_t mng_phy_blk_nbr = sbdi_blic_phy_to_mng_blk_nbr(mng_phy_idx);
      if (mng_log_blk_nbr != mng_phy_blk_nbr) {
        std::cout << "log: " << log_idx << " phy: " << phy_idx
            << " mng_nbr(log): " << mng_log_blk_nbr << " mng_nbr(phy) "
            << mng_phy_blk_nbr << std::endl;
        CPPUNIT_ASSERT(0);
      }
      if (mng_log_idx != sbdi_blic_mng_blk_nbr_to_mng_phy(mng_log_blk_nbr)) {
        std::cout << "log: " << log_idx << " phy: " << phy_idx << " mng_idx_1: "
            << mng_log_idx << " mng_idx_2 "
            << sbdi_blic_mng_blk_nbr_to_mng_phy(mng_log_blk_nbr) << std::endl;
        CPPUNIT_ASSERT(0);
      }
      mng_phy_blk_nbr = sbdi_blic_phy_to_mng_blk_nbr(phy_idx);
      if (mng_phy_blk_nbr != mng_log_blk_nbr) {
        std::cout << "log: " << log_idx << " phy: " << phy_idx
            << " mng_nbr_phy: " << mng_phy_blk_nbr << " mng_nbr_log "
            << mng_log_blk_nbr << std::endl;
        CPPUNIT_ASSERT(0);
      }
    }
  }

  void testSimpleReadWrite()
  {
    loadStore();
    f_write(0, 0x10);
    c_read(0, 0x10);
    f_write(1, 0x11);
    c_read(1, 0x11);
    CPPUNIT_ASSERT(sbdi_bc_sync(sbdi->cache) == SBDI_SUCCESS);
    closeStore();
    loadStore();
    c_read(1, 0x11);
    c_read(0, 0x10);
    closeStore();
    deleteStore();
  }

  void testSimpleIntegrityCheck()
  {
    loadStore();
    f_write(0, 0);
    f_write(128, 128);
    closeStore();
    loadStore();
    c_read(0, 0);
    c_read(128, 128);
    closeStore();
    deleteStore();
  }

  void testExtendedReadWrite()
  {
    loadStore();
    f_write(0x80, 0x80);
    c_read(0x80, 0x80);
    f_write(2049, 0xF0);
    c_read(2049, 0xF0);
    closeStore();
    loadStore();
    c_read(0x80, 0x80);
    c_read(2049, 0xF0);
    closeStore();
    deleteStore();
  }

  void testLinearReadWrite()
  {
    loadStore();
    const int linear_rw_test_size = 4122;
    for (int i = 0; i < linear_rw_test_size; ++i) {
      f_write(i, i % UINT8_MAX);
    }
    for (int i = 0; i < linear_rw_test_size; ++i) {
      c_read(i, i % UINT8_MAX);
    }
    std::cout << std::endl;
    sbdi_bc_print_stats(sbdi->cache);
    closeStore();
    loadStore();
    for (int i = 0; i < linear_rw_test_size; ++i) {
      c_read(i, i % UINT8_MAX);
    }
    std::cout << std::endl;
    sbdi_bc_print_stats(sbdi->cache);
    closeStore();
    deleteStore();
  }

};

unsigned char SbdiBLockLayerTest::SIV_KEYS[32] = {
    // Part 1: fffefdfc fbfaf9f8 f7f6f5f4 f3f2f1f0
    0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0xf7, 0xf6, 0xf5, 0xf4,
    0xf3, 0xf2, 0xf1, 0xf0,
    // Part 2: f0f1f2f3 f4f5f6f7 f8f9fafb fcfdfeff
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb,
    0xfc, 0xfd, 0xfe, 0xff };

CPPUNIT_TEST_SUITE_REGISTRATION(SbdiBLockLayerTest);
