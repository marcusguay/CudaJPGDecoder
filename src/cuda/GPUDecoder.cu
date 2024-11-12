#include "./include/GPUDecoder.hpp"

#define CHECK_CUDA_ERROR(err)                                            \
    if (err != cudaSuccess)                                              \
    {                                                                    \
        fprintf(stderr, "CUDA Error at %s:%d: %s\n", __FILE__, __LINE__, \
                cudaGetErrorString(err));                                \
        return;                                                          \
    }

// Device variables
float *(d_blocks);
QuantizationTable(*d_quantizationTables);
int *(d_zigzagTable);
ImageSpecification *d_imageSpecification;

// Host variables
ImageSpecification imageSpecification;

__global__ void zigzagScanCUDA(float *d_matrix, int *d_zigzagTable)
{
    /* Converts a matrix into its zigzag ordered matrix */

    __shared__ float temp[64];
    int idx = threadIdx.x + (threadIdx.y * 8) + (blockIdx.x * 64);
    int zigZagIndex = threadIdx.x + threadIdx.y * 8;
    temp[zigZagIndex] = d_matrix[idx];

    __syncthreads();
    d_matrix[idx] = temp[d_zigzagTable[zigZagIndex]];
}

__global__ void check(float *d_matrix)
{
    int idx = threadIdx.x + (threadIdx.y * 8) + (blockIdx.x * 64);
    printf("at %d is %d \n", idx, d_matrix[idx]);
}

__global__ void applyQuantizationTables(float *d_matrix, QuantizationTable *d_quantizationTables, int *quantizationIndices)
{
    /* Applies element wise multiplication with correct quantization table for component  */

    int qtableIndex = quantizationIndices[((blockIdx.x % 3) + 1)];
    int insideQTTableIdx = threadIdx.x + (threadIdx.y * 8);
    int idx = threadIdx.x + (threadIdx.y * 8) + (blockIdx.x * 64);
    QuantizationTable table = d_quantizationTables[qtableIndex];

    if (table.precision == 0)
    {

        d_matrix[idx] = d_matrix[idx] * table.table8[insideQTTableIdx];
    }
    else
    {
        d_matrix[idx] = d_matrix[idx] * table.table16[insideQTTableIdx];
    }

    __syncthreads();
}

__global__ void inverseDCT(float *d_matrix)
{
    /* Standard formula for 2D Inverse Discrete Cosine Transform
       there are probably more optimized versions of this that I should probably look into.... */

    __shared__ float temp[64];
    int offset = (blockIdx.x * 64);
    int idx = threadIdx.x + (threadIdx.y * 8) + offset;
    int localIndex = threadIdx.x + (threadIdx.y * 8);
    temp[localIndex] = 0.0f;

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {

            float ci = (i == 0) ? 1.0 / sqrtf(2.0) : 1.0;
            float cj = (j == 0) ? 1.0 / sqrtf(2.0) : 1.0;
            int inputIndex = j * 8 + i + offset;
            temp[localIndex] += ci * cj * d_matrix[inputIndex] *
                                cosf((2.0 * threadIdx.x + 1.0) * i * M_PI / 16.0) *
                                cosf((2.0 * threadIdx.y + 1.0) * j * M_PI / 16.0);
        }
    }

    __syncthreads();

    /* JPG specs say to divide by four after the IDCT */
    d_matrix[idx] = roundf(temp[localIndex] / 4.0f);
}

__global__ void YCbCrToRGB(float *d_matrix, RGB *pixels, int imageWidth, int imageHeight)
{

    /*  Index translation here gets a bit complicated since the indices we
        are working with in the float array dont correspond to the indices the
        BMP file is expecting, ie: index 8 corresponds to (0, 1) in our 8 x 8 blocks
        yet index 8 in the BMP image it would be (9, 0)
    */

    int blockX = blockIdx.x % ((imageWidth + 7) / 8); // Find which CUDA block we are in
    int blockY = blockIdx.x / ((imageWidth + 7) / 8);

    int offsetX = blockX * 8 + threadIdx.x; // each X block index contributes 8 to the offset + our local x offset
                                            /*
                                            This one is the one that is hard to see until, you draw out the mappings
                                            For each thread Y index it adds the image width to the offset, each block index is equivalent
                                            to increasing the thread Y index by 8
                                            */


    /*
    Checks if padding pixels are contributing to final image
    */
    if (offsetX >= imageWidth || (blockY * 8 + threadIdx.y) >= imageHeight) {
        return;
    }


    int offsetY = (blockY * 8 + threadIdx.y) * imageWidth;
    int pixelIndex = offsetX + offsetY;

    /* We are expecting the MCU pattern to be (Y, Cb, Cr) consecutively */
    int yIndex = threadIdx.x + (threadIdx.y * 8) + blockIdx.x * 192;
    int cbIndex = yIndex + 64;
    int crIndex = yIndex + 128;

    /* Level only shift for luminance */
    float Y = d_matrix[yIndex] + 128.0;
    float Cb = d_matrix[cbIndex];
    float Cr = d_matrix[crIndex];

    /* (Y, Cb, Cr) -> (R,G,B) conversion formula according to the JPG specification */
    float R = Y + 1.402f * Cr;
    float G = Y - 0.344136f * Cb - 0.714136f * Cr;
    float B = Y + 1.772f * Cb;

    /* Have to make sure the values are in the [0,255] range */
    R = static_cast<uint8_t>(roundf(fminf(fmaxf(R, 0.0f), 255.0f)));
    G = static_cast<uint8_t>(roundf(fminf(fmaxf(G, 0.0f), 255.0f)));
    B = static_cast<uint8_t>(roundf(fminf(fmaxf(B, 0.0f), 255.0f)));

    /* Save the pixel to the right index of the array*/
    
    if(blockX == 0){
       printf("%d %d PixelIDX %d \n", blockX, blockY, pixelIndex);
    }

    pixels[pixelIndex].r = R;
    pixels[pixelIndex].g = G;
    pixels[pixelIndex].b = B;
}

void decodeImageCuda(std::vector<float> &flatBlocks, int quantizationIndex[4])
{

    /* Only supports (Y, Cb, Cr) images for now... */
    int numberOfComponents = 3;

    int numBlocks = std::ceil(flatBlocks.size() / 64);
    int size = sizeof(float) * flatBlocks.size();

    /* Each MCU is 8 x 8 block */
    dim3 nThreads(8, 8);

    cudaError_t err;

    int *quantizationIndices;

    std::cout << "flatBlocks size: " << flatBlocks.size() << ", total size in bytes: " << size << "\n";
    std::cout << "Quantization indexes " << quantizationIndex[1] << " " << quantizationIndex[2] << " " << quantizationIndex[3] << "\n";

    CHECK_CUDA_ERROR(cudaMalloc(&d_blocks, size));
    CHECK_CUDA_ERROR(cudaMemcpy(d_blocks, flatBlocks.data(), size, cudaMemcpyHostToDevice));

    CHECK_CUDA_ERROR(cudaMalloc(&quantizationIndices, sizeof(int) * 4));
    CHECK_CUDA_ERROR(cudaMemcpy(quantizationIndices, quantizationIndex, sizeof(int) * 4, cudaMemcpyHostToDevice));

    std::cout << "allocated " << numBlocks << " of size 64 on GPU " << "\n";

    applyQuantizationTables<<<numBlocks, nThreads>>>(d_blocks, d_quantizationTables, quantizationIndices);
    cudaDeviceSynchronize();

    err = cudaGetLastError();
    CHECK_CUDA_ERROR(err);

    zigzagScanCUDA<<<numBlocks, nThreads>>>(d_blocks, d_zigzagTable);
    cudaDeviceSynchronize();

    err = cudaGetLastError();
    CHECK_CUDA_ERROR(err);

    inverseDCT<<<numBlocks, nThreads>>>(d_blocks);
    cudaDeviceSynchronize();

    err = cudaGetLastError(); // is there a better to check this, then to do it for each CUDA call?
    CHECK_CUDA_ERROR(err);

    RGB *d_pixels;
    int numPixels = imageSpecification.width * imageSpecification.height;
    std::cout << "image has num pixels " << numPixels << "\n";
    CHECK_CUDA_ERROR(cudaMalloc(&d_pixels, sizeof(RGB) * numPixels));
    std::cout << "num 192 blocks " << (numBlocks / numberOfComponents) << "\n";

    /* Grid and block calculation here are a bit different
      since we are combining three components into one pixel
      we should launch a third of the blocks with size 192, (3 * 64)
      instead of the usual 64 size */

    int numMCUBlocks = numBlocks / numberOfComponents;
    int numBlocksWidth = (imageSpecification.width + 7) / 8;

    YCbCrToRGB<<<numMCUBlocks, nThreads>>>(d_blocks, d_pixels, imageSpecification.width, imageSpecification.height);
    cudaDeviceSynchronize();

    std::vector<RGB> rgb(numPixels);
    CHECK_CUDA_ERROR(cudaMemcpy(rgb.data(), d_pixels, sizeof(RGB) * numPixels, cudaMemcpyDeviceToHost));

    writeBMP("/usr/src/app/testImagesOutput/output1.bmp", rgb, imageSpecification.width, imageSpecification.height);
    std::cout << "decoded image to BMP at /usr/src/app/testImagesOutput !" << "\n";

    /* Now we have to clear everything from GPU memory
       for now the program is supposed to be ran once per image
       decoded, so clear zigzagtable too */

    cudaFree(d_blocks);
    cudaFree(d_imageSpecification);
    cudaFree(d_pixels);
    cudaFree(d_quantizationTables);
    cudaFree(d_zigzagTable);
}

void setupCuda(QuantizationTable (&quantizationTables)[4], ImageSpecification imageSpecificationHost)
{

    cudaError_t err;
    imageSpecification = imageSpecificationHost;

    err = cudaDeviceReset();
    CHECK_CUDA_ERROR(err);

    err = cudaSetDevice(0);
    CHECK_CUDA_ERROR(err);

    /* print device properties
       (to see if GPU is available to use) */

    cudaDeviceProp prop;
    err = cudaGetDeviceProperties(&prop, 0);
    CHECK_CUDA_ERROR(err);
    printf("Using GPU: %s\n", prop.name);

    /* allocate all the stuff we need on GPU */
    int quantizationTableSize = sizeof(QuantizationTable) * 4;
    CHECK_CUDA_ERROR(cudaMalloc(&d_quantizationTables, quantizationTableSize));
    CHECK_CUDA_ERROR(cudaMemcpy(d_quantizationTables, quantizationTables, quantizationTableSize, cudaMemcpyHostToDevice));

    int zigZagTableSize = sizeof(int) * 64;
    CHECK_CUDA_ERROR(cudaMalloc(&d_zigzagTable, zigZagTableSize));
    CHECK_CUDA_ERROR(cudaMemcpy(d_zigzagTable, zigzagTable, zigZagTableSize, cudaMemcpyHostToDevice));

    CHECK_CUDA_ERROR(cudaMalloc(&d_imageSpecification, sizeof(ImageSpecification)));
    CHECK_CUDA_ERROR(cudaMemcpy(d_imageSpecification, &imageSpecificationHost, sizeof(ImageSpecification), cudaMemcpyHostToDevice));
}
