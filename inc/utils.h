#ifndef UTILS_H
#define UTILS_H

#include <string>

struct Config {
    std::string hpatches_dir;
    std::string mirflickr_dir;
    std::string split_file;
    std::string output_file = "output.txt";
    std::string method = "exact";
    std::string query_set = "validation";
    int vocab_size = 256;
    int max_S = 500;                      
    int seed = 1;
    int D = 0;
};

bool parse_args(int argc, char* argv[], Config& config);

#endif // UTILS_H