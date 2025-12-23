#pragma once
#include <acpi.h>
#include <common.h>

/** Reference
    https://uefi.org/sites/default/files/resources/CSRT%20v2.pdf

    Csrt: Core System Resource Table
    Structure:
        +----------------+
        |      CSRT      |
        +----------------+
        |  ACPI Header   |
        +----------------+
        | Resource Group | -------> +----------------+
        +----------------+          | Resource Group |
        | Resource Group |          |     Header     |
        +----------------+          +----------------+
        |      ***       |          | Resource Group | -+
        +----------------+          |   Shared Info  |  |
        | Resource Group |          +----------------+  |
        +----------------+                              |
                                +----------------+ <----+
                                | Resource Desc. |
                                +----------------+
                                | Silicon-vendor |
                                |  Defined info  |
                                +----------------+
*/

/* ACPI Header */
#define ACPI_CSRT_SIGNATURE 'C', 'S', 'R', 'T'
#define ACPI_CSRT_REVISION 0

#define ACPI_CSRT_TABLE_STRUCTURE_NAME CORE_SYSTEM_RESOURCE_TABLE

/* Body Structures */
// Resource Groups Header Format
typedef struct {
  UINT32 Length;      // Length of this resource group, including this header
  UINT32 VendorId;    // Vendor Identifier, little-endian orider char array
  UINT32 SubVendorId; // SubVendor Identifier, little-endian order char array
  UINT16 DeviceId;    // Device ID
  UINT16 SubDeviceId; // SubDevice ID
  UINT16 Revision;    // Revision ID
  UINT16 Reserved;    // Must be zero
  UINT32 SharedInfoLength; // Length of Shared Info Structure that follows
  // UINT8 ResourceGroupSharedInfo[/* SharedInfoLength */]; // Variable length
} __attribute__((packed)) CSRT_RESOURCE_GROUPS_HEADER_FORMAT;
_Static_assert(sizeof(CSRT_RESOURCE_GROUPS_HEADER_FORMAT) == 24,
               "CSRT_RESOURCE_GROUPS_HEADER_FORMAT size incorrect");

// Resource descriptor format
typedef struct {
  UINT32 Length; // Length of this resource descriptor, including this header
  UINT16 ResourceType;    // Resource Type
  UINT16 ResourceSubType; // Resource Subtype
  UINT32 UID;             // 32-bit resource identifier.
  //  UINT8 SiliconVendorDefinedInfo[/* Length - 12 */]; // Variable length
} __attribute__((packed)) CSRT_RESOURCE_DESCRIPTOR_FORMAT;
_Static_assert(sizeof(CSRT_RESOURCE_DESCRIPTOR_FORMAT) == 12,
               "CSRT_RESOURCE_DESCRIPTOR_FORMAT size incorrect");

// CSRT Resource Types and Subtypes
enum CSRT_RESOURCE_TYPE {
  CSRT_RESOURCE_TYPE_RESERVED = 0,
  CSRT_RESOURCE_TYPE_INTERRUPT = 1,
  CSRT_RESOURCE_TYPE_TIMER = 2,
  CSRT_RESOURCE_TYPE_DMA = 3,
  CSRT_RESOURCE_TYPE_PLATFORM_SECURITY = 4,
  CSRT_RESOURCE_TYPE_RESERVED2 = 0x7FF, /* 0x0005 ~ 0x07FF */
  CSRT_RESOURCE_TYPE_RESERVED3 = 0xFFFF /* 0x0800 ~ 0xFFFF */
};
// CSRT Resource Subtypes
enum CSRT_RESOURCE_INTERRUPT_SUBTYPE {
  CSRT_RESOURCE_INTERRUPT_SUBTYPE_INTERRUPT_LINE = 0,
  CSRT_RESOURCE_INTERRUPT_SUBTYPE_INTERRUPT_CONTROLLER = 1
};

enum CSRT_RESOURCE_TIMER_SUBTYPE { CSRT_RESOURCE_TIMER_SUBTYPE_TIMER = 0 };

enum CSRT_RESOURCE_DMA_SUBTYPE {
  CSRT_RESOURCE_DMA_SUBTYPE_DMA_CHANNEL = 0,
  CSRT_RESOURCE_DMA_SUBTYPE_DMA_CONTROLLER = 1
};

enum CSRT_RESOURCE_PLATFORM_SECURITY_SUBTYPE {
  CSRT_RESOURCE_PLATFORM_SECURITY_SUBTYPE_RESERVED = 0,
  CSRT_RESOURCE_PLATFORM_SECURITY_SUBTYPE_PLATFORM_SECURITY = 1,
  CSRT_RESOURCE_PLATFORM_SECURITY_SUBTYPE_DRAM_ENCRYPTION = 2
};

#define CSRT_DEFINE_RESOURCE_GROUP(suffix, shared_info_len,                    \
                                   vendor_defined_info_len)                    \
  typedef struct {                                                             \
    CSRT_RESOURCE_GROUPS_HEADER_FORMAT Header;                                 \
    UINT8 ResourceGroupSharedInfo[shared_info_len];                            \
    CSRT_RESOURCE_DESCRIPTOR_FORMAT Resource;                                  \
    UINT8 SiliconVendorDefinedInfo[vendor_defined_info_len -                   \
                                   sizeof(CSRT_RESOURCE_DESCRIPTOR_FORMAT)];   \
  } __attribute__((packed)) CSRT_RESOURCE_GROUP_##suffix;

#define CSRT_RESOURCE_GROUP_STRUCTURE_NAME(suffix) CSRT_RESOURCE_GROUP_##suffix
#define CSRT_TABLE_DEFINE_RESOURCE_GROUP(suffix)                               \
  CSRT_RESOURCE_GROUP_STRUCTURE_NAME(suffix) suffix;

/* Helper macro fill data */
#define CSRT_DECLARE_RG_HEADER(suffix, vid, svid, did, sdid, rev, si_len)      \
  .Header = {                                                                  \
      .Length = sizeof(CSRT_RESOURCE_GROUP_STRUCTURE_NAME(suffix)),            \
      .VendorId = vid,                                                         \
      .SubVendorId = svid,                                                     \
      .DeviceId = did,                                                         \
      .SubDeviceId = sdid,                                                     \
      .Revision = rev,                                                         \
      .Reserved = 0,                                                           \
      .SharedInfoLength = si_len,                                              \
  }

#define CSRT_DECLARE_RD_HEADER(vendor_defined_info_len, type, subtype, uid)    \
  .Resource = {                                                                \
      .Length = vendor_defined_info_len,                                       \
      .ResourceType = type,                                                    \
      .ResourceSubType = subtype,                                              \
      .UID = uid,                                                              \
  }

#define CSRT_DECLARE_RD_RESOURCE_INFO(...)                                     \
  .SiliconVendorDefinedInfo = {__VA_ARGS__}

#define CSRT_DECLARE_RG(suffix, vid, svid, did, sdid, rev, si_len,             \
                        vendor_defined_info_len, type, subtype, uid, ...)      \
  .suffix = {                                                                  \
      /* Header */                                                             \
      CSRT_DECLARE_RG_HEADER(suffix, vid, svid, did, sdid, rev,                \
                             si_len), /* Ignore Shared Info*/                  \
      CSRT_DECLARE_RD_HEADER(vendor_defined_info_len, type, subtype, uid),     \
      CSRT_DECLARE_RD_RESOURCE_INFO(__VA_ARGS__),                              \
  }

#define CSRT_DEFINE_TABLE(...)                                                 \
  typedef struct {                                                             \
    ACPI_TABLE_HEADER Header;                                                  \
    __VA_OPT__(__VA_ARGS__)                                                    \
  } __attribute__((packed)) ACPI_CSRT_TABLE_STRUCTURE_NAME;

#define CSRT_DECLARE_HEADER                                                    \
  ACPI_DECLARE_TABLE_HEADER(                                                   \
      ACPI_CSRT_SIGNATURE, ACPI_CSRT_TABLE_STRUCTURE_NAME, ACPI_CSRT_REVISION)

/* CSRT Table with Magic */
#define CSRT_DEFINE_WITH_MAGIC                                                 \
  ACPI_TABLE_WITH_MAGIC(ACPI_CSRT_TABLE_STRUCTURE_NAME)

#define CSRT_START ACPI_TABLE_START(ACPI_CSRT_TABLE_STRUCTURE_NAME)
#define CSRT_END ACPI_TABLE_END(ACPI_CSRT_TABLE_STRUCTURE_NAME)