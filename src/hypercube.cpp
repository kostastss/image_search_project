#include "../inc/hypercube.h"
#include <random>
#include <algorithm>

Hypercube::Hypercube(int k, int probes, int candidates, int dimension)
    : num_bits(k), dim(dimension), max_probes(probes), max_candidates(candidates) {    
    
    std::mt19937 gen(1337); // Διαφορετικό seed από το LSH για ποικιλία
    std::normal_distribution<float> d(0.0, 1.0);

    for (int j = 0; j < num_bits; ++j) {
        cv::Mat r(1, dim, CV_32F);
        for (int d_idx = 0; d_idx < dim; ++d_idx) {
            r.at<float>(0, d_idx) = d(gen);
        }
        random_vectors.push_back(r);
    }
}

int Hypercube::compute_hash(const cv::Mat& hist) const {
    int hash_val = 0;
    for (int j = 0; j < num_bits; ++j) {
        if (hist.dot(random_vectors[j]) > 0) {
            hash_val |= (1 << j);
        }
    }
    return hash_val;
}

void Hypercube::add_to_database(const std::string& image_id, const cv::Mat& histogram) {
    db_histograms[image_id] = histogram.clone();
    int hash_val = compute_hash(histogram);
    buckets[hash_val].push_back(image_id); // Προσθήκη στην αντίστοιχη κορυφή
}

std::vector<SearchResult> Hypercube::search(const cv::Mat& query_hist, int top_k, int& checked_candidates) const {
    int q_hash = compute_hash(query_hist);
    std::vector<std::string> candidates;
    
    // Γεννάμε όλες τις 2^k κορυφές (π.χ. για 10 bits -> 1024 κορυφές)
    int num_vertices = (1 << num_bits);
    std::vector<std::pair<int, int>> vertices(num_vertices);
    
    for (int i = 0; i < num_vertices; ++i) {
        int xor_val = q_hash ^ i; // Το XOR μας δείχνει ποια bits διαφέρουν
        int dist = 0;
        
        // Υπολογισμός Hamming distance (μετρώντας τους άσους)
        int temp = xor_val;
        while(temp) {
            dist += temp & 1;
            temp >>= 1;
        }
        vertices[i] = {dist, i};
    }
    
    // Ταξινομούμε βάσει Hamming distance (απόσταση 0, μετά 1, μετά 2 κ.ο.κ.)
    std::sort(vertices.begin(), vertices.end());

    int probes = 0;
    for (const auto& v : vertices) {
        // Αν φτάσαμε το όριο κορυφών ή το όριο υποψηφίων, σταματάμε!
        if (probes >= max_probes || (int)candidates.size() >= max_candidates) break;
        
        int bucket_id = v.second;
        auto it = buckets.find(bucket_id);
        if (it != buckets.end()) {
            for (const auto& img_id : it->second) {
                candidates.push_back(img_id);
            }
            probes++; // Μετράμε πόσες ΜΗ κενές κορυφές προσπελάσαμε
        }
    }

    checked_candidates = candidates.size();

    // Ακριβής υπολογισμός Ευκλείδειας απόστασης μόνο για τους candidates
    std::vector<SearchResult> results;
    for (const auto& img_id : candidates) {
        const cv::Mat& db_hist = db_histograms.at(img_id);
        float dist = cv::norm(query_hist, db_hist, cv::NORM_L2);
        results.push_back({img_id, dist});
    }

    std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        return a.distance < b.distance;
    });

    if ((int)results.size() > top_k) {
        results.resize(top_k);
    }

    return results;
}