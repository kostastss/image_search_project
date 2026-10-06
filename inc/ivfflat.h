#ifndef IVFFLAT_H
#define IVFFLAT_H

#include <vector>
#include <string>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include "search_engine.h"

class IVFFlat {
private:
    int num_clusters; // Πλήθος clusters για τη βάση δεδομένων (Kc)
    int num_probes;   // Σε πόσα κοντινότερα clusters θα ψάξουμε κατά το query (W)

    cv::Mat centroids; // Τα κέντρα των clusters
    // Το ανεστραμμένο ευρετήριο: Cluster ID -> Λίστα από Image IDs
    std::vector<std::vector<std::string>> inverted_index;
    
    std::unordered_map<std::string, cv::Mat> db_histograms;
    
    // Προσωρινή αποθήκευση για την εκπαίδευση του K-means
    std::vector<cv::Mat> raw_histograms;
    std::vector<std::string> raw_db_ids;

public:
    // Βάζουμε προεπιλογή 40 clusters και ψάχνουμε στα 4 πιο κοντινά
    IVFFlat(int clusters = 40, int probes = 4);
    
    void add_to_database(const std::string& image_id, const cv::Mat& histogram);
    
    // Πρέπει να κληθεί ΑΦΟΥ μπουν όλες οι εικόνες στη βάση, για να γίνει το clustering
    void build_index(); 
    
    std::vector<SearchResult> search(const cv::Mat& query_hist, int top_k, int& checked_candidates) const;
};

#endif // IVFFLAT_H