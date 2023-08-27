#include <gtest/gtest.h>
#include <string.h>
#include "cantp.h"
#include "cantp_callout.h"

static uint8_t rx_frame[4095] = {0};
static uint8_t rx_length = 0;
void dcm_rx_indication(uint8_t length,uint8_t* data)
{
	rx_length = length;
    for(int i = 0; i<length; i++)
    {
    	rx_frame[i] = data[i];
    	printf("0x%x ",data[i]);
    }
    printf("******************");
    printf("\n\r");
}
static uint8_t tx_frame[8]={0};
void canif_transmit(uint8_t tx_sdu_id, const pdu_info_t* tx_info_ptr)
{
    for(int i = 0; i<tx_info_ptr->sdu_length; i++)
    {
    	tx_frame[i] = tx_info_ptr->sdu_data_ptr[i];
    	printf("0x%x ",tx_info_ptr->sdu_data_ptr[i]);
    }
    printf("@@@@@@@@@@@@@@@@@@@");
    printf("\n\r");
}
class CanTpTest : public testing::Test {
protected:
	uint8_t test_data_tx_mf[22]={0x22,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x9,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x20,0x21};
	uint8_t can_data_tx_mf_ff[8]={0x10,0x16,0x22,0x1,0x2,0x3,0x4,0x5};
	uint8_t can_data_tx_mf_cf_1[8]={0x21,0x06,0x07,0x08,0x9,0x10,0x11,0x12};
	uint8_t can_data_tx_mf_cf_2[8]={0x22,0x13,0x14,0x15,0x16,0x17,0x18,0x19};
	uint8_t can_data_tx_mf_cf_3[8]={0x23,0x20,0x21,0xaa,0xaa,0xaa,0xaa,0xaa};

	uint8_t test_data_rx_fc[3]={0x30, 0x00, 0x00};
	uint8_t test_data_rx_sf[3]={0x02,0x10,0x02};
	uint8_t can_data_rx_sf[2]={0x10,0x02};

	uint8_t test_data_rx_mf[14]={0x36,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x9,0x10,0x11,0x12,0x13};
	uint8_t test_data_rx_ff[8]={0x10,0x0e,0x36,0x01,0x02,0x03,0x04,0x05};
	uint8_t test_data_rx_cf_1[8]={0x21,0x06,0x07,0x08,0x09,0x10,0x11,0x12};
	uint8_t test_data_rx_cf_2[2]={0x22,0x13};
	uint8_t test_data_tx_sf[2]={0x10,0x02};
	uint8_t can_data_tx_sf[8]={0x02,0x10,0x02,0xaa,0xaa,0xaa,0xaa,0xaa};

	pdu_info_t test_tx_mf = {
		test_data_tx_mf,
        22U
	};
	pdu_info_t test_tx_sf = {
        test_data_tx_sf,
        2U
	};
	pdu_info_t test_rx_fc = {
        test_data_rx_fc,
        3U
	};
	pdu_info_t test_rx_sf = {
        test_data_rx_sf,
        3U
	};
	pdu_info_t test_rx_ff = {
		test_data_rx_ff,
        8U
	};
	pdu_info_t test_rx_cf_1 = {
		test_data_rx_cf_1,
        8U
	};
	pdu_info_t test_rx_cf_2 = {
		test_data_rx_cf_2,
        2U
	};

    void SetUp() override {
    }

    void cantp_test_tx_sf() {
    	cantp_transmit(0U, &test_tx_sf);
    	EXPECT_EQ(memcmp(tx_frame,can_data_tx_sf,8), 0);
    	cantp_tx_confirmation(0);
    }

    void cantp_test_tx_mf() {

    	cantp_transmit(0U, &test_tx_mf);
    	EXPECT_EQ(memcmp(tx_frame,can_data_tx_mf_ff,8), 0);
    	cantp_tx_confirmation(0);
    	cantp_main_function();
    	cantp_rx_indication(0,&test_rx_fc);
    	cantp_main_function();
    	EXPECT_EQ(memcmp(tx_frame,can_data_tx_mf_cf_1,8), 0);
    	cantp_tx_confirmation(0);
    	cantp_main_function();
    	EXPECT_EQ(memcmp(tx_frame,can_data_tx_mf_cf_2,8), 0);
    	cantp_tx_confirmation(0);
    	cantp_main_function();
    	EXPECT_EQ(memcmp(tx_frame,can_data_tx_mf_cf_3,8), 0);
    	cantp_tx_confirmation(0);
    }

    void cantp_test_rx_sf() {
    	cantp_rx_indication(0, &test_rx_sf);
    	cantp_main_function();
    	cantp_tx_confirmation(0);
    	EXPECT_EQ(memcmp(rx_frame,can_data_rx_sf,2), 0);
    }

    void cantp_test_rx_mf() {
    	cantp_rx_indication(0, &test_rx_ff);
    	cantp_main_function();
    	cantp_tx_confirmation(0);
    	cantp_main_function();
    	cantp_rx_indication(0, &test_rx_cf_1);
    	cantp_main_function();
    	cantp_rx_indication(0, &test_rx_cf_2);
    	cantp_main_function();
    	EXPECT_EQ(memcmp(rx_frame,test_data_rx_mf,14), 0);
    }
};

TEST_F(CanTpTest, TxSf) {
	cantp_test_tx_sf();
}
TEST_F(CanTpTest, TxMf) {
	cantp_test_tx_mf();
}
TEST_F(CanTpTest, RxSf) {
	cantp_test_rx_sf();
}
TEST_F(CanTpTest, RxMf) {
	cantp_test_rx_mf();
}


