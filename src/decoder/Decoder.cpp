

#include "./include/Decoder.hpp"
#define AC 0b00010000
#define DC 0b00000000

void hexdump(void *ptr, int buflen)
{
    unsigned char *buf = (unsigned char *)ptr;
    int i, j;
    for (i = 0; i < buflen; i += 16)
    {
        printf("%06x: ", i);
        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                printf("%02x ", buf[i + j]);
            else
                printf("   ");
        printf(" ");
        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                printf("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
        printf("\n");
    }
}

/* When decoding AC and DC Cofficients a '1' bit at the start means
   its a negative number (so even though we are decoding unsigned integers)
   the result should be negative
*/
int16_t extendSign(uint16_t value, int category)
{
    if (value < (1 << (category - 1)))
    {
        value = value - (1 << category) + 1;
    }
    return static_cast<int16_t>(value);
}

/*
 Helper function to decode two bytes from the buffer
 and combine them into a uint_16
*/
uint16_t decodeTwoBytes(std::vector<uint8_t> &buffer, int index)
{
    uint16_t firstByte = (static_cast<uint8_t>(buffer[index]));
    uint16_t secondByte = (static_cast<uint8_t>(buffer[index + 1]));
    return secondByte | firstByte << 8;
}

void Decoder::decodeHuffmanTable(std::vector<uint8_t> &buffer, int index)
{
    std::cout << "Decoding huffman tree at " << index << "\n";
    uint16_t totalLength = decodeTwoBytes(buffer, index + 2);
    std::cout << "Table length of " << totalLength << "\n";

    int bytesRead = 0;
    while (bytesRead < totalLength - 2)
    {
        uint8_t type = buffer[index + 4 + bytesRead] & 0xF0;
        int tableIndex = buffer[index + 4 + bytesRead] & 0x0F;
        DecoderHuffmanTree *tree = new DecoderHuffmanTree();
        bytesRead = bytesRead + (tree->createTree(buffer, index + 5 + bytesRead) + 1);

        if (type == AC)
        {
            std::cout << "AC table read:" << bytesRead << " saving tree to " << tableIndex << " \n";
            this->ACTrees[tableIndex] = tree;
        }
        else if (type == DC)
        {
            std::cout << "DC table read:" << bytesRead << " saving tree to " << tableIndex << " \n";
            this->DCTrees[tableIndex] = tree;
        }
        else
        {
            throw std::runtime_error("Table not AC or DC !?!?!?");
        }
    }
}

void Decoder::decodeStartOfFrame(std::vector<uint8_t> &buffer, int index)
{
    std::cout << "Decoding start of frame at: " << index << "\n";

    uint16_t totalLength = decodeTwoBytes(buffer, index + 2);
    int precision = buffer[index + 4];
    uint16_t totalHeight = decodeTwoBytes(buffer, index + 5);
    uint16_t totalWidth = decodeTwoBytes(buffer, index + 7);

    int numComponents = buffer[index + 9];

    std::cout << "Total length: " << totalLength << " Precision: " << precision << " Total height: " << totalHeight << " TotalWidth : " << totalWidth << " NumComponents : " << numComponents << "\n";

    this->imageSpecification = ImageSpecification{totalWidth, totalHeight, numComponents};

    if (numComponents == 1)
    {
        throw std::runtime_error("Grayscale images not supported yet");
    }

    int offset = 10;
    int i = 0;
    while (i < numComponents)
    {

        int type = buffer[index + offset];
        int samplingFactorVertical = (buffer[index + offset + 1] & 0xF0) >> 4;
        int samplingFactorHorizontal = buffer[index + offset + 1] & 0x0F;
        int tableIndex = buffer[index + offset + 2];

        std::cout << type << " " << samplingFactorHorizontal << " " << samplingFactorVertical << " Table number " << tableIndex << "\n";

        if (samplingFactorHorizontal != 1 || samplingFactorVertical != 1)
        {
            throw std::runtime_error("Chroma subsampling other than 4:4:4 not supported yet");
        }
        std::cout << "For component: " << i << " Table index is: " << tableIndex << " Table values:\n";

        for (int k = 0; k < 64; k++)
        {
            std::cout << (int)this->quantizationTables[tableIndex].table8[k] << " ";
        }

        std::cout << "\n";

        this->quantizationTableMapping[type] = tableIndex;
        offset += 3;
        i++;
    }
}

void Decoder::decodeQuantizationTable(std::vector<uint8_t> &buffer, int index)
{
    std::cout << "Start of quantization table: " << index << "\n";
    uint16_t totalLength = decodeTwoBytes(buffer, index + 2);

    int numTables = totalLength / 65;

    std::cout << "QT Header has: " << numTables << " tables \n";

    int offset = 4;
    while (numTables > 0)
    {

        int precision = (buffer[index + offset] & 0xF0) >> 4;
        this->imageSpecification.precision = precision;
        int tableIndex = (buffer[index + offset] & 0x0F);
        int numValues = 64 * (precision + 1);
        std::cout << "Table has length: " << totalLength << " Index: " << tableIndex << " Precision: " << precision << " numValues: " << numValues << "\n";

        /* redefine quantization table if it is at the same index as a previous one (apparently this happens) */
        quantizationTables[tableIndex] = QuantizationTable();
        quantizationTables[tableIndex].precision = precision;

        int i = 0;
        int decNumTables = 1;

        if (precision == 1)
        {
            decNumTables = 2;
        }

        while (i < numValues)
        {

            if (precision == 0)
            {

                // 8 bit precision
                quantizationTables[tableIndex].table8[i] = (buffer[index + offset + 1 + i]);
                std::cout << (int)buffer[index + offset + 1 + i] << " ";
                i++;
            }
            else
            {
                // 16 bit precision
                uint16_t totalByte = decodeTwoBytes(buffer, index + offset + i + 1);
                quantizationTables[tableIndex].table16[i / 2] = (totalByte);
                i += 2;
            }
        }

        numTables -= decNumTables;
        offset += numValues + 1;
    }
}

void Decoder::decodeStartOfScan(std::vector<uint8_t> &buffer, int index)
{
    std::cout << "decoding start of scan tree at " << index << "\n";
    uint16_t totalLength = decodeTwoBytes(buffer, index + 2);
    int numComponents = buffer[index + 4];

    std::cout << "header length " << totalLength << " numComponents " << numComponents << "\n";
    int offset = 5;
    for (int i = 0; i < numComponents; i++)
    {
        int type = (buffer[index + offset]);
        int dcHuffmanTableIndex = ((buffer[index + offset + 1]) & 0xF0) >> 4;
        int acHuffmanTableIndex = ((buffer[index + offset + 1]) & 0x0F);
        std::cout << " type " << type << " dcHtIndex " << dcHuffmanTableIndex << " acHtIndex " << acHuffmanTableIndex << "\n";
        this->componentHuffmanTableIndex[i] = std::pair(dcHuffmanTableIndex, acHuffmanTableIndex);
        offset += 2;
    }

    /* next three bytes are spectral selector stuff (not sure quite sure what it is meant for) */
    offset += 3;



    /* Copying the Image data over for scanning 
     I guess it might be a little redundant to copy it over 
     but it makes it easier for the bitstream and its index
    */
    int newBufferLength = buffer.size() - index;
    std::vector<uint8_t> scanDataBuffer(newBufferLength);
    std::memcpy(scanDataBuffer.data(), buffer.data() + offset + index, newBufferLength);

    /* Round the width and height up to handle images 
       that have widths and heights arent multiples of 8
    */
    int numBlocksWidth = (imageSpecification.width + 7) / 8; 
    int numBlocksHeight = (imageSpecification.height + 7) / 8; 
    int numBlocks = numBlocksWidth * numBlocksHeight;         

    std::cout << "Image has :" << numBlocks << " 64 pixel blocks \n";
    std::cout << "New buffer has size :" << newBufferLength << "\n";
    std::cout << offset << "\n";

    int numBits = 0;
    int blocksize = 64;
    int componentBlockSize = 192;
    bool restart = false;

   
    BitStreamStruct *bitStream = new BitStreamStruct(scanDataBuffer);

    std::vector<int16_t> prevDC(numComponents, 0);
    std::vector<float> flatBlocks(numBlocks * blocksize * numComponents, 0.0f);

    /* Function to call CUDA Setup */
    setupCuda(this->quantizationTables, this->imageSpecification);

    /* Start decoding Image data!!!! */
    for (int i = 0; i < numBlocks; i++)
    {
        for (int componentIndex = 0; componentIndex < imageSpecification.numComponents; componentIndex++)
        {
            int dcHTIndex = componentHuffmanTableIndex[componentIndex].first;
            int acHTIndex = componentHuffmanTableIndex[componentIndex].second;
            DecoderHuffmanTree *dcTree = DCTrees[dcHTIndex];
            DecoderHuffmanTree *acTree = ACTrees[acHTIndex];

            int dcCategory = 0;
            int dcIteration = 0;
            while (dcIteration < 16)
            {
                restart = getNextBit(bitStream);
                numBits++;
                uint16_t value = bitStream->value;
                std::pair<uint16_t, int> mapIndex(value, numBits);

                if (dcTree->map.count(mapIndex) > 0)
                {
                    dcCategory = dcTree->map[mapIndex];
                    bitStream->value = 0;
                    numBits = 0;
                    break;
                }

                if (dcIteration == 16)
                {
                    if (restart)
                    {
                        std::cout << "restart interval found" << "\n";
                        dcIteration = 0;
                        bitStream->value = 0;
                        numBits = 0;

                        /* You are supposed to reset all previous DC coefficients for
                          each component upon a restart interval */

                        for (int c = 0; c < numComponents; c++)
                        {
                            prevDC[c] = 0;
                        }
                    }
                    return;
                }

                dcIteration++;
            }

            int16_t dcAmplitude = 0;
            if (dcCategory > 0)
            {
                uint16_t value = 0;
                for (int j = 0; j < dcCategory; j++)
                {
                    getNextBit(bitStream);
                    value = (value << 1) | bitStream->value;
                }

                dcAmplitude = extendSign(value, dcCategory);
            }

            flatBlocks[(componentBlockSize * i) + (componentIndex * blocksize)] = dcAmplitude + prevDC[componentIndex];
            prevDC[componentIndex] = dcAmplitude + prevDC[componentIndex];
            bitStream->value = 0;
            numBits = 0;

            // AC coefficient decoding
            int currentIndex = 1;
            while (currentIndex < 64)
            {
                int acCategory = 0;
                int16_t acAmplitude = 0;
                uint8_t zeroRun = 0;

                while (true)
                {
                    getNextBit(bitStream);
                    numBits++;
                    uint16_t value = bitStream->value;
                    std::pair<uint16_t, int> mapIndex(value, numBits);

                    if (acTree->map.count(mapIndex) > 0)
                    {
                        uint8_t huffValue = acTree->map[mapIndex];
                        zeroRun = (huffValue >> 4) & 0xF;
                        acCategory = huffValue & 0xF;
                        bitStream->value = 0;
                        numBits = 0;

                        /*  Reached end of the AC Block  */
                        if (huffValue == 0x00)
                        {
                            currentIndex = 64;
                            break;
                        }
                        break;
                    }
                }

                currentIndex += zeroRun;
                if (currentIndex < 64 && acCategory > 0)
                {
                    uint16_t value = 0;
                    for (int j = 0; j < acCategory; j++)
                    {
                        getNextBit(bitStream);
                        value = (value << 1) | bitStream->value;
                    }

                    acAmplitude = extendSign(value, acCategory);
                    flatBlocks[(componentBlockSize * i) + (componentIndex * blocksize) + currentIndex] = acAmplitude;

                    bitStream->value = 0;
                    numBits = 0;
                    currentIndex++;
                }
            }
        }
    }

    decodeImageCuda(flatBlocks, this->quantizationTableMapping);
}

void Decoder::decode(std::string imagePath)
{
    std::filesystem::path path(imagePath);
    auto file = std::fopen(imagePath.c_str(), "r+");

    if (!file)
    {
        throw std::runtime_error("file was not found!");
    }

    int fileSize = std::filesystem::file_size(path);
    std::vector<uint8_t> buffer(fileSize);

    std::fread(&buffer[0], fileSize, 1, file);

    /* For debugging
       hexdump(&buffer[0], fileSize);  */

    std::cout << "file at " << path.c_str() << " is " << fileSize << " bytes \n";

    if (std::ferror(file))
    {
        throw std::runtime_error("error when reading file");
    }
    else if (std::feof(file))
    {
        throw std::runtime_error("reached end of file before finding tables");
    }

    for (int i = 0; i < buffer.size(); i++)
    {
        uint8_t c1 = buffer[i];
        if (c1 == 0xFF && (i + 1 < buffer.size()))
        {
            uint8_t c2 = buffer[i + 1];
            switch (c2)
            {
            case 0xE0:
                std::cout << "Header at " << i << "\n";
                i++;
                break;
            case 0xD8:
                std::cout << "Start of image at " << i << "\n";
                i++;
                break;
            case 0xD9:
                std::cout << "End of image at " << i << "\n";
                i++;
                break;
            case 0xC0:
                std::cout << "Start of frame base" << i << "\n";
                this->decodeStartOfFrame(buffer, i);
                i++;
                break;
            case 0xC2:
                std::cout << "Start of frame base (proggresive)" << i << "\n";
                throw std::runtime_error("Progressive images not supported yet");
                i++;
                break;
            case 0xC4:
                std::cout << "Huffman tables " << i << "\n";
                this->decodeHuffmanTable(buffer, i);
                i++;
                break;
            case 0xDB:
                std::cout << "Quantization tables " << i << "\n";
                this->decodeQuantizationTable(buffer, i);
                i++;
                break;
            case 0xDA:
                std::cout << "Scan begin " << i << "\n";
                this->decodeStartOfScan(buffer, i);
                i++;
                break;
            case 0xDD:
                std::cout << "Restart interval " << i << "\n";
                i++;
                break;
            case 0xFE:
                std::cout << "Comment " << i << "\n";
                i++;
                break;
            default:
                break;
            }
        }
    }

    fclose(file);
}