#include "../inc/ivfflat.h"
#include <algorithm>
#include <iostream>

IVFFlat::IVFFlat(int clusters, int probes) : num_clusters(clusters), num_probes(probes) {
    inverted_index.resize(num_clusters);
}

void IVFFlat::add_to_database(const std::string& image_id, const cv::Mat& histogram) {
    db_histograms[image_id] = histogram.clone();
    raw_histograms.push_back(histogram.clone());
    raw_db_ids.push_back(image_id);
}

void IVFFlat::build_index() {
    std::cout << "\n[IVF-Flat] Εκπαίδευση k-means για δημιουργία " << num_clusters << " clusters (Voronoi cells)..." << std::endl;
    
    // Ενώνουμε τα ιστογράμματα σε έναν πίνακα NxD
    cv::Mat data;
    cv::vconcat(raw_histograms, data);
    data.convertTo(data, CV_32F); // Το k-means του OpenCV απαιτεί float32

    int k = std::min(num_clusters, data.rows);
    cv::Mat labels;
    cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 100, 0.001);
    
    cv::kmeans(data, k, labels, criteria, 3, cv::KMEANS_PP_CENTERS, centroids);

    // Κατανομή των εικόνων της βάσης στα clusters τους
    for (int i = 0; i < data.rows; ++i) {
        int cluster_idx = labels.at<int>(i);
        inverted_index[cluster_idx].push_back(raw_db_ids[i]);
    }

    // Καθαρίζουμε τη μνήμη των προσωρινών δομών
    raw_histograms.clear();
    raw_db_ids.clear();
    
    std::cout << "[IVF-Flat] Η κατασκευή του Inverted Index ολοκληρώθηκε." << std::endl;
}

std::vector<SearchResult> IVFFlat::search(const cv::Mat& query_hist, int top_k, int& checked_candidates) const {
    if (centroids.empty()) {
        checked_candidates = 0;
        return {};
    }

    // 1. Βρίσκουμε τις αποστάσεις του query από όλα τα centroids (κέντρα)
    std::vector<std::pair<float, int>> cluster_distances;
    cv::Mat query_f;
    query_hist.convertTo(query_f, CV_32F);

    for (int i = 0; i < centroids.rows; ++i) {
        float dist = cv::norm(query_f, centroids.row(i), cv::NORM_L2);
        cluster_distances.push_back({dist, i});
    }

    // 2. Ταξινομούμε τα clusters για να βρούμε τα 'num_probes' (W) πιο κοντινά
    std::sort(cluster_distances.begin(), cluster_distances.end());

    std::vector<std::string> candidates;
    int probes_to_check = std::min(num_probes, centroids.rows);
    
    // 3. Μαζεύουμε τους υποψήφιους ΜΟΝΟ από αυτά τα W clusters
    for (int p = 0; p < probes_to_check; ++p) {
        int cluster_idx = cluster_distances[p].second;
        for (const auto& img_id : inverted_index[cluster_idx]) {
            candidates.push_back(img_id);
        }
    }

    checked_candidates = candidates.size();

    // 4. Ακριβής υπολογισμός Ευκλείδειας απόστασης μόνο για τους επιλεγμένους
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