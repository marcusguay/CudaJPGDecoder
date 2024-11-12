#include "./include/BitStream.hpp"

 /* Gets the next bit from the buffer */
bool getNextBit(BitStreamStruct *bitstream)
{

  uint8_t old_value = bitstream->currentBufferValue;
  uint8_t old_index = bitstream->byteIndex;
  uint16_t old_bits = bitstream->value;
  bitstream->value = (bitstream->value << 1) | ((static_cast<uint16_t>(bitstream->currentBufferValue) >> (7 - bitstream->byteIndex)) & 1);
  bitstream->byteIndex++;

  if (bitstream->byteIndex == 8)
  {
    bitstream->indexInBuffer++;
    /*
    This part is to automatically handle byte stuffing in the bitstream parsing
    ie any (0xFF) byte that is not a marker (like 0xFF 0xAA for example)
    then it will be followed by a 0x00 byte that should be ignored
    */
    if (bitstream->currentBufferValue == 0xFF)
    {
      if (bitstream->buffer[bitstream->indexInBuffer] == 0x00)
      {
        bitstream->indexInBuffer++;
      }
      else
      {

        std::cout << "restart interval" << "\n";
        return false;
      }
    }

    bitstream->currentBufferValue = bitstream->buffer[(int)bitstream->indexInBuffer];
    bitstream->byteIndex = 0;
  }

  return true;
}



/*  (bad) debugging function */
void print(BitStreamStruct *bitstream)
{
  std::cout << std::bitset<8>(bitstream->buffer[bitstream->indexInBuffer - 3]) << " ";
  std::cout << std::bitset<8>(bitstream->buffer[bitstream->indexInBuffer - 2]) << " ";
  std::cout << std::bitset<8>(bitstream->buffer[bitstream->indexInBuffer - 1]) << " ";
  std::cout << std::bitset<8>(bitstream->currentBufferValue) << " ";
  std::cout << std::bitset<8>(bitstream->buffer[bitstream->indexInBuffer + 1]) << "\n";
}
