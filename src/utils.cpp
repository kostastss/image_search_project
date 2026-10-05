#include "../inc/utils.h"
#include <iostream>
#include <string>

bool parse_args(int argc, char* argv[], Config& config) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // Βασικές παράμετροι διαδρομών
        if (arg == "-hp" && i + 1 < argc) {
            config.hpatches_dir = argv[++i];
        } else if (arg == "-mir" && i + 1 < argc) {
            config.mirflickr_dir = argv[++i];
        } else if (arg == "-split" && i + 1 < argc) {
            config.split_file = argv[++i];
        } else if (arg == "-o" && i + 1 < argc) {
            config.output_file = argv[++i];
        } 
        // Παράμετροι συνόλων και μεγεθών
        else if (arg == "-set" && i + 1 < argc) {
            config.query_set = argv[++i];
        } else if (arg == "-vocab" && i + 1 < argc) {
            config.vocab_size = std::stoi(argv[++i]);
        } else if (arg == "-S" && i + 1 < argc) {
            config.max_S = std::stoi(argv[++i]);
        } else if (arg == "-D" && i + 1 < argc) {
            config.D = std::stoi(argv[++i]);
        } else if (arg == "-seed" && i + 1 < argc) {
            config.seed = std::stoi(argv[++i]);
        } 
        // Μέθοδοι αναζήτησης
        else if (arg == "-exact") {
            config.method = "exact";
        } else if (arg == "-lsh") {
            config.method = "lsh";
        } else if (arg == "-hypercube") {
            config.method = "hypercube";
        } else if (arg == "-ivfflat") {
            config.method = "ivfflat";
        } else if (arg == "-ivfpq") {
            config.method = "ivfpq";
        }
    }

    // Έλεγχος αν δόθηκαν τα απολύτως απαραίτητα
    if (config.hpatches_dir.empty() || config.split_file.empty()) {
        std::cerr << "Σφάλμα: Πρέπει να δώσετε τα -hp και -split!" << std::endl;
        return false;
    }

    return true;
}