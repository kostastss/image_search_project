#ifndef DATASET_H
#define DATASET_H

#include <string>
#include <vector>

struct Sequence {
    std::string name;
    std::string type; // "train", "validation" ή "test"
};

struct Dataset {
    std::vector<Sequence> train_seqs;
    std::vector<Sequence> validation_seqs; 
    std::vector<Sequence> test_seqs;      
};

bool load_hpatches_splits(const std::string& split_file, Dataset& dataset);

#endif // DATASET_H