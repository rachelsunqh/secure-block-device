# Secure Block Device Library

## 1. Introduction

The Secure Block Device Library is a software library that applies
cryptographic confidentiality and integrity protection, including data
freshness, to arbitrary block device like storage mechanisms. To take advantage
of the Secure Block Device Library you need the ability to securely store a
cryptographic key and a root hash value.  

The Secure Block Device Library API allows reading, and writing arbitrary sized
byte buffers, at arbitrary offsets, to and from a data store wrapped by the
Secure Block Device Library. The Secure Block Device Library applies a
protection mechanisms based on a selectable [Authenticating
Encryption](https://en.wikipedia.org/wiki/Authenticated_encryption) scheme, the
[CMAC](https://en.wikipedia.org/wiki/CMAC) message authentication code, and a
[Merkle-Tree](https://en.wikipedia.org/wiki/Merkle_tree) (SHA-256) to achieve
confidentiality and integrity. The Secure Block Device Library requires its
user to protect a master key and an integrity value between using the Secure
Block Device Library. The Secure Block Device Library derives two internal keys
form the master key at Secure Block Device Library creation, using the [SIV
S2V](https://tools.ietf.org/html/rfc5297) pseudo random function construction.
These keys are the Authenticating Encryption key and the MAC key, and they are
stored in the Secure Block Device Library header encrypted under the master
key, when at rest. The Secure Block Device Library splits incoming read and
write requests into data blocks of a fixed size (Secure Block Device block size
is a compile time parameter) to allow scalable random access. 

### 1.1 Frequently Asked Questions

#### 1.1.1 For what purpose can I use the Secure Block Device Library?

To protect sensitive Data-at-Rest, i.e. data that is stored on a non-volatile
storage, such as a hard disk, against data theft and data modification. For
example you have an application that operates on sensitive data. This
sensitive data is stored in a number of files on an untrusted storage, for
example a network share, or a cloud drive. Now instead of accessing these
files directly, you access them through the Secure Block Device Library, which
encrypts the files and protects them against modification.

#### 1.1.2 What are the requirements for the Secure Block Device Library?

To really take advantage of the Secure Block Device Library you need the
ability to store two fixed size pieces of data in a trusted storage per back
end data store (e.g. a file). These two pieces of data are a symmetric
cryptographic key and a root hash.

For example you have an application that uses an untrusted storage, e.g.
network share or cloud drive, to store the bulk of its data, but your
application also has access to trusted local storage, e.g. a hard drive. In
this scenario you can use the Secure Block Device Library to store data on the
untrusted storage, while storing the cryptographic key and root hash locally. 

#### 1.1.3 Is the Secure Block Device Library a file system?

No! The Secure Block Device Library wraps a block device like back end
storage, for example a file, and provides cryptographic data protection on
this file. The Secure Block Device Library does retain fast random access,
i.e. you can access any data in it randomly, but it does not provide a file
system. In fact, a back end storage accessed via the Secure Block Device
Library behaves very much like the POSIX file abstraction. You can open,
close, read, write, pread and pwrite data and the Secure Block Device Library
will take care of protecting this data against theft and undetected modification.

#### 1.1.4 What are the features of the Secure Block Device Library?

##### 1.1.4.1 Security

The Secure Block Device Library uses [Authenticating
Encryption](https://en.wikipedia.org/wiki/Authenticated_encryption), the
[CMAC](https://en.wikipedia.org/wiki/CMAC) message authentication code, and a
Merkle tree (hash tree) to protect data confidentiality and integrity while
retaining fast random access. In other words, no one but the holder of the
cryptographic key should be able to read the protected data. Furthermore, no
one, but the holder of the cryptographic key and root hash should be able to
tamper with the data undetected. 

##### 1.1.4.2 Efficiency

The Secure Block Device Library has three efficiency mechanisms. First, it
protects data integrity using a [Merkle tree (hash
tree)](https://en.wikipedia.org/wiki/Merkle_tree), allowing for random data
access. Second, it has an integrated cache that holds unencrypted data in
memory, because data encryption accounts for approximately 90% of the
computation time spent by the Secure Block Device Library. Third, it allows
selecting which authenticating encryption scheme to use. Currently, the Secure
Block Device Library supports the use of the [AES
OCB](https://en.wikipedia.org/wiki/OCB_mode) and [AES
SIV](https://tools.ietf.org/html/rfc5297) authenticating encryption schemes.

Also in agreement with the license of the AES SIV implementation we use:

This product includes software written by Dan Harkins (dharkins at lounge dot org) 

#### 1.1.4.3 Extensibility

The Secure Block Device Library allows adding support for new authenticating
encryption schemes via its Cryptography Abstraction Layer, as well as adding
support for new kinds of back end stores through its Block Device Abstraction
Layer.

#### 1.1.5 Why did you develop the Secure Block Device Library

Actually, the Secure Block Device Library was developed for [ANDIX
OS](http://andix.iaik.tugraz.at/sbd/) to provide cryptographically protected
storage for Trusted Applications.

#### 1.1.6 Is the Secure Block Device Library threat safe?

No! Also don't try to access the same back end storage, e.g. the file, through
two different Secure Block Device instances! We have no idea what would
happen, but integrity violations are very probable.

### 1.2. Further Reading

If you are interested, and have access to the IEEE Xplore(R) Digital Library,
there is a paper [1], which has been presented at the 14th IEEE International
Conference on Trust, Security and Privacy in Computing and Communications
(IEEE TrustCom-15). Currently, the paper is not yet online.

## 2. Building the Secure Block Device Library

The library build system is based on Make. We currently do not support a
configure script, if you want to adapt the library to your needs, adapt the
'src/config.h' header file by yourself.

### 2.1 Configuration Options

#### 2.1.1. SBDI_BLOCK_SIZE

The Secure Block Device Library logically splits the back end storage into
blocks. Each block is individually encrypted and integrity protected. A block
is the smallest amount of data the Secure Block Device Library can access. For
example, even if you just want to read or write a single byte of information,
the Secure Block Device Library needs to read or write a whole block of
SBDI_BLOCK_SIZE bytes. For each block the Secure Block Device Library
maintains 32 bytes of overhead data (a cryptographic nonce and an authenticity
tag). So the smaller the block size, the faster the access (at least within
certain limits), but the more overhead.

#### 2.1.2 SBDI_SIZE_MAX

The maximum size of an individual Secure Block Device. We have developed the
Secure Block Device Library with 32-bit systems in mind, therefore we limited
the max size to 2147483647 bytes. If you go beyond this number, you reach
untested territories. You have been warned.

#### 2.1.3 SBDI_CACHE_MAX_SIZE

The Secure Block Device Library maintains an individual in-memory cache for
each Secure Block Device instance. This flag sets the size of the cache in
blocks. The cache is a write-back, write-allocate cache and stores data in
unencrypted form, and thus greatly speeds up access.

### 2.2 Dependencies

Merkle Tree Library - The Secure Block Device Library requires the Merkle Tree
Library. The Merkle Tree Library should be part of the source tarball, and the
Make files setup in a way that if you first 'make' the Merkle Tree Library
then 'making' the Secure Block Device Library should work.

CppUnit - We provide a small set of unit test cases using CppUnit. For
building and running the tests CppUnit is a dependency.

Valgrind - By default the test suite is run with Valgrind's memcheck tool to
help detect memory leaks. Unless use of Valgrind is deactivated, it needs to
be installed.

Doxygen - The library is (sparsely) documented. Doxygen is required to create
the documentation.

### 2.2 Building the library

Untar the source, first change into the Merkle Tree Library root directory and
build it, then change into the Secure Block Device Library's root directory,
and build that.  Supported targets are:

* debug    - build with debug information enabled
* coverage - build with debugging and coverage support
* release  - build optimzed (-O3, no debug) release version
* doc      - build the documentation (as it is) using Doxygen 
* test     - run the CppUnit based test suite. Call with 'make VGRUN= test' to
             deactivate Valgrind. 
* clean    - clean up build artifacts

All successful builds create two static libraries, for linking to other
applications. First, 'libSecureBlock.a' in the 'src' directory, and second,
'libSbdiCrypto.a' in the 'src/crypto' directory. In addition any application
using the Secure Block Device Library will also require the 'libMerkleTree.a'
from the Merkle Tree Library.

### 2.3 Comments

This library has so far been tested on ARM (32-bit) and AMD64. 

## 3. Using the Secure Block Device Library

The library's user interface is specified in
'src/SecureBlockDeviceInterface.h'. The 'tests/SbdiTests.cpp' outlines how to
use the library. The back end storage the Secure Block Device Library uses is
abstracted via the Block Device Abstraction Layer.

The Secure Block Device Library comes with a wrapper for files. Typically, a
new Secure Block Device instance is created by calling sbdi_open() with a
concrete Block Device Abstraction Layer instance, a cryptographic root key, a
root hash, and the type of authenticated encryption to use. The instance has
to be destroyed by a subsequent call to sbdi_close(). The sbdi_close()
function also requires the cryptographic root key (to re-encrypt the updated
Secure Block Device header), and the root hash, this time as out parameter.
The root hash is a single representative of the overall integrity of the
Secure Block Device and has to be kept safe until the next opening of the same
Secure Block Device. The Secure Block Device can be used between a call to
sbdi_open() and its corresponding call to sbdi_close(). The rest of the Secure
Block Device Library's API is modelled after the POSIX file abstraction. So,
sbdi_read(), sbdi_write(), sbdi_lseek(), sbdi_pread(), sbdi_pwrite(),
sbdi_sync(), and sbdi_fsync() work similar to their POSIX read, write, ...
counterparts.

## 4. Licensing

For details on the licensing of the Secure Block Device Library see LICENSE. 

## 5. Bibliography
* [1] Daniel Hein, Johannes Winter, and Andreas Fitzek, "Secure Block Device -
      Secure, Flexible, and Efficient Data Storage for ARM TrustZone Systems,"
	  	in TrustCom, 2015.
* https://en.wikipedia.org/wiki/Authenticated_encryption
* https://en.wikipedia.org/wiki/CMAC
* https://en.wikipedia.org/wiki/Merkle_tree
* https://en.wikipedia.org/wiki/OCB_mode
* https://tools.ietf.org/html/rfc5297
* http://andix.iaik.tugraz.at/sbd/
