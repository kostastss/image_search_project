#ifndef KMEANS_VOCAB_H
#define KMEANS_VOCAB_H

#include <vector>
#include <opencv2/opencv.hpp>

class KMeansVocab {
private:
    int vocab_size; // Το Κ (π.χ. 256, 512, κ.λπ.)
    int max_iter;
    double epsilon;
    cv::Mat centers; // Τα κέντρα των clusters (visual words)

public:
    KMeansVocab(int k, int max_iterations = 100, double eps = 1e-4);

    // Εκπαίδευση (clustering) με τους descriptors του training set
    bool train(const std::vector<cv::Mat>& training_descriptors);

    // Δημιουργία ιστογράμματος BoW (Bag of Words) για μια εικόνα
    cv::Mat compute_bow_histogram(const std::vector<cv::Mat>& image_descriptors) const;

    int get_vocab_size() const { return vocab_size; }
};

#endif // KMEANS_VOCAB_H