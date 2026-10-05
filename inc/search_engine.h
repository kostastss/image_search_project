#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

struct SearchResult {
    std::string image_id;
    float distance;
};

class SearchEngine {
private:
    std::vector<std::string> db_image_ids;
    cv::Mat db_histograms; // Ο πίνακας με όλα τα διανύσματα της βάσης

public:
    // Προσθήκη εικόνας στη βάση δεδομένων
    void add_to_database(const std::string& image_id, const cv::Mat& histogram);
    
    // Εξαντλητική αναζήτηση (Exact Search) με Ευκλείδεια απόσταση
    std::vector<SearchResult> exact_search(const cv::Mat& query_hist, int top_k = 10) const;
    
    void clear();
};

#endif // SEARCH_ENGINE_H