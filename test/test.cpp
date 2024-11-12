#include "../src/encoder/include/EncoderHuffmanTree.hpp"
#include <vector>
#include <bitset>
#include "../src/decoder/include/DecoderHuffmanTree.hpp"
#include <cmath>
#include <stdio.h>
#include <cstring>
#include "../src/utils/include/BitStream.hpp"
#include <gtest/gtest.h>

/*
File was used to test the huffman encoding but in the
future I want to add tests for both the decoder and encoder
*/

TEST(TestProcessImages, TestHuffman)
{
  std::string s = "1iuh23u91uhiashujicuhauhidsuih1223213125";
  std::vector<uint8_t> buffer(s.begin(), s.end());
  EncoderHuffmanTree *encoderHuffmanTree = new EncoderHuffmanTree();
  Node *root = encoderHuffmanTree->Encode(buffer, 0, buffer.size());
  std::vector<uint16_t> encodedBuffer;
  encoderHuffmanTree->EncodeBuffer(buffer, encodedBuffer, root);

  std::cout << "start of original  : ";
  for (u_int8_t c : s)
  {
    std::cout << std::bitset<8>(c);
  }

  std::cout << "\n";
  std::vector<uint8_t> treeLengthsAndVals = encoderHuffmanTree->getLengthsAndValues();
  std::cout << "\n";
  DecoderHuffmanTree *tree = new DecoderHuffmanTree();
  tree->createTree(treeLengthsAndVals, 0);

  std::cout << "\n start of new buffer : ";
  for (u_int16_t c : encodedBuffer)
  {
    std::cout << std::bitset<16>(c);
  }

  std::vector<uint8_t> newBuffer(encodedBuffer.size() * sizeof(uint16_t));

  for (size_t i = 0; i < encodedBuffer.size(); ++i)
  {
    newBuffer[2 * i] = static_cast<uint8_t>(encodedBuffer[i] >> 8);       // High byte
    newBuffer[2 * i + 1] = static_cast<uint8_t>(encodedBuffer[i] & 0xFF); // Low byte
  }

  std::cout << "\n";

  std::vector<uint8_t> decodedBuffer;
  tree->decode(newBuffer, decodedBuffer);

  std::cout << "decoded buffer is " << "\n";
  for (char c : decodedBuffer)
  {
    std::cout << c;
  }
  std::cout << "\n compared to " << s;

  ASSERT_EQ(decodedBuffer.size(), s.length());

  for (int i = 0; i < decodedBuffer.size(); i++)
  {
    ASSERT_EQ(decodedBuffer[i], s[i]);
  }
}
