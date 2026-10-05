#include "../inc/kmeans_vocab.h"
#include <iostream>

using namespace cv;

KMeansVocab::KMeansVocab(int k, int max_iterations, double eps) 
    : vocab_size(k), max_iter(max_iterations), epsilon(eps) {}

bool KMeansVocab::train(const std::vector<cv::Mat>& training_descriptors) {
    if (training_descriptors.empty()) {
        std::cerr << "Σφάλμα: Δεν υπάρχουν descriptors για την εκπαίδευση του k-means." << std::endl;
        return false;
    }

    // Μετατροπή του vector από cv::Mat σε έναν ενιαίο πίνακα cv::Mat (Row x 128)
    int total_desc = training_descriptors.size();
    cv::Mat samples(total_desc, 128, CV_32F);
    
    for (int i = 0; i < total_desc; ++i) {
        training_descriptors[i].copyTo(samples.row(i));
    }

    std::cout << "[KMeans] Εκκίνηση k-means clustering για K = " << vocab_size 
              << " με σύνολο " << total_desc << " descriptors..." << std::endl;

    cv::Mat labels;
    cv::TermCriteria criteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, max_iter, epsilon);    
    
    // Εκτέλεση k-means της OpenCV
    cv::kmeans(samples, vocab_size, labels, criteria, 3, cv::KMEANS_PP_CENTERS, centers);

    std::cout << "[KMeans] Η εκπαίδευση ολοκληρώθηκε επιτυχώς!" << std::endl;
    return true;
}

cv::Mat KMeansVocab::compute_bow_histogram(const std::vector<cv::Mat>& image_descriptors) const {
    // Δημιουργία ιστογράμματος μηδενικών διαστάσεων K
    cv::Mat histogram = cv::Mat::zeros(1, vocab_size, CV_32F);

    if (image_descriptors.empty() || centers.empty()) {
        return histogram;
    }

    // Για κάθε descriptor της εικόνας, βρίσκουμε το πλησιέστερο visual word (cluster center)
    for (const auto& desc : image_descriptors) {
        float min_dist = std::numeric_limits<float>::max();
        int best_cluster = 0;

        for (int i = 0; i < vocab_size; ++i) {
            // Υπολογισμός Ευκλείδειας απόστασης
            float dist = cv::norm(desc, centers.row(i), cv::NORM_L2);
            if (dist < min_dist) {
                min_dist = dist;
                best_cluster = i;
            }
        }

        // Αυξάνουμε το αντίστοιχό "bin" στο ιστόγραμμα
        histogram.at<float>(0, best_cluster) += 1.0f;
    }

    // Κανονικοποίηση ιστογράμματος (L2 norm) για καλύτερα αποτελέσματα ανάκτησης
    cv::normalize(histogram, histogram, 1, 0, cv::NORM_L1);
    
    return histogram;
}