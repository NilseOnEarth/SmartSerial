#ifndef CHECKALGORITHM_H
#define CHECKALGORITHM_H


//#define uint8_t unsigned char

unsigned char Alg_CheckSum(unsigned char *Buf, int Len);
unsigned char Alg_CheckXOR(unsigned char *Buf, int Len);
unsigned short CRC16_Modbus(unsigned char *buf, int len);
unsigned char Alg_CheckLRC(unsigned char* data, int data_len);

#endif // CHECKALGORITHM_H
