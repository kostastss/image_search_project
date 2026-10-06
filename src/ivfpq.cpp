#include "../inc/ivfpq.h"
#include <algorithm>
#include <iostream>

IVFPQ::IVFPQ(int clusters, int probes, int m_sub, int k_subcenters, int dimension) 
    : num_clusters(clusters), num_probes(probes), m(m_sub), k_sub(k_subcenters) {
    sub_dim = dimension / m;
    inverted_index.resize(num_clusters);
    pq_centroids.resize(m);
}

void IVFPQ::add_to_database(const std::string& image_id, const cv::Mat& histogram) {
    raw_histograms.push_back(histogram.clone());
    raw_db_ids.push_back(image_id);
}

void IVFPQ::build_index() {
    std::cout << "\n[IVF-PQ] Εκπαίδευση Coarse Quantizer (" << num_clusters << " clusters)..." << std::endl;
    cv::Mat data;
    cv::vconcat(raw_histograms, data);
    data.convertTo(data, CV_32F);

    cv::Mat labels;
    cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 100, 0.001);
    int k_coarse = std::min(num_clusters, data.rows);
    cv::kmeans(data, k_coarse, labels, criteria, 3, cv::KMEANS_PP_CENTERS, coarse_centroids);

    // Υπολογισμός residuals (Διαφορά της εικόνας από το κέντρο της)
    cv::Mat residuals = cv::Mat::zeros(data.size(), CV_32F);
    for (int i = 0; i < data.rows; ++i) {
        int cluster_idx = labels.at<int>(i);
        cv::subtract(data.row(i), coarse_centroids.row(cluster_idx), residuals.row(i));
    }

    std::cout << "[IVF-PQ] Εκπαίδευση Fine Quantizers (PQ σε " << m << " υπο-χώρους)..." << std::endl;
    for (int i = 0; i < m; ++i) {
        // Κόβουμε τον πίνακα στα αντίστοιχα m κομμάτια
        cv::Mat sub_data = residuals(cv::Rect(i * sub_dim, 0, sub_dim, residuals.rows));
        cv::Mat sub_labels, sub_centers;
        int k_actual = std::min(k_sub, sub_data.rows);
        cv::kmeans(sub_data, k_actual, sub_labels, criteria, 3, cv::KMEANS_PP_CENTERS, sub_centers);
        pq_centroids[i] = sub_centers;
    }

    std::cout << "[IVF-PQ] Κωδικοποίηση της Βάσης Δεδομένων..." << std::endl;
    for (int i = 0; i < data.rows; ++i) {
        int cluster_idx = labels.at<int>(i);
        PQCode item;
        item.image_id = raw_db_ids[i];
        
        // Αντιστοίχιση του residual στο κοντινότερο sub-centroid ανά υπο-χώρο
        for (int j = 0; j < m; ++j) {
            cv::Mat sub_res = residuals.row(i)(cv::Rect(j * sub_dim, 0, sub_dim, 1));
            int best_c = 0;
            float min_d = FLT_MAX;
            for (int c = 0; c < pq_centroids[j].rows; ++c) {
                float d = cv::norm(sub_res, pq_centroids[j].row(c), cv::NORM_L2);
                if (d < min_d) { min_d = d; best_c = c; }
            }
            item.codes.push_back((uint8_t)best_c);
        }
        inverted_index[cluster_idx].push_back(item);
    }

    raw_histograms.clear();
    raw_db_ids.clear();
    std::cout << "[IVF-PQ] Η κατασκευή του Index ολοκληρώθηκε!" << std::endl;
}

std::vector<SearchResult> IVFPQ::search(const cv::Mat& query_hist, int top_k, int& checked_candidates) const {
    if (coarse_centroids.empty()) return {};

    cv::Mat query_f;
    query_hist.convertTo(query_f, CV_32F);

    // 1. Βρίσκουμε τις W κοντινότερες γειτονιές
    std::vector<std::pair<float, int>> cluster_distances;
    for (int i = 0; i < coarse_centroids.rows; ++i) {
        float dist = cv::norm(query_f, coarse_centroids.row(i), cv::NORM_L2);
        cluster_distances.push_back({dist, i});
    }
    std::sort(cluster_distances.begin(), cluster_distances.end());

    int probes_to_check = std::min(num_probes, coarse_centroids.rows);
    std::vector<SearchResult> results;
    checked_candidates = 0;

    // 2. Αναζήτηση στα W clusters με ADC (Asymmetric Distance Computation)
    for (int p = 0; p < probes_to_check; ++p) {
        int c = cluster_distances[p].second;
        cv::Mat q_res;
        cv::subtract(query_f, coarse_centroids.row(c), q_res);

        // Precompute τον πίνακα αποστάσεων (Look-up Table) για ΑΣΤΡΑΠΙΑΙΟ υπολογισμό!
        std::vector<std::vector<float>> adc_table(m);
        for (int i = 0; i < m; ++i) {
            cv::Mat q_sub = q_res(cv::Rect(i * sub_dim, 0, sub_dim, 1));
            adc_table[i].resize(pq_centroids[i].rows);
            for (int j = 0; j < pq_centroids[i].rows; ++j) {
                adc_table[i][j] = cv::norm(q_sub, pq_centroids[i].row(j), cv::NORM_L2);
            }
        }

        // Τώρα οι αποστάσεις υπολογίζονται χωρίς Ευκλείδεια πράξη, μόνο με 8 αθροίσματα!
        for (const auto& item : inverted_index[c]) {
            float dist = 0;
            for (int i = 0; i < m; ++i) {
                dist += adc_table[i][item.codes[i]];
            }
            results.push_back({item.image_id, dist});
        }
        checked_candidates += inverted_index[c].size();
    }

    std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        return a.distance < b.distance;
    });

    if ((int)results.size() > top_k) results.resize(top_k);
    return results;
}