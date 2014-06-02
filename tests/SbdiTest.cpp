/*
 * SbdiBLockLayerTest.cpp
 *
 *  Created on: May 21, 2014
 *      Author: dhein
 */

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

class SbdiTest: public CppUnit::TestFixture {
CPPUNIT_TEST_SUITE( SbdiTest );
  CPPUNIT_TEST(testSimpleReadWrite);CPPUNIT_TEST_SUITE_END()
  ;

private:
  static unsigned char SIV_KEYS[32];
  sbdi_t *sbdi;
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
    CPPUNIT_ASSERT(sbdi_open(&sbdi, pio, SIV_KEYS, root) == SBDI_SUCCESS);
  }

  void closeStore()
  {
    CPPUNIT_ASSERT(sbdi_close(sbdi, SIV_KEYS, root) == SBDI_SUCCESS);
    int fd = *(int *) pio->iod;
    CPPUNIT_ASSERT(close(fd) != -1);
    sbdi_pio_delete(pio);
  }

  void deleteStore()
  {
    memset(root, 0, sizeof(mt_hash_t));
    CPPUNIT_ASSERT(unlink(FILE_NAME) != -1);
  }

  void read(unsigned char *buf, size_t len, off_t off)
  {
    off_t rd = 0;
    sbdi_error_t r = sbdi_pread(&rd, sbdi, buf, len, off);
    if (r != SBDI_SUCCESS) {
      std::cout << "Reading file @ offset " << off << ". Error: "
          << err_to_string(r) << std::endl;
    }
    CPPUNIT_ASSERT(r == SBDI_SUCCESS);
  }

  void write(unsigned char *buf, size_t len, off_t off)
  {
    off_t wr = 0;
    sbdi_error_t r = sbdi_pwrite(&wr, sbdi, buf, len, off);
    if (r != SBDI_SUCCESS) {
      std::cout << "Writing file @ offset " << off << ". Error: "
          << err_to_string(r) << std::endl;
    }
    CPPUNIT_ASSERT(r == SBDI_SUCCESS);
  }

  void fill(uint32_t c, unsigned char *buf, size_t len)
  {
    CPPUNIT_ASSERT(c <= UINT8_MAX);
    for (size_t i = 0; i < len; ++i, ++c) {
      buf[i] = c % UINT8_MAX;
    }
  }

  void cmp(uint32_t c, unsigned char *buf, size_t len)
  {
    CPPUNIT_ASSERT(c <= UINT8_MAX);
    for (size_t i = 0; i < len; ++i, ++c) {
      if (buf[i] != c % UINT8_MAX) {
        std::cout << "Comparison @ offset " << i << " fails." << std::endl;
      }
      CPPUNIT_ASSERT(buf[i] == c % UINT8_MAX);
    }
  }

  void f_write(uint32_t v, unsigned char *buf, size_t len, off_t off)
  {
    fill(v, buf, len);
    write(buf, len, off);
  }

  void c_read(uint32_t v, unsigned char *buf, size_t len, off_t off)
  {
    memset(buf, 0xFF, len);
    read(buf, len, off);
    cmp(v, buf, len);
  }

public:
  void setUp()
  {
    unlink(FILE_NAME);
    memset(root, 0, sizeof(mt_hash_t));
  }

  void tearDown()
  {

  }

  void testSimpleReadWrite()
  {
    loadStore();
    unsigned char *b = (unsigned char *)malloc(sizeof(unsigned char) * 1024 * 1024);
    f_write(17, b, 5 * 1024, 34);
    c_read(17, b, 5 * 1024, 34);
    CPPUNIT_ASSERT(sbdi_bc_sync(sbdi->cache) == SBDI_SUCCESS);
    closeStore();
    loadStore();
    c_read(17, b, 5 * 1024, 34);
    closeStore();
    deleteStore();
  }

};

unsigned char SbdiTest::SIV_KEYS[32] = {
    // Part 1: fffefdfc fbfaf9f8 f7f6f5f4 f3f2f1f0
    0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0xf7, 0xf6, 0xf5, 0xf4,
    0xf3, 0xf2, 0xf1, 0xf0,
    // Part 2: f0f1f2f3 f4f5f6f7 f8f9fafb fcfdfeff
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb,
    0xfc, 0xfd, 0xfe, 0xff };

CPPUNIT_TEST_SUITE_REGISTRATION(SbdiTest);