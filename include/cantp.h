#ifndef CANTP_H
#define CANTP_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
  bool is_canfd;
  uint16_t canid_tx;
  uint16_t canid_rx;
  uint16_t stmin;
  uint16_t n_as;
  uint16_t n_ar;
  uint16_t n_bs;
  uint16_t n_br;
  uint16_t n_cs;
  uint16_t n_cr;
} cantp_channel_config_t;

typedef struct
{
	cantp_channel_config_t cantp_channel_config[2];
}cantp_config_t;

#ifdef __cplusplus
extern "C" {
#endif
extern void cantp_init(cantp_config_t* cantp_config_ptr);
extern void cantp_transmit(uint8_t tx_sdu_id, uint8_t* sdu_data_ptr, uint16_t sdu_length);
extern void cantp_main_function(void);
void cantp_rx_indication(uint8_t rx_pdu_id, uint8_t* sdu_data_ptr, uint16_t sdu_length);
extern void cantp_tx_confirmation(uint8_t tx_pdu_id);
#ifdef __cplusplus
}
#endif
#endif
