#include "../inc/sift_extractor.h"
#include <iostream>
#include <random>
#include <algorithm>

SiftExtractor::SiftExtractor() {
    // Δημιουργία του SIFT extractor της OpenCV
    sift = cv::SIFT::create();
}

bool SiftExtractor::extract_descriptors(const std::string& image_path, std::vector<cv::Mat>& descriptors_out) {
    // Ανάγνωση εικόνας σε grayscale (υποστηρίζει ppm και jpg)[cite: 2]
    cv::Mat img = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        // Ορισμένες εικόνες μπορεί να μην ανοίγουν ή να απορρίπτονται
        return false;
    }

    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;
    
    // Εξαγωγή SIFT keypoints και descriptors
    sift->detectAndCompute(img, cv::noArray(), keypoints, descriptors);

    if (descriptors.empty()) {
        return false; // Αν δεν βρεθούν descriptors
    }

    // Μετατροπή του cv::Mat σε vector από cv::Mat (κάθε γραμμή είναι ένας descriptor 128 διαστάσεων)
    descriptors_out.clear();
    for (int i = 0; i < descriptors.rows; ++i) {
        descriptors_out.push_back(descriptors.row(i).clone());
    }

    return true;
}

std::vector<cv::Mat> SiftExtractor::sample_descriptors(const std::vector<std::vector<cv::Mat>>& all_img_descriptors, int max_S, int seed) {
    std::vector<cv::Mat> pooled_descriptors;

    // Συγκεντρώνουμε όλους τους descriptors από όλες τις εικόνες του training set
    for (const auto& img_desc : all_img_descriptors) {
        if (img_desc.size() <= (size_t)max_S) {
            // Αν η εικόνα έχει λιγότερους από S, τους παίρνουμε όλους[cite: 2]
            pooled_descriptors.insert(pooled_descriptors.end(), img_desc.begin(), img_desc.end());
        } else {
            // Διαφορετικά, επιλέγουμε τυχαία έως S descriptors[cite: 1, 2]
            std::vector<cv::Mat> temp = img_desc;
            std::mt19937 g(seed);
            std::shuffle(temp.begin(), temp.end(), g);
            
            for (int i = 0; i < max_S; ++i) {
                pooled_descriptors.push_back(temp[i]);
            }
        }
    }

    return pooled_descriptors;
}