#include "../inc/lsh.h"
#include <random>
#include <algorithm>
#include <unordered_set>

LSH::LSH(int L, int k, int dimension) : num_tables(L), num_bits(k), dim(dimension) {
    hash_tables.resize(num_tables);
    random_vectors.resize(num_tables);

    // Αρχικοποιούμε μια γεννήτρια Κανονικής Κατανομής για να φτιάξουμε τα επίπεδα
    std::mt19937 gen(42); // Σταθερό seed για να έχουμε τα ίδια επίπεδα σε κάθε εκτέλεση
    std::normal_distribution<float> d(0.0, 1.0);

    for (int i = 0; i < num_tables; ++i) {
        for (int j = 0; j < num_bits; ++j) {
            cv::Mat r(1, dim, CV_32F);
            for (int d_idx = 0; d_idx < dim; ++d_idx) {
                r.at<float>(0, d_idx) = d(gen);
            }
            random_vectors[i].push_back(r);
        }
    }
}

int LSH::compute_hash(const cv::Mat& hist, int table_idx) const {
    int hash_val = 0;
    for (int j = 0; j < num_bits; ++j) {
        // Υπολογίζουμε το εσωτερικό γινόμενο (dot product)
        double dot_product = hist.dot(random_vectors[table_idx][j]);
        
        // Αν το γινόμενο είναι θετικό (είναι "μπροστά" από το επίπεδο), βάζουμε τον άσο στο αντίστοιχο bit
        if (dot_product > 0) {
            hash_val |= (1 << j);
        }
    }
    return hash_val;
}

void LSH::add_to_database(const std::string& image_id, const cv::Mat& histogram) {
    // 1. Κρατάμε το αυθεντικό ιστόγραμμα
    db_histograms[image_id] = histogram.clone();

    // 2. Το κάνουμε hash και το ρίχνουμε στον αντίστοιχο κουβά και στα L Hash Tables
    for (int i = 0; i < num_tables; ++i) {
        int hash_val = compute_hash(histogram, i);
        hash_tables[i][hash_val].push_back(image_id);
    }
}

std::vector<SearchResult> LSH::search(const cv::Mat& query_hist, int top_k, int& checked_candidates) const {
    std::unordered_set<std::string> candidates;

    // ΒΗΜΑ 1: Βρίσκουμε τους υποψήφιους
    for (int i = 0; i < num_tables; ++i) {
        int hash_val = compute_hash(query_hist, i);
        auto it = hash_tables[i].find(hash_val);
        
        // Αν υπάρχουν εικόνες σε αυτόν τον κουβά, τις προσθέτουμε στο set (ώστε να αποφύγουμε διπλοτυπίες)
        if (it != hash_tables[i].end()) {
            for (const auto& img_id : it->second) {
                candidates.insert(img_id);
            }
        }
    }

    checked_candidates = candidates.size();

    // ΒΗΜΑ 2: Ακριβής υπολογισμός ΜΟΝΟ για τους υποψήφιους (από εδώ κερδίζουμε χρόνο!)
    std::vector<SearchResult> results;
    for (const auto& img_id : candidates) {
        const cv::Mat& db_hist = db_histograms.at(img_id);
        float dist = cv::norm(query_hist, db_hist, cv::NORM_L2);
        results.push_back({img_id, dist});
    }

    // ΒΗΜΑ 3: Ταξινόμηση και επιλογή των κορυφαίων
    std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        return a.distance < b.distance;
    });

    if ((int)results.size() > top_k) {
        results.resize(top_k);
    }

    return results;
}