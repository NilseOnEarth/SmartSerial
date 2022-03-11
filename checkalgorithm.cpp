#include "checkalgorithm.h"




unsigned char Alg_CheckSum(unsigned char *Buf, int Len)
{
  int i = 0;
  unsigned char sum = 0;
  unsigned char checksum = 0;

  for(i=0; i<Len; i++)
  {
    sum += *Buf++;
  }

  checksum = sum & 0xff;

  return checksum;
}


unsigned char Alg_CheckXOR(unsigned char *Buf, int Len)
{
  int i = 0;
  unsigned char x = 0;

  for(i=0; i<Len; i++)
  {
    x = x^(*(Buf+i));
  }

  return x;
}

unsigned short CRC16_Modbus(unsigned char *buf, int len)
{
    unsigned short crc = 0xFFFF;
    for (int pos = 0; pos < len; pos++)
    {
        crc ^= (unsigned int)buf[pos]; // XOR byte into least sig. byte of crc
        for(int i = 8; i != 0; i--)   // Loop over each bit
        {
            if ((crc & 0x0001) != 0)   // If the LSB is set
            {
                crc >>= 1; // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else // Else LSB is not set
            {
                crc >>= 1;    // Just shift right
            }
        }
    }

    //高低字节转换
    crc = ((crc & 0x00ff) << 8) | ((crc & 0xff00) >> 8);
    return crc;
}

unsigned char Alg_CheckLRC(unsigned char* data, int data_len)
{
    unsigned char lrc = 0;

    for (int i = 0; i < data_len; i++)
    {
     lrc +=  data[i];
    }
    lrc = ~lrc;
    lrc++;
    return lrc;
}


