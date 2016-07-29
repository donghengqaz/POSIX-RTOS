#ifndef _IPPKG_H_
#define _IPPKG_H_

#include "types.h"

struct pbuf* ippkg_pack(os_u8 *src, os_u16 len);
size_t ippkg_unpack(struct pbuf *pbuf, os_u8 *src, size_t len);

#endif
