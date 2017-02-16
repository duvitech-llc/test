#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct rs485_frame {
	uint8_t 	start;
	uint8_t 	addr;
	uint16_t 	len;
	uint32_t 	memaddr;
	uint8_t* 	pData;
	uint16_t 	crc;
	uint8_t 	stop;
	uint8_t 	pad;
};


/*  https://www.lammertbies.nl/comm/info/crc-calculation.html  */

static unsigned short crc16(const unsigned char* data_p, unsigned char length) {
	unsigned char x;
	unsigned short crc = 0xFFFF;

	while (length--) {
		x = crc >> 8 ^ *data_p++;
		x ^= x >> 4;
		crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x << 5)) ^ ((unsigned short)x);
	}
	return crc;
}

static void dump_hex(uint8_t* byteData, int length) {

	uint8_t* pData;
	
	pData = byteData;

	if (pData == 0) {
		printf("EMPTY DATA\n\n");
	}
	else {
		for (int x = 1; x <= length; x++) {
			printf("0x%02X", *pData);
			if (x % 16 == 0) {
				printf("\n");
			}
			else {
				printf(" ");
			}

			pData++;
		}

		printf("\n\n");
	}
}

static void print_frame_details(struct rs485_frame* frame) {
	int datacount = frame->len;
	int frame_size = datacount + 11;
	uint8_t* pData;

	printf("\n===> Start Frame Details: <===\n");
	printf("Frame Size: %d\n", frame_size);
	printf("Start Byte: 0x%02X\n", frame->start);
	printf("Device Address: 0x%02X\n", frame->addr);
	printf("Memory Address: 0x%08X\n", frame->memaddr);
	printf("Data Length: 0x%04X\n", frame->len);

	printf("\nByte Data:\n\n");
	pData = (uint8_t*)frame->pData;

	dump_hex(frame->pData, frame->len);

	printf("CRC 16: 0x%04X\n", frame->crc);
	printf("Stop Byte: 0x%02X\n", frame->stop);
	printf("===> End Frame Details: <===\n\n");

}

static int send_receive_frame(struct rs485_frame* sendFrame, struct rs485_frame* pResponse) {
	uint8_t* txBuffer = 0;
	uint8_t* pBuffer;
	int ret = 0;
	printf("send_receive_frame start\n");
	
	print_frame_details(sendFrame);

	/* determine frame size and allocate memory*/
	int frame_size = sendFrame->len + 11;
	txBuffer = (uint8_t*)malloc(frame_size);
	pBuffer = txBuffer;
	
	/* fill the frame with data */
	memset(txBuffer, 0x00, frame_size);

	/* start byte */
	memcpy(pBuffer,(uint8_t*)&(sendFrame->start), 1);
	pBuffer++;

	/* address byte */
	memcpy(pBuffer, (uint8_t*)&(sendFrame->addr), 1);
	pBuffer++;

	/* memory address */
	memcpy(pBuffer, (uint8_t*)&(sendFrame->memaddr), 4);
	pBuffer+=4;

	/* data length word */
	memcpy(pBuffer, (uint8_t*)&(sendFrame->len), 2);
	pBuffer += 2;

	/* data variable length*/
	if (sendFrame->len > 0) {

		if (sendFrame->pData != 0) {
			memcpy(pBuffer, sendFrame->pData, sendFrame->len);
			pBuffer += sendFrame->len;
		}
		else {
			printf("data not allocated in frame correctly\n");
			ret = -1;
			goto finish;
		}
	}

	/* crc */
	memcpy(pBuffer, (uint8_t*)&(sendFrame->crc), 2);
	pBuffer += 2;

	/* stop byte */
	memcpy(pBuffer, (uint8_t*)&(sendFrame->stop), 1);
	pBuffer++;

	/* dump frame */
	printf("\nFrame Data:\n\n");
	dump_hex(txBuffer, frame_size);

finish:
	/* free memory */
	if (txBuffer != 0) {
		free(txBuffer);
	}
	
	txBuffer = 0;
	pBuffer = 0;

	printf("send_receive_frame stop\n");
	return ret;
}

int main() {
	
	int res;
	struct rs485_frame frame;
	struct rs485_frame recFrame;

	uint8_t data[] = { '1','2','3','4','5','6','7','8','9' };

	printf("uint8_t size: %d\n", sizeof(uint8_t));
	printf("uint16_t size: %d\n", sizeof(uint16_t));
	printf("uint32_t size: %d\n", sizeof(uint32_t));
	printf("pointer size: %d\n", sizeof(void*));

	printf("Struct Size: %d\n", sizeof(struct rs485_frame));

	frame.start = 0xFF;
	frame.addr = 0x00;
	frame.memaddr = 0x00000000;
	frame.len = 9;
	frame.pData = (uint8_t*)malloc(frame.len);
	memcpy(frame.pData, data, frame.len);

	if (frame.len > 0) {
		frame.crc = crc16(frame.pData, frame.len);
	}
	else {
		frame.crc = 0x0000;
	}

	frame.stop = 0xFF;
	
	res = send_receive_frame(&frame, &recFrame);

	if (frame.pData != 0)
		free(frame.pData);
	
	return 0;
}
