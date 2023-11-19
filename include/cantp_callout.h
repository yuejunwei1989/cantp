#ifndef CANTP_CALLOUT_H
#define CANTP_CALLOUT_H

#include "cantp.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void canif_transmit(uint8_t tx_sdu_id, uint8_t* sdu_data_ptr, uint16_t sdu_length);
extern void dcm_rx_indication(uint8_t length, uint8_t* data);

#ifdef __cplusplus
}
#endif
#endif
