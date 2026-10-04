#include "utils.h"
#include <iostream>
#include <cstdlib>

bool parse_args(int argc, char* argv[], Config& config) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-hp" && i + 1 < argc) config.hpatches_dir = argv[++i];
        else if (arg == "-mir" && i + 1 < argc) config.mirflickr_dir = argv[++i];
        else if (arg == "-split" && i + 1 < argc) config.split_file = argv[++i];
        else if (arg == "-vocab" && i + 1 < argc) config.vocab_size = std::atoi(argv[++i]);
        else if (arg == "-S" && i + 1 < argc) config.max_sift_per_img = std::atoi(argv[++i]);
        else if (arg == "-D" && i + 1 < argc) config.mir_count = std::atoi(argv[++i]);
        else if (arg == "-set" && i + 1 < argc) config.eval_set = argv[++i];
        else if (arg == "-o" && i + 1 < argc) config.output_file = argv[++i];
        else if (arg == "-seed" && i + 1 < argc) config.seed = std::atoi(argv[++i]);
        // Μέθοδοι
        else if (arg == "-exact") config.method = "exact";
        else if (arg == "-lsh") config.method = "lsh";
        else if (arg == "-hypercube") config.method = "hypercube";
        else if (arg == "-ivfflat") config.method = "ivfflat";
        else if (arg == "-ivfpq") config.method = "ivfpq";
        // LSH Params
        else if (arg == "-k" && i + 1 < argc) config.lsh_k = std::atoi(argv[++i]);
        else if (arg == "-L" && i + 1 < argc) config.lsh_L = std::atoi(argv[++i]);
        else if (arg == "-w" && i + 1 < argc) {
            if (config.method == "hypercube") config.cube_w = std::atof(argv[++i]);
            else config.lsh_w = std::atof(argv[++i]);
        }
        // Hypercube Params
        else if (arg == "-kproj" && i + 1 < argc) config.cube_k = std::atoi(argv[++i]);
        else if (arg == "-M" && i + 1 < argc) {
            if (config.method == "ivfpq") config.pq_M = std::atoi(argv[++i]);
            else config.cube_M = std::atoi(argv[++i]);
        }
        else if (arg == "-probes" && i + 1 < argc) config.cube_probes = std::atoi(argv[++i]);
        // IVF Params
        else if (arg == "-kclusters" && i + 1 < argc) config.ivf_kclusters = std::atoi(argv[++i]);
        else if (arg == "-nprobe" && i + 1 < argc) config.ivf_nprobe = std::atoi(argv[++i]);
        else if (arg == "-nbits" && i + 1 < argc) config.pq_nbits = std::atoi(argv[++i]);
    }

    if (config.hpatches_dir.empty() || config.split_file.empty() || config.output_file.empty()) {
        std::cerr << "Σφάλμα: Λείπουν βασικές παράμετροι εισόδου (-hp, -split, -o)." << std::endl;
        return false;
    }

    return true;
}