#ifndef IVFPQ_H
#define IVFPQ_H

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include "search_engine.h"

// Αντί να αποθηκεύουμε ένα τεράστιο διάνυσμα 256-float (1024 bytes),
// αποθηκεύουμε μόνο m bytes (π.χ. 8 bytes) ανά εικόνα!
struct PQCode {
    std::string image_id;
    std::vector<uint8_t> codes;
};

class IVFPQ {
private:
    int num_clusters; // Coarse clusters (Kc)
    int num_probes;   // Πόσα clusters ψάχνουμε (W)
    int m;            // Σε πόσα υπο-διανύσματα χωρίζουμε το 256-D (π.χ. 8)
    int sub_dim;      // Η διάσταση κάθε υπο-διανύσματος (256/8 = 32)
    int k_sub;        // Πόσα sub-centroids ανά κομμάτι (έως 256 για να χωράει σε uint8_t)

    cv::Mat coarse_centroids; // Τα μεγάλα κέντρα του IVF
    std::vector<cv::Mat> pq_centroids; // Τα μικρά κέντρα συμπίεσης (ένα Mat για κάθε m)

    std::vector<std::vector<PQCode>> inverted_index;

    // Προσωρινή αποθήκευση πριν το build
    std::vector<cv::Mat> raw_histograms;
    std::vector<std::string> raw_db_ids;

public:
    // Εξ ορισμού: 40 clusters, ψάχνουμε 4, m=8 κομμάτια, 256 κέντρα/κομμάτι
    IVFPQ(int clusters = 40, int probes = 4, int m_sub = 8, int k_subcenters = 256, int dimension = 256);
    
    void add_to_database(const std::string& image_id, const cv::Mat& histogram);
    
    void build_index(); 
    
    std::vector<SearchResult> search(const cv::Mat& query_hist, int top_k, int& checked_candidates) const;
};

#endif // IVFPQ_H