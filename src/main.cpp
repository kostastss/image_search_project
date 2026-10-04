#include <iostream>
#include "utils.h"

int main(int argc, char* argv[]) {
    Config config;

    if (!parse_args(argc, argv, config)) {
        std::cerr << "Χρήση: ./bin/search-hp -hp <path> -split <file> -o <output> [options]" << std::endl;
        return 1;
    }

    std::cout << "--- Εκκίνηση Αναζήτησης Εικόνων ---" << std::endl;
    std::cout << "HPatches Dir: " << config.hpatches_dir << std::endl;
    std::cout << "MIRFlickr Dir: " << config.mirflickr_dir << std::endl;
    std::cout << "Split File: " << config.split_file << std::endl;
    std::cout << "Method: " << config.method << std::endl;
    std::cout << "Vocab (K): " << config.vocab_size << std::endl;
    std::cout << "Seed: " << config.seed << std::endl;

    // TODO: Βήμα 2 - Φόρτωση Dataset
    // TODO: Βήμα 3 - Εξαγωγή SIFT & k-means
    // TODO: Βήμα 4 - Δημιουργία Διανυσμάτων
    // TODO: Βήμα 5 - Εκτέλεση Αναζήτησης & Εξαγωγή Αποτελεσμάτων

    return 0;
}