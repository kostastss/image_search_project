#ifndef SIFT_EXTRACTOR_H
#define SIFT_EXTRACTOR_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

class SiftExtractor {
private:
    cv::Ptr<cv::SIFT> sift;

public:
    SiftExtractor();

    // Εξαγωγή SIFT descriptors από μια εικόνα
    bool extract_descriptors(const std::string& image_path, std::vector<cv::Mat>& descriptors_out);

    // Τυχαία επιλογή (sampling) έως S descriptors από ένα σύνολο descriptors (για το training set)
    std::vector<cv::Mat> sample_descriptors(const std::vector<std::vector<cv::Mat>>& all_img_descriptors, int max_S, int seed);
};

#endif // SIFT_EXTRACTOR_H