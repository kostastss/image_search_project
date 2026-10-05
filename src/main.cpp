#include <iostream>
#include "../inc/utils.h"
#include "../inc/dataset.h"
#include "../inc/kmeans_vocab.h"
#include "../inc/sift_extractor.h"

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

    // 1. Φόρτωση Dataset
    Dataset dataset;
    if (!load_hpatches_splits(config.split_file, dataset)) {
        return 1;
    }

    // 2. Εξαγωγή SIFT descriptors για το Training Set
    SiftExtractor sift_ext;
    std::vector<std::vector<cv::Mat>> train_img_descriptors;

    std::cout << "\n[Main] Επεξεργασία " << dataset.train_seqs.size() << " training ακολουθιών..." << std::endl;
    
    // Για απλότητα δοκιμής, διατρέχουμε τις ακολουθίες του train set
    for (const auto& seq : dataset.train_seqs) {
        // Υποθέτουμε τη δομή φακέλων του HPatches (π.χ. hpatches_dir/seq_name/1.ppm, κλπ.)
        for (int i = 1; i <= 6; ++i) {
            std::string img_path = config.hpatches_dir + "/" + seq.name + "/" + std::to_string(i) + ".ppm";
            std::vector<cv::Mat> descs;
            if (sift_ext.extract_descriptors(img_path, descs)) {
                train_img_descriptors.push_back(descs);
            }
        }
    }

    std::cout << "[Main] Συλλέχθηκαν descriptors από " << train_img_descriptors.size() << " εικόνες training." << std::endl;

    // 3. Τυχαία δειγματοληψία έως S descriptors ανά εικόνα (π.χ. S = 500)
    int max_S = 500;
    std::vector<cv::Mat> pooled_train_descs = sift_ext.sample_descriptors(train_img_descriptors, max_S, config.seed);
    std::cout << "[Main] Τελικό σύνολο δειγμάτων για k-means: " << pooled_train_descs.size() << " descriptors." << std::endl;

    // 4. Εκπαίδευση K-Means / Visual Vocabulary
    KMeansVocab vocab(config.vocab_size);
    if (!vocab.train(pooled_train_descs)) {
        std::cerr << "Σφάλμα στην εκπαίδευση του k-means!" << std::endl;
        return 1;
    }

    std::cout << "\n[Main] Η διαδικασία προχώρησε επιτυχώς έως τη δημιουργία του Visual Vocabulary!" << std::endl;

    return 0;
}