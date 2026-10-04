#include <iostream>
#include "utils.h"
#include "dataset.h"

int main(int argc, char* argv[]) {
    Config config;

    if (!parse_args(argc, argv, config)) {
        std::cerr << "Χρήση: ./bin/search-hp -hp <path> -split <file> -o <output> [options]" << std::endl;
        return 1;
    }

    std::cout << "--- Εκκίνηση Αναζήτησης Εικόνων ---" << std::endl;
    std::cout << "HPatches Dir: " << config.hpatches_dir << std::endl;
    std::cout << "Split File: " << config.split_file << std::endl;
    std::cout << "Method: " << config.method << std::endl;
    std::cout << "Vocab (K): " << config.vocab_size << "\n" << std::endl;

    // Φόρτωση Dataset
    Dataset dataset;
    if (!load_hpatches_splits(config.split_file, dataset)) {
        return 1; // Τερματίζει αν δεν βρει το αρχείο
    }

    return 0;
}