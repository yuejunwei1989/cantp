#ifndef CANTP_H
#define CANTP_H

#include <stdint.h>

typedef struct
{
  uint8_t* sdu_data_ptr;
  uint16_t sdu_length;
} pdu_info_t;

#ifdef __cplusplus
extern "C" {
#endif
extern void cantp_init(void);
extern void cantp_transmit(uint8_t tx_sdu_id, const pdu_info_t* tx_info_ptr);
extern void cantp_main_function(void);
extern void cantp_rx_indication(uint8_t rx_pdu_id, const pdu_info_t* rx_info_ptr);
extern void cantp_tx_confirmation(uint8_t tx_pdu_id);
#ifdef __cplusplus
}
#endif
#endif
