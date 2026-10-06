#ifndef HYPERCUBE_H
#define HYPERCUBE_H

#include <vector>
#include <string>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include "search_engine.h"

class Hypercube {
private:
    int num_bits;       // Η διάσταση του υπερκύβου (k)
    int dim;            // Η διάσταση του διανύσματος SIFT BoW (256)
    int max_probes;     // Μέγιστος αριθμός κορυφών που θα ψάξουμε (M)
    int max_candidates; // Μέγιστος αριθμός υποψήφιων εικόνων (points)

    std::vector<cv::Mat> random_vectors; // Τα επίπεδα προβολής
    
    // Αντιστοίχιση Κορυφής (hash value) -> Λίστα εικόνων
    std::unordered_map<int, std::vector<std::string>> buckets;
    std::unordered_map<std::string, cv::Mat> db_histograms;

    // Επιστρέφει την κορυφή (ως ακέραιο)
    int compute_hash(const cv::Mat& hist) const;

public:
    // Βάζουμε k=10 bits, ψάχνουμε το πολύ 20 κορυφές και σταματάμε στους 500 υποψήφιους
    Hypercube(int k = 10, int probes = 20, int candidates = 500, int dimension = 256);
    
    void add_to_database(const std::string& image_id, const cv::Mat& histogram);
    std::vector<SearchResult> search(const cv::Mat& query_hist, int top_k, int& checked_candidates) const;
};

#endif // HYPERCUBE_H