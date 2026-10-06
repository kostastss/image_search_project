#ifndef LSH_H
#define LSH_H

#include <vector>
#include <string>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include "search_engine.h" // Χρησιμοποιούμε το struct SearchResult

class LSH {
private:
    int num_tables; // L: Αριθμός από Hash Tables
    int num_bits;   // k: Αριθμός bits (προβολών) ανά Hash Table
    int dim;        // Η διάσταση του διανύσματος (vocab_size, π.χ. 256)

    // Οι τυχαίες προβολές: num_tables πίνακες, ο καθένας με num_bits διανύσματα (διαστάσεων dim)
    std::vector<std::vector<cv::Mat>> random_vectors;

    // Τα Hash Tables: Αντιστοιχίζουν ένα ακέραιο hash value σε μια λίστα από image IDs
    std::vector<std::unordered_map<int, std::vector<std::string>>> hash_tables;

    // Αποθήκευση των πραγματικών ιστογραμμάτων για τον τελικό (ακριβή) υπολογισμό απόστασης
    std::unordered_map<std::string, cv::Mat> db_histograms;

    // Βοηθητική: Υπολογίζει σε ποιον "κουβά" πέφτει ένα διάνυσμα για το συγκεκριμένο table
    int compute_hash(const cv::Mat& hist, int table_idx) const;

public:
    // Εξ ορισμού βάζουμε L=5 πίνακες και k=10 bits (άρα 1024 κουβάδες ανά πίνακα)
    LSH(int L = 5, int k = 10, int dimension = 256);

    void add_to_database(const std::string& image_id, const cv::Mat& histogram);
    
    // Επιστρέφει τα κοντινότερα αποτελέσματα και το πλήθος των διανυσμάτων που τελικά ελέγχθηκαν
    std::vector<SearchResult> search(const cv::Mat& query_hist, int top_k, int& checked_candidates) const;
};

#endif // LSH_H