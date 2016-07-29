

#include "ssl_msg.h"
#include "errno.h"

/**
 *
 */
#define SSL_SET_MSG_TYPE(pbuf, type) pbuf[0] = type

#define SSL_SET_MEG_BODY_BYTES(pbuf, bytes) \
            pbuf[1] = (uint8_t)(bytes >> 16); \
            pbuf[2] = (uint8_t)(bytes >>  8); \
            pbuf[3] = (uint8_t)(bytes >>  0);

#define SSL_SET_PROTOCOL_VERSION(pbuf, ver)
/*************************************************/

INLINE err_t __ssl_set_msg_payload(ssl_t *ssl,
		                           char *pbuf,
		                           size_t buf_len)
{

	switch (pbuf[0]) {
		case HELLO_REQUEST :
		case CLIENT_HELLO :
		case SERVER_HELLO :
		case CERTIFICATE :
		case SERVER_KEY_EXCHANGE :
		case CERTIFICATE_REQUEST :
		case SERVER_HELLO_DONE :
		case CERTIFICATE_VERIFY :
		case CLIENT_KEY_EXCHANGE :
		case FINISH :
		default :
			break;
	}
}

/**
 *
 */
INLINE err_t __ssl_set_msg(ssl_t *ssl,
						   ssl_msg_t msg_type,
						   char *pbuf,
						   size_t buf_len)
{
	int payload_bytes;

	SSL_SET_MSG_TYPE(pbuf, msg_type);

	payload_bytes = __ssl_set_msg_payload(ssl, pbuf, buf_len);

	if (payload_bytes > 0)
	{
		SSL_SET_MEG_BODY_BYTES(pbuf, payload_bytes);
	}
	else
	{
		payload_bytes = -ENOMEM;
		goto set_payload_error;
	}

	payload_bytes += SSL_MSG_HEAD_BYTES;

set_payload_error:

	return payload_bytes;
}
