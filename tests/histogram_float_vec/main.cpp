#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>

#include <sys/time.h>
#include <FreeImage.h>
#include "hipacc.hpp"


#define WIDTH    1024
#define HEIGHT   1024
#define NUM_BINS 1024


using namespace hipacc;


class Histogram : public Kernel<float, float4> {
    private:
        Accessor<float> &input;

    public:
        Histogram(IterationSpace<float> &iter, Accessor<float> &input)
                : Kernel(iter), input(input) {
            add_accessor(&input);
        }

        void kernel() {
            output() = input();
        }

        void binning(unsigned int x, unsigned int y, float pixel) {
            bin((uint)(pixel*num_bins()-.5f)) = { 1.0f, 1.0f, 1.0f, 1.0f };
        }

        float4 reduce(float4 left, float4 right) const {
            return left + right;
        }
};

/*************************************************************************
 * Main function                                                         *
 *************************************************************************/
int main(int argc, const char **argv) {
    float timing = .0f;
    int num_bins = NUM_BINS;

    if (argc > 1) {
      num_bins = atoi(argv[1]);
    }

    const int width = WIDTH;
    const int height = HEIGHT;
    
    float refBin[num_bins];
    for (size_t i = 0; i < num_bins; ++i) {
      refBin[i] = 0.0f;
    }

    float* input = new float[width*height]; 
    srand(time(NULL));
    for(size_t y = 0; y < height; ++y) {
        for(size_t x = 0; x < width; ++x) {
            // rand -> [0.0f - 1.0f]
            float pixel = static_cast<float>(rand() % num_bins)
                    / static_cast<float>(num_bins-1);
            input[x+y*width] = pixel;
            refBin[(uint)(pixel*num_bins-.5f)] += 1.0f;
        }           
    }

    //*************************************************************************//

    Image<float> IN(width, height, input);
    Image<float> OUT(width, height);

    IterationSpace<float> IS(OUT);

    Accessor<float> AccIN(IN);

    Histogram Hist(IS, AccIN);

    std::cout << "Executing kernel" << std::endl;
    Hist.execute();
    timing += hipacc_last_kernel_timing();

    std::cout << "Executing histogram" << std::endl;
    float4* bin = Hist.binned_data(num_bins);
    timing += hipacc_last_kernel_timing();

    //float* output = OUT.data();

    //*************************************************************************//

    delete [] input;

    bool pass = true;

    for (size_t i = 0; i < num_bins; ++i) {
      if (i == 0) std::cout << "refBin: ";
      else        std::cout << ", ";
      std::cout << refBin[i];
      if (i == num_bins-1) std::cout << std::endl;
    }

    for (size_t i = 0; i < num_bins; ++i) {
      if (i == 0) std::cout << "bin: ";
      else        std::cout << ", ";
      std::cout << bin[i].w;
      if (i == num_bins-1) std::cout << std::endl;
      if (bin[i].w != refBin[i]) {pass = false;
      std::cout << std::endl << "FAIL at " << i << ": " << bin[i].w << " vs. " << refBin[i] << std::endl;}
    }

    std::cout << (pass ? "PASSED" : "FAILED") << std::endl;

    std::cout << "OVERALL: " << timing << std::endl;

    return EXIT_SUCCESS;
}

