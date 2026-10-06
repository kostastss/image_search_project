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
#include "../inc/lsh.h" // Προσθήκη της κεφαλίδας για το LSH
#include "../inc/hypercube.h" // <-- Προσθήκη για τον Υπερκύβο
#include "../inc/ivfflat.h" // <-- Προσθήκη για το IVF-Flat
#include "../inc/ivfpq.h" // <-- Προσθήκη για το Product Quantization

int main(int argc, char* argv[]) {
    Config config;
    if (!parse_args(argc, argv, config)) return 1;

    Dataset dataset;
    if (!load_hpatches_splits(config.split_file, dataset)) return 1;

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

    int max_S = config.max_S; 
    std::vector<cv::Mat> pooled_train_descs = sift_ext.sample_descriptors(train_img_descriptors, max_S, config.seed);
    
    KMeansVocab vocab(config.vocab_size);
    if (!vocab.train(pooled_train_descs)) return 1;

    const std::vector<Sequence>* target_set = (config.query_set == "test") ? &dataset.test_seqs : &dataset.validation_seqs;

    // Αρχικοποίηση όλων των μηχανών
    SearchEngine exact_engine;
    LSH lsh_engine(5, 10, config.vocab_size); 
    Hypercube hc_engine(10, 20, 500, config.vocab_size); 
    IVFFlat ivf_engine(40, 4); 
    IVFPQ ivfpq_engine(40, 4, 8, 256, config.vocab_size); // <-- Νέο: 40 clusters, W=4, m=8, k_sub=256
    
    std::cout << "\n[Search] Χτίσιμο βάσης δεδομένων..." << std::endl;
    for (const auto& seq : *target_set) {
        for (int i = 2; i <= 6; ++i) {
            std::string img_id = seq.name + "/" + std::to_string(i);
            std::string img_path = config.hpatches_dir + "/" + seq.name + "/" + std::to_string(i) + ".ppm";
            
            std::vector<cv::Mat> descs;
            if (sift_ext.extract_descriptors(img_path, descs)) {
                cv::Mat hist = vocab.compute_bow_histogram(descs);
                
                // Προσθήκη στην κατάλληλη μηχανή
                if (config.method == "exact") exact_engine.add_to_database(img_id, hist);
                else if (config.method == "lsh") lsh_engine.add_to_database(img_id, hist);
                else if (config.method == "hypercube") hc_engine.add_to_database(img_id, hist);
                else if (config.method == "ivfflat") ivf_engine.add_to_database(img_id, hist);
                else if (config.method == "ivfpq") ivfpq_engine.add_to_database(img_id, hist); // <-- Νέο
            }
        }
    }

    if (config.D > 0) {
        std::cout << "[Search] Προσθήκη " << config.D << " εικόνων MIRFlickr στη βάση..." << std::endl;
        int added_mir = 0, img_idx = 1;
        while (added_mir < config.D) {
            std::string img_id = "im" + std::to_string(img_idx) + ".jpg";
            std::string img_path = config.mirflickr_dir + "/" + img_id;
            
            std::vector<cv::Mat> descs;
            if (sift_ext.extract_descriptors(img_path, descs) && !descs.empty()) {
                cv::Mat hist = vocab.compute_bow_histogram(descs);
                
                if (config.method == "exact") exact_engine.add_to_database(img_id, hist);
                else if (config.method == "lsh") lsh_engine.add_to_database(img_id, hist);
                else if (config.method == "hypercube") hc_engine.add_to_database(img_id, hist);
                else if (config.method == "ivfflat") ivf_engine.add_to_database(img_id, hist);
                else if (config.method == "ivfpq") ivfpq_engine.add_to_database(img_id, hist); // <-- Νέο
                
                added_mir++;
                if (added_mir % 1000 == 0) std::cout << "  Προστέθηκαν " << added_mir << "/" << config.D << " distractors..." << std::endl;
            }
            img_idx++;
            if (img_idx > 25000 && added_mir < config.D) break;
        }
    }

    // Οι δομές IVF απαιτούν clustering ΑΦΟΥ μπουν τα δεδομένα
    if (config.method == "ivfflat") {
        ivf_engine.build_index();
    } else if (config.method == "ivfpq") { // <-- Νέο
        ivfpq_engine.build_index();
    }

    std::ofstream out_file(config.output_file);
    out_file << "Method: " << config.method << "\n"; 

    double total_recall5 = 0.0, total_recall10 = 0.0, total_ap10 = 0.0, total_time = 0.0;
    long long total_candidates_checked = 0; 
    int num_queries = target_set->size();

    std::cout << "[Search] Εκτέλεση queries..." << std::endl;
    
    for (const auto& seq : *target_set) {
        std::string query_path = config.hpatches_dir + "/" + seq.name + "/1.ppm";
        std::vector<cv::Mat> query_descs;
        sift_ext.extract_descriptors(query_path, query_descs);
        cv::Mat query_hist = vocab.compute_bow_histogram(query_descs);

        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<SearchResult> results;
        int checked_cands = 0;

        // Εκτέλεση Αναζήτησης
        if (config.method == "exact") {
            results = exact_engine.exact_search(query_hist, 10);
            checked_cands = (115 + config.D); 
        } else if (config.method == "lsh") {
            results = lsh_engine.search(query_hist, 10, checked_cands);
        } else if (config.method == "hypercube") {
            results = hc_engine.search(query_hist, 10, checked_cands);
        } else if (config.method == "ivfflat") {
            results = ivf_engine.search(query_hist, 10, checked_cands);
        } else if (config.method == "ivfpq") { // <-- Νέο
            results = ivfpq_engine.search(query_hist, 10, checked_cands);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        total_time += elapsed.count();
        total_candidates_checked += checked_cands;

        out_file << "\nQuery: " << seq.name << "\n";
        
        int relevant_found = 0, relevant_at_5 = 0;
        double ap10 = 0.0;
        
        for (size_t j = 0; j < results.size(); ++j) {
            out_file << "Rank-" << (j + 1) << ": " << results[j].image_id << "\n";
            if (results[j].image_id.find(seq.name + "/") == 0) {
                relevant_found++;
                if (j < 5) relevant_at_5++;
                ap10 += (double)relevant_found / (j + 1);
            }
        }
        
        double recall5 = (double)relevant_at_5 / 5.0; 
        double recall10 = (double)relevant_found / 5.0;
        ap10 = ap10 / 5.0; 

        out_file << "Recall@5: " << std::fixed << std::setprecision(4) << recall5 << "\n";
        out_file << "Recall@10: " << recall10 << "\n";
        out_file << "AP@10: " << ap10 << "\n";

        total_recall5 += recall5;
        total_recall10 += recall10;
        total_ap10 += ap10;
    }

    out_file << "\nMean Recall@5: " << std::fixed << std::setprecision(4) << (total_recall5 / num_queries) << "\n";
    out_file << "Mean Recall@10: " << (total_recall10 / num_queries) << "\n";
    out_file << "mAP@10: " << (total_ap10 / num_queries) << "\n";
    out_file << "Average query time: " << (total_time / num_queries) << " ms\n";
    out_file << "Average candidates checked per query: " << (total_candidates_checked / num_queries) << " / " << (115 + config.D) << "\n";

    out_file.close();
    std::cout << "[Search] Ολοκληρώθηκε! Τα αποτελέσματα αποθηκεύτηκαν στο " << config.output_file << std::endl;

    return 0;
}