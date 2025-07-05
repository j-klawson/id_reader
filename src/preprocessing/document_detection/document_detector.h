/*
 * Universal ID Reader - Cross-platform ID document scanner
 * Copyright (C) 2025 J. Keith Lawson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef ID_READER_DOCUMENT_DETECTOR_H
#define ID_READER_DOCUMENT_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <vector>

namespace id_reader {
namespace preprocessing {

struct DocumentBounds {
    float x1, y1;  // Top-left corner (normalized coordinates 0-1)
    float x2, y2;  // Top-right corner
    float x3, y3;  // Bottom-right corner
    float x4, y4;  // Bottom-left corner
    float confidence;  // Detection confidence (0-1)
    
    DocumentBounds() : x1(0), y1(0), x2(0), y2(0), x3(0), y3(0), x4(0), y4(0), confidence(0) {}
};

class DocumentDetector {
public:
    DocumentDetector();
    ~DocumentDetector();
    
    // Main detection function
    bool detectDocument(const cv::Mat& input_image, DocumentBounds& bounds);
    
    // Configuration methods
    void setCannyThresholds(double threshold1, double threshold2);
    void setContourAreaRange(double min_area, double max_area);
    void setApproximationEpsilon(double epsilon_factor);
    
private:
    // Image preprocessing
    bool preprocessImage(const cv::Mat& input, cv::Mat& output);
    
    // Contour detection and filtering
    bool findContours(const cv::Mat& edge_image, std::vector<std::vector<cv::Point>>& contours);
    bool findBestDocumentContour(const std::vector<std::vector<cv::Point>>& contours, 
                                 std::vector<cv::Point>& best_contour);
    
    // Document bounds extraction
    bool extractDocumentBounds(const std::vector<cv::Point>& contour, 
                              const cv::Size& image_size,
                              DocumentBounds& bounds);
    
    // Helper methods
    std::vector<cv::Point> sortCornerPoints(const std::vector<cv::Point>& points);
    float calculateConfidence(const std::vector<cv::Point>& contour, const cv::Size& image_size);
    
    // Detection parameters
    double canny_threshold1_;
    double canny_threshold2_;
    double min_contour_area_;
    double max_contour_area_;
    double approx_epsilon_factor_;
};

} // namespace preprocessing
} // namespace id_reader

#endif // ID_READER_DOCUMENT_DETECTOR_H