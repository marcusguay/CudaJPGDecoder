#pragma once
#include <iostream>
#include <cstdio>
#include "../../structs/DecoderStructs.hpp"
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>


extern int numComponents;
extern int numBlocks;
extern int imageWidth;
extern int imageHeight;


void decodeImageCuda(std::vector<float> & flatBlocks, int quantizationIndex [3]);
void setupCuda(QuantizationTable (&quantizationTables)[4], ImageSpecification imageSpecification);
