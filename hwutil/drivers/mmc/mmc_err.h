#ifndef _MMC_ERR_H_
#define _MMC_ERR_H_

#include "rtos.h"

/* mmc commad request responses error */
#define  SD_RESPONSE_NO_ERROR                   (0x00UL)
#define  SD_IN_IDLE_STATE                       (0x01UL)
#define  SD_ERASE_RESET                         (0x02UL)
#define  SD_ILLEGAL_COMMAND                     (0x04UL)
#define  SD_COM_CRC_ERROR                       (0x08UL)
#define  SD_ERASE_SEQUENCE_ERROR                (0x10UL)
#define  SD_ADDRESS_ERROR                       (0x20UL)
#define  SD_PARAMETER_ERROR                     (0x40UL)
#define  SD_RESPONSE_FAILURE                    (0xFFUL)

/* mmc data request responses error */
#define  SD_DATA_OK                             (0x05UL)
#define  SD_DATA_CRC_ERROR                      (0x0BUL)
#define  SD_DATA_WRITE_ERROR                    (0x0DUL)
#define  SD_DATA_OTHER_ERROR                    (0xFFUL)

typedef err_t                                   sd_err;

#endif
