#ifndef _MMC_H_
#define _MMC_H_

#include "mmc_err.h"

typedef struct _SD_CSD
{
  __IO os_u8  CSDStruct;            /*!< CSD structure */
  __IO os_u8  SysSpecVersion;       /*!< System specification version */
  __IO os_u8  Reserved1;            /*!< Reserved */
  __IO os_u8  TAAC;                 /*!< Data read access-time 1 */
  __IO os_u8  NSAC;                 /*!< Data read access-time 2 in CLK cycles */
  __IO os_u8  MaxBusClkFrec;        /*!< Max. bus clock frequency */
  __IO os_u16 CardComdClasses;      /*!< Card command classes */
  __IO os_u8  RdBlockLen;           /*!< Max. read data block length */
  __IO os_u8  PartBlockRead;        /*!< Partial blocks for read allowed */
  __IO os_u8  WrBlockMisalign;      /*!< Write block misalignment */
  __IO os_u8  RdBlockMisalign;      /*!< Read block misalignment */
  __IO os_u8  DSRImpl;              /*!< DSR implemented */
  __IO os_u8  Reserved2;            /*!< Reserved */
  __IO os_u32 DeviceSize;           /*!< Device Size */
  __IO os_u8  MaxRdCurrentVDDMin;   /*!< Max. read current @ VDD min */
  __IO os_u8  MaxRdCurrentVDDMax;   /*!< Max. read current @ VDD max */
  __IO os_u8  MaxWrCurrentVDDMin;   /*!< Max. write current @ VDD min */
  __IO os_u8  MaxWrCurrentVDDMax;   /*!< Max. write current @ VDD max */
  __IO os_u8  DeviceSizeMul;        /*!< Device size multiplier */
  __IO os_u8  EraseGrSize;          /*!< Erase group size */
  __IO os_u8  EraseGrMul;           /*!< Erase group size multiplier */
  __IO os_u8  WrProtectGrSize;      /*!< Write protect group size */
  __IO os_u8  WrProtectGrEnable;    /*!< Write protect group enable */
  __IO os_u8  ManDeflECC;           /*!< Manufacturer default ECC */
  __IO os_u8  WrSpeedFact;          /*!< Write speed factor */
  __IO os_u8  MaxWrBlockLen;        /*!< Max. write data block length */
  __IO os_u8  WriteBlockPaPartial;  /*!< Partial blocks for write allowed */
  __IO os_u8  Reserved3;            /*!< Reserded */
  __IO os_u8  ContentProtectAppli;  /*!< Content protection application */
  __IO os_u8  FileFormatGrouop;     /*!< File format group */
  __IO os_u8  CopyFlag;             /*!< Copy flag (OTP) */
  __IO os_u8  PermWrProtect;        /*!< Permanent write protection */
  __IO os_u8  TempWrProtect;        /*!< Temporary write protection */
  __IO os_u8  FileFormat;           /*!< File Format */
  __IO os_u8  ECC;                  /*!< ECC code */
  __IO os_u8  CSD_CRC;              /*!< CSD CRC */
  __IO os_u8  Reserved4;            /*!< always 1*/
} SD_CSD;

typedef struct _SD_CID
{
  __IO os_u8  ManufacturerID;       /*!< ManufacturerID */
  __IO os_u16 OEM_AppliID;          /*!< OEM/Application ID */
  __IO os_u32 ProdName1;            /*!< Product Name part1 */
  __IO os_u8  ProdName2;            /*!< Product Name part2*/
  __IO os_u8  ProdRev;              /*!< Product Revision */
  __IO os_u32 ProdSN;               /*!< Product Serial Number */
  __IO os_u8  Reserved1;            /*!< Reserved1 */
  __IO os_u16 ManufactDate;         /*!< Manufacturing Date */
  __IO os_u8  CID_CRC;              /*!< CID CRC */
  __IO os_u8  Reserved2;            /*!< always 1 */
} SD_CID;

typedef struct _SD_CardInfo
{
  SD_CSD SD_csd;
  SD_CID SD_cid;
  os_u32 CardCapacity;  /*!< Card Capacity */
  os_u32 CardBlockSize; /*!< Card Block Size */
} sd_card_info_t;

typedef os_u16                      card_type_t;

struct mmc
{
    /* register hardware device port */
    sd_err (*open) (struct mmc *mmc);
  
    void (*select) (void);
    void (*release) (void);
    
    os_u8 (*read_write) (os_u8 byte);
    
    sd_card_info_t    sd_card_info;
    
    card_type_t       card_type;
};
typedef struct mmc mmc_t;


sd_err mmc_open(struct mmc *mmc);

#endif
