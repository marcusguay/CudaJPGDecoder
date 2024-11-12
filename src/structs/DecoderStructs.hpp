
#pragma once
#include <vector>
#include <stdint.h>
#include <fstream>

#ifndef zigzag
#define zigzag

/* Zigzag table from the JPG standard */
const int zigzagTable[64] = {
    0, 1, 5, 6, 14, 15, 27, 28,
    2, 4, 7, 13, 16, 26, 29, 42,
    3, 8, 12, 17, 25, 30, 41, 43,
    9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63};

#endif

typedef struct
{
    int type;
    int tableIndex;
} QuantizationTableMapping;

typedef struct
{
    int width;
    int height;
    int numComponents;
    int precision;
    int numYBlocks;
    int numCbBlocks;
    int numCrBlocks;
} ImageSpecification;

/*  Union reserves the same space for both tables
 in the struct since we can only have one or the other */
typedef struct
{
    union
    {
        uint8_t table8[64];
        uint16_t table16[64];
    };
    int precision; // 0 if 8 bit, 1 if 16 bit
} QuantizationTable;

struct RGB
{
    uint8_t r, g, b;
};

#pragma pack(push, 1)    // this changes the struct padding
struct BMPHeader
{
    uint16_t signature = 0x4D42; // 'BM'
    uint32_t fileSize;           // Size of BMP file
    uint16_t reserved1 = 0;      // Reserved
    uint16_t reserved2 = 0;      // Reserved
    uint32_t dataOffset = 54;    // Offset to image data
};

struct BMPInfoHeader
{
    uint32_t headerSize = 40;     // Size of info header
    int32_t width;                // Image width
    int32_t height;               // Image height
    uint16_t planes = 1;          // Number of color planes
    uint16_t bitsPerPixel = 24;   // Bits per pixel (RGB)
    uint32_t compression = 0;     // Compression type
    uint32_t imageSize;           // Size of image data
    int32_t xPixelsPerMeter = 0;  // X pixels per meter
    int32_t yPixelsPerMeter = 0;  // Y pixels per meter
    uint32_t colorsUsed = 0;      // Number of colors used
    uint32_t importantColors = 0; // Important colors
};

#pragma pack(pop)

/* function is inline because i dont want to create its own file for one function lol.... */
inline void writeBMP(const std::string &filename, const std::vector<RGB> &imageData, int width, int height)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Cannot create BMP file");
    }
    int padding = (4 - (width * 3) % 4) % 4;
    int rowSize = width * 3 + padding;

    BMPHeader header;
    BMPInfoHeader infoHeader;

    infoHeader.width = width;
    infoHeader.height = height;
    std::cout << "BMP size " << width << " height " << height << "\n"; 
    infoHeader.imageSize = rowSize * height;
    header.fileSize = 54 + infoHeader.imageSize;

    file.write(reinterpret_cast<char *>(&header), sizeof(header));
    file.write(reinterpret_cast<char *>(&infoHeader), sizeof(infoHeader));

    std::vector<uint8_t> padding_bytes(padding, 0);
    for (int y = height - 1; y >= 0; y--)
    {
        for (int x = 0; x < width; x++)
        {
            const RGB &pixel = imageData[y * width + x];
            file.put((pixel.b));
            file.put((pixel.g));
            file.put(pixel.r);
        }
        if (padding > 0)
        {
            file.write(reinterpret_cast<char *>(padding_bytes.data()), padding);
        }
    }
    file.close();
}