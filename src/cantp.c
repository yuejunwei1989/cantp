#include "cantp.h"
#include "cantp_callout.h"
#include <stdio.h>
#include <string.h>

#define CANTP_NPCI_TYPE_SF                     (0x00U)
#define CANTP_NPCI_TYPE_FF                     (0x10U)
#define CANTP_NPCI_TYPE_CF                     (0x20U)
#define CANTP_NPCI_TYPE_FC                     (0x30U)

#define CANTP_STATE_IDLE                       (0x00u)
#define CANTP_STATE_TX_WAIT_SF_CONF            (0x01u)
#define CANTP_STATE_TX_WAIT_FF_CONF            (0x02u)
#define CANTP_STATE_TX_WAIT_FC                 (0x03u)
#define CANTP_STATE_TX_WAIT_CF_CONF            (0x04u)
#define CANTP_STATE_TX_SENDING_CF              (0x05u)
#define CANTP_STATE_RX_TX_FC                   (0x06u)
#define CANTP_STATE_RX_WAIT_FC_CONF            (0x07u)
#define CANTP_STATE_RX_RECEIVING_CF            (0x08u)


#define CANTP_LENGTH_CAN_FRAME  8U
#define CANTP_PADDING_PATTERN 0xAAU

/* Parse 16 bit data in byte stream */
#define GET_HIGH_BYTE(data)               ((uint8_t)(((uint16_t)(data)) >> 8))
#define GET_LOW_BYTE(data)                ((uint8_t)(data))


typedef struct
{
  uint8_t channel_state;
  uint8_t sequence_number;
  uint8_t block_size_counter;
  uint8_t data_index;
  uint8_t data[4095];
  uint16_t payload_length;
  uint16_t data_length;
  uint16_t timer;
} cantp_channel_t;

static cantp_channel_t cantp_channels[2]={0};

static void rcv_ff(cantp_channel_t* channel_ptr, const pdu_info_t* rx_info_ptr)
{
	uint16_t ff_len = 0U;

    if (rx_info_ptr->sdu_length == CANTP_LENGTH_CAN_FRAME)
    {
    	ff_len  = (((uint16_t)(rx_info_ptr->sdu_data_ptr[0] & 0x0F)) << 8);
    	ff_len |=   (uint16_t) rx_info_ptr->sdu_data_ptr[1];

    	memcpy(&channel_ptr->data,
    		   &rx_info_ptr->sdu_data_ptr[2],
    		   6);
    	channel_ptr->channel_state = CANTP_STATE_RX_TX_FC;
    	channel_ptr->sequence_number = 1U;
    	channel_ptr->data_index           = 0U;
        channel_ptr->payload_length = 6;
        channel_ptr->data_index = channel_ptr->data_index + channel_ptr->payload_length;
        channel_ptr->data_length = ff_len;
    }
    else
    {

    }

}

static void rcv_fc(cantp_channel_t* channel_ptr, const pdu_info_t* rx_info_ptr)
{
	switch (channel_ptr->channel_state)
	{
	case CANTP_STATE_TX_WAIT_FC:
		channel_ptr->channel_state = CANTP_STATE_TX_SENDING_CF;
		break;
	default:
		break;
	}
}

static void rcv_cf(cantp_channel_t* channel_ptr, const pdu_info_t* rx_info_ptr)
{
	uint8_t  cf_len = 0U;

	if (((uint8_t)((rx_info_ptr->sdu_data_ptr[0]) & 0x0F)) == channel_ptr->sequence_number) {
		cf_len = rx_info_ptr->sdu_length - 1;

    	memcpy(&channel_ptr->data[channel_ptr->data_index],
    		   &rx_info_ptr->sdu_data_ptr[1],
			   cf_len);

    	channel_ptr->data_index = channel_ptr->data_index + cf_len;

		if((channel_ptr->data_length - channel_ptr->data_index) > 0)
		{
			channel_ptr->channel_state = CANTP_STATE_RX_RECEIVING_CF;
		}
		else
		{
			channel_ptr->channel_state = CANTP_STATE_IDLE;
			dcm_rx_indication(channel_ptr->data_length, channel_ptr->data);

		}

		if (channel_ptr->sequence_number < 0x0F) {
			channel_ptr->sequence_number++;
		}
		else {
			channel_ptr->sequence_number = 0;
		}
	}
	else
	{

	}

}

static void rcv_sf(cantp_channel_t* channel_ptr, const pdu_info_t* rx_info_ptr)
{
	uint8_t	sf_len = 0U;
	uint8_t* sf_data;

	if (rx_info_ptr->sdu_length <= CANTP_LENGTH_CAN_FRAME) {
		sf_len = (uint8_t)((rx_info_ptr->sdu_data_ptr[0]) & 0x0F);
		sf_data = &rx_info_ptr->sdu_data_ptr[1];
	}
	else {
		sf_len = (uint8_t)(rx_info_ptr->sdu_data_ptr[0+1]);
		sf_data = &rx_info_ptr->sdu_data_ptr[2];
	}

	dcm_rx_indication(sf_len, sf_data);
}

void cantp_init(void)
{
}

void cantp_transmit(uint8_t tx_sdu_id, const pdu_info_t* tx_info_ptr)
{
    pdu_info_t       tx_frame;
    uint8_t          tx_frame_buffer[8];
    uint8_t          payload_length;
    cantp_channel_t* channel_ptr;

    channel_ptr = &cantp_channels[tx_sdu_id];
    tx_frame.sdu_data_ptr = tx_frame_buffer;
    tx_frame.sdu_length  = 0U;

    if (tx_info_ptr->sdu_length > 7U)
    {
    	tx_frame.sdu_data_ptr[0]  = CANTP_NPCI_TYPE_FF;
    	tx_frame.sdu_data_ptr[0] |= (uint8_t)(GET_HIGH_BYTE(tx_info_ptr->sdu_length) & 0x0F);
    	tx_frame.sdu_data_ptr[1]  = (uint8_t)(GET_LOW_BYTE(tx_info_ptr->sdu_length));
    	tx_frame.sdu_length += 2U;
    	
    	payload_length = 6U;

        channel_ptr->channel_state = CANTP_STATE_TX_WAIT_FF_CONF;
        channel_ptr->sequence_number = 1U;
        channel_ptr->block_size_counter   = 0U;
        channel_ptr->data_index           = 0U;
        channel_ptr->data_length          = tx_info_ptr->sdu_length;
        //channel_ptr->data          = tx_info_ptr->sdu_data_ptr;
        memcpy((uint8_t*)(&channel_ptr->data),
                          (uint8_t*)(tx_info_ptr->sdu_data_ptr),
						  tx_info_ptr->sdu_length);
        channel_ptr->payload_length = 6;
        channel_ptr->data_index = channel_ptr->data_index + channel_ptr->payload_length;
    }
    else
    {
        tx_frame.sdu_data_ptr[0]  = CANTP_NPCI_TYPE_SF;
        tx_frame.sdu_data_ptr[0] |= (uint8_t)((tx_info_ptr->sdu_length) & 0x0F);
        tx_frame.sdu_length += 1U;

        payload_length = tx_info_ptr->sdu_length;
    }
    memcpy((uint8_t*)(&tx_frame.sdu_data_ptr[tx_frame.sdu_length]),
                      (uint8_t*)( tx_info_ptr->sdu_data_ptr),
					  payload_length);
    tx_frame.sdu_length += payload_length;
    while( tx_frame.sdu_length < CANTP_LENGTH_CAN_FRAME )
    {
    	tx_frame.sdu_data_ptr[tx_frame.sdu_length] = CANTP_PADDING_PATTERN;
    	tx_frame.sdu_length++;
    }
    canif_transmit(0, &tx_frame);
}

void cantp_main_function(void)
{
    uint8_t channel_idx;
	cantp_channel_t* channel_ptr;
    pdu_info_t     tx_frame;
    uint8_t        tx_frame_buffer[8];
    tx_frame.sdu_data_ptr = tx_frame_buffer;
    tx_frame.sdu_length  = 0U;
    for (channel_idx = 0; channel_idx < 0x01; channel_idx++)
    {
        channel_ptr = &cantp_channels[channel_idx];

        if ((CANTP_STATE_IDLE != channel_ptr->channel_state))
        {
			switch(channel_ptr->channel_state)
			{
				case CANTP_STATE_TX_WAIT_SF_CONF:
					break;
				case CANTP_STATE_TX_WAIT_FF_CONF:
					break;
				case CANTP_STATE_TX_WAIT_FC:
					break;
				case CANTP_STATE_TX_WAIT_CF_CONF:
					break;
				case CANTP_STATE_TX_SENDING_CF:
					tx_frame.sdu_data_ptr[0]  = CANTP_NPCI_TYPE_CF;
					tx_frame.sdu_data_ptr[0] |= (uint8_t) (channel_ptr->sequence_number);
					tx_frame.sdu_length += 1U;
					if((channel_ptr->data_length - channel_ptr->data_index) >= 7)
					{
						channel_ptr->payload_length = 7;
					}
					else
					{
						channel_ptr->payload_length = channel_ptr->data_length - channel_ptr->data_index;
					}

					memcpy((uint8_t*)(&tx_frame.sdu_data_ptr[tx_frame.sdu_length]),
									  (uint8_t*)(&channel_ptr->data[channel_ptr->data_index]),
									  channel_ptr->payload_length);
					tx_frame.sdu_length += channel_ptr->payload_length;

				    while( tx_frame.sdu_length < CANTP_LENGTH_CAN_FRAME )
				    {
				    	tx_frame.sdu_data_ptr[tx_frame.sdu_length] = CANTP_PADDING_PATTERN;
				    	tx_frame.sdu_length++;

				    }
					channel_ptr->channel_state = CANTP_STATE_TX_WAIT_CF_CONF;
					channel_ptr->sequence_number  = (uint8_t)((channel_ptr->sequence_number + 1) & 0x0F);
					channel_ptr->data_index = channel_ptr->data_index + channel_ptr->payload_length;

					canif_transmit(0, &tx_frame);
					break;

				case CANTP_STATE_RX_TX_FC:
					tx_frame.sdu_data_ptr[0]  = CANTP_NPCI_TYPE_FC;
					tx_frame.sdu_data_ptr[1]  = 0x00;
					tx_frame.sdu_data_ptr[2]  = 0x00;
					tx_frame.sdu_length = 3U;
					canif_transmit(0, &tx_frame);
					channel_ptr->channel_state = CANTP_STATE_RX_WAIT_FC_CONF;
					break;
				case CANTP_STATE_RX_WAIT_FC_CONF:
					break;
				case CANTP_STATE_RX_RECEIVING_CF:
					break;
				default:
					/*invalid states*/
					break;
			} /*end of switch(channel_ptr->channel_state)*/

		}
	}
}

void cantp_rx_indication(uint8_t rx_pdu_id, const pdu_info_t* rx_info_ptr)
{
	uint8_t	n_pci;
	cantp_channel_t* channel_ptr;
	channel_ptr = &cantp_channels[rx_pdu_id];

    n_pci = rx_info_ptr->sdu_data_ptr[0]&0xF0;
	switch (n_pci) {
	case CANTP_NPCI_TYPE_SF :
		rcv_sf(channel_ptr, rx_info_ptr);
		break;
	case CANTP_NPCI_TYPE_FF :
		rcv_ff(channel_ptr, rx_info_ptr);
		break;
	case CANTP_NPCI_TYPE_CF :
		rcv_cf(channel_ptr, rx_info_ptr);
		break;
	case CANTP_NPCI_TYPE_FC :
		rcv_fc(channel_ptr, rx_info_ptr);
		break;
	default :
		break;
	}
}

void cantp_tx_confirmation(uint8_t tx_pdu_id)
{
	cantp_channel_t* channel_ptr;
	channel_ptr = &cantp_channels[tx_pdu_id];
	switch (channel_ptr->channel_state)
	{
	case CANTP_STATE_TX_WAIT_SF_CONF:
		break;
	case CANTP_STATE_TX_WAIT_FF_CONF:
		channel_ptr->channel_state = CANTP_STATE_TX_WAIT_FC;
		break;
	case CANTP_STATE_TX_WAIT_CF_CONF:
		if (channel_ptr->data_index < channel_ptr->data_length)
		{
			channel_ptr->channel_state = CANTP_STATE_TX_SENDING_CF;
		}
		else
		{
			channel_ptr->channel_state = CANTP_STATE_IDLE;
		}
		break;
	case CANTP_STATE_RX_WAIT_FC_CONF:
		channel_ptr->channel_state = CANTP_STATE_RX_RECEIVING_CF;
		break;
	default:
		break;
	}
}
