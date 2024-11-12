# About

Note: This project was mainly for learning purposes and so I probably would not put this in production project (Atleast not yet) 

At the time of writing this the decoder only supports images with
three channels, 4:4:4 subsampling and that was non progressively encoded

I plan on adding compatibility to these types of JPGs later
and maybe an encoder eventually

## Requirements
- NVIDIA GPU with compute capability 3.0 or higher
- Docker (for using the Docker setup)
- CMake (For compiling)
- Make (For compiling)


# Running the code 
1. Create a build directory and navigate into it:
```
mkdir build && cd build
```
2. Run CMake to configure the project:

```
cmake ..
```

3. Build the project:
```
make
```

4. Run the app
```
./app
```

# Example usage

```
#include <iostream>
#include <filesystem>
#include <vector>
#include "./decoder/include/Decoder.hpp"

int main(int nargs, char *args[]) {

    Decoder* decoder = new Decoder();
    decoder->decode("/usr/src/app/testImages/hello.jpg"); 

}

```