#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include "../inc/utils.h"
#include "../inc/dataset.h"
#include "../inc/sift_extractor.h"
#include "../inc/kmeans_vocab.h"
#include "../inc/search_engine.h"

int main(int argc, char* argv[]) {
    Config config;
    if (!parse_args(argc, argv, config)) {
        return 1;
    }

    // 1. Φόρτωση Dataset
    Dataset dataset;
    if (!load_hpatches_splits(config.split_file, dataset)) {
        return 1;
    }

    // 2. SIFT Extractor
    SiftExtractor sift_ext;
    std::vector<std::vector<cv::Mat>> train_img_descriptors;
    
    std::cout << "[Main] Εξαγωγή SIFT για την εκπαίδευση του k-means..." << std::endl;
    for (const auto& seq : dataset.train_seqs) {
        for (int i = 1; i <= 6; ++i) {
            std::string img_path = config.hpatches_dir + "/" + seq.name + "/" + std::to_string(i) + ".ppm";
            std::vector<cv::Mat> descs;
            if (sift_ext.extract_descriptors(img_path, descs)) {
                train_img_descriptors.push_back(descs);
            }
        }
    }

    int max_S = 500; // Όπως προτείνει η εκφώνηση
    std::vector<cv::Mat> pooled_train_descs = sift_ext.sample_descriptors(train_img_descriptors, max_S, config.seed);
    
    KMeansVocab vocab(config.vocab_size);
    if (!vocab.train(pooled_train_descs)) {
        return 1;
    }

    // 3. Επιλογή Συνόλου για Αναζήτηση (Validation ή Test)
    const std::vector<Sequence>* target_set = &dataset.validation_seqs;
    if (config.query_set == "test") {
        target_set = &dataset.test_seqs;
    }

    SearchEngine engine;
    
    // Κατασκευή της Βάσης Δεδομένων (εικόνες 2-6 από τις ακολουθίες του συνόλου)
    std::cout << "\n[Search] Χτίσιμο βάσης δεδομένων..." << std::endl;
    for (const auto& seq : *target_set) {
        for (int i = 2; i <= 6; ++i) {
            std::string img_id = seq.name + "/" + std::to_string(i);
            std::string img_path = config.hpatches_dir + "/" + seq.name + "/" + std::to_string(i) + ".ppm";
            
            std::vector<cv::Mat> descs;
            if (sift_ext.extract_descriptors(img_path, descs)) {
                cv::Mat hist = vocab.compute_bow_histogram(descs);
                engine.add_to_database(img_id, hist);
            }
        }
    }

    // Εδώ μελλοντικά θα προσθέσουμε και τις MIRFlickr εικόνες στη βάση (engine.add_to_database)

    std::ofstream out_file(config.output_file);
    if (!out_file.is_open()) {
        std::cerr << "Σφάλμα δημιουργίας αρχείου εξόδου: " << config.output_file << std::endl;
        return 1;
    }

    out_file << (config.method == "exact" ? "Exact Search\n" : config.method + "\n");

    double total_recall5 = 0.0, total_recall10 = 0.0, total_ap10 = 0.0, total_time = 0.0;
    int num_queries = target_set->size();

    std::cout << "[Search] Εκτέλεση queries..." << std::endl;
    
    // 4. Εκτέλεση Αναζήτησης για κάθε Query (εικόνα 1)
    for (const auto& seq : *target_set) {
        std::string query_path = config.hpatches_dir + "/" + seq.name + "/1.ppm";
        std::vector<cv::Mat> query_descs;
        sift_ext.extract_descriptors(query_path, query_descs);
        cv::Mat query_hist = vocab.compute_bow_histogram(query_descs);

        auto start = std::chrono::high_resolution_clock::now();
        
        // Exact Search
        std::vector<SearchResult> results = engine.exact_search(query_hist, 10);
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        total_time += elapsed.count();

        out_file << "\nQuery: " << seq.name << "\n";
        
        // Υπολογισμός Μετρικών για το Query
        int relevant_found = 0;
        int relevant_at_5 = 0;
        double ap10 = 0.0;
        
        for (size_t j = 0; j < results.size(); ++j) {
            out_file << "Rank-" << (j + 1) << ": " << results[j].image_id << "\n";
            
            // Μια εικόνα είναι σχετική αν ξεκινάει με το όνομα της ακολουθίας (π.χ. "v_bird/")
            if (results[j].image_id.find(seq.name + "/") == 0) {
                relevant_found++;
                if (j < 5) relevant_at_5++;
                ap10 += (double)relevant_found / (j + 1); //
            }
        }
        
        double recall5 = (double)relevant_at_5 / 5.0; // 5 είναι το σύνολο των σχετικών εικόνων
        double recall10 = (double)relevant_found / 5.0;
        ap10 = ap10 / 5.0; //

        out_file << "Recall@5: " << std::fixed << std::setprecision(4) << recall5 << "\n";
        out_file << "Recall@10: " << recall10 << "\n";
        out_file << "AP@10: " << ap10 << "\n";

        total_recall5 += recall5;
        total_recall10 += recall10;
        total_ap10 += ap10;
    }

    // 5. Συνολικά Αποτελέσματα
    out_file << "\nMean Recall@5: " << std::fixed << std::setprecision(4) << (total_recall5 / num_queries) << "\n";
    out_file << "Mean Recall@10: " << (total_recall10 / num_queries) << "\n";
    out_file << "mAP@10: " << (total_ap10 / num_queries) << "\n"; //
    out_file << "Average query time: " << (total_time / num_queries) << " ms\n"; //

    out_file.close();
    std::cout << "[Search] Ολοκληρώθηκε! Τα αποτελέσματα αποθηκεύτηκαν στο " << config.output_file << std::endl;

    return 0;
}