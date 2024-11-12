
#include <iostream>
#include <filesystem>
#include <vector>
#include "./decoder/include/Decoder.hpp";

int main(int nargs, char *args[])
{

  Decoder* decoder = new Decoder();

  /* example use case (so far it will just create a BMP file in ./src/testImagesOutput)
     I plan on adding more functionality later ...
     decoder->decode("/usr/src/app/testImages/bello.jpg"); 
  */
}