#include "../inc/search_engine.h"
#include <algorithm>

void SearchEngine::add_to_database(const std::string& image_id, const cv::Mat& histogram) {
    db_image_ids.push_back(image_id);
    
    // Ενώνουμε τα ιστογράμματα σε έναν μεγάλο πίνακα για γρήγορο υπολογισμό
    if (db_histograms.empty()) {
        db_histograms = histogram.clone();
    } else {
        cv::vconcat(db_histograms, histogram, db_histograms);
    }
}

std::vector<SearchResult> SearchEngine::exact_search(const cv::Mat& query_hist, int top_k) const {
    std::vector<SearchResult> results;
    
    if (db_histograms.empty() || query_hist.empty()) {
        return results;
    }

    // Υπολογισμός Ευκλείδειας απόστασης (L2 norm) με όλα τα διανύσματα
    for (int i = 0; i < db_histograms.rows; ++i) {
        float dist = cv::norm(query_hist, db_histograms.row(i), cv::NORM_L2);
        results.push_back({db_image_ids[i], dist});
    }

    // Ταξινόμηση βάσει απόστασης (από τη μικρότερη στη μεγαλύτερη)
    std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        return a.distance < b.distance;
    });

    // Κρατάμε τα top_k αποτελέσματα (συνολικά 10 βάσει της εκφώνησης)
    if ((int)results.size() > top_k) {
        results.resize(top_k);
    }

    return results;
}

void SearchEngine::clear() {
    db_image_ids.clear();
    db_histograms.release();
}