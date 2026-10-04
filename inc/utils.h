#ifndef UTILS_H
#define UTILS_H

#include <string>

// Δομή για τη διατήρηση των παραμέτρων εισόδου
struct Config {
    std::string hpatches_dir;
    std::string mirflickr_dir;
    std::string split_file;
    int vocab_size = 256;      // -vocab (K)
    int max_sift_per_img = 500; // -S
    int mir_count = 1000;       // -D
    std::string eval_set;       // validation / test
    std::string output_file;
    int seed = 1;

    // Μέθοδος αναζήτησης
    std::string method;         // exact, lsh, hypercube, ivfflat, ivfpq

    // Παράμετροι LSH
    int lsh_k = 4;
    int lsh_L = 5;
    double lsh_w = 0.05;

    // Παράμετροι Hypercube
    int cube_k = 10;
    double cube_w = 0.05;
    int cube_M = 500;
    int cube_probes = 10;

    // Παράμετροι IVF (Flat & PQ)
    int ivf_kclusters = -1;     // default calculation: ceil(sqrt(n))
    int ivf_nprobe = 5;

    // Παράμετροι IVFPQ
    int pq_M = 16;
    int pq_nbits = 8;
};

// Parsing των παραμέτρων από το CLI
bool parse_args(int argc, char* argv[], Config& config);

#endif // UTILS_H