#include "dataset.h"
#include <iostream>
#include <fstream>
#include <sstream>

bool load_hpatches_splits(const std::string& split_file, Dataset& dataset) {
    std::ifstream file(split_file);
    if (!file.is_open()) {
        std::cerr << "Σφάλμα: Δεν ήταν δυνατό το άνοιγμα του αρχείου " << split_file << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        std::string seq_name, seq_type;
        
        if (iss >> seq_name >> seq_type) {
            Sequence seq{seq_name, seq_type};
            if (seq_type == "train") dataset.train_seqs.push_back(seq);
            else if (seq_type == "validation") dataset.val_seqs.push_back(seq);
            else if (seq_type == "test") dataset.test_seqs.push_back(seq);
        }
    }
    
    std::cout << "[Dataset] Φορτώθηκαν: " << dataset.train_seqs.size() << " train, " 
              << dataset.val_seqs.size() << " validation, " 
              << dataset.test_seqs.size() << " test ακολουθίες." << std::endl;
    
    return true;
}