/*
 * Universal ID Reader - ISO/IEC 7810 ID-1 Document Detector
 * Copyright (C) 2025 J. Keith Lawson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Enhanced document detector optimized for ISO/IEC 7810 ID-1 format documents
 * including driver's licenses, national ID cards, and other standardized credentials
 * with complex security features, rounded corners, and holographic elements.
 */

#ifndef ID_READER_ISO_ID1_DOCUMENT_DETECTOR_H
#define ID_READER_ISO_ID1_DOCUMENT_DETECTOR_H

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

class ISO_ID1_DocumentDetector {
public:
    ISO_ID1_DocumentDetector();
    ~ISO_ID1_DocumentDetector();
    
    // Main detection function optimized for ISO/IEC 7810 ID-1 format documents
    bool detectDocument(const cv::Mat& input_image, DocumentBounds& bounds);
    
    // Configuration methods
    void setCannyThresholds(double threshold1, double threshold2);
    void setAreaRatios(double min_ratio, double max_ratio);
    void setTargetAspectRatio(double ratio, double tolerance);
    
private:
    // Enhanced preprocessing for ISO/IEC 7810 ID-1 documents
    bool preprocessImageForID1Document(const cv::Mat& input, cv::Mat& output);
    
    // Adaptive parameter adjustment based on image characteristics
    void adaptParametersForImageSize(const cv::Size& image_size);
    
    // Contour detection and filtering for ID-1 format documents
    bool findDocumentContours(const cv::Mat& edge_image, std::vector<std::vector<cv::Point>>& contours);
    bool findBestDocumentContour(const std::vector<std::vector<cv::Point>>& contours, 
                                 const cv::Size& image_size,
                                 std::vector<cv::Point>& best_contour);
    
    // Advanced scoring system for ID-1 document detection
    double calculateDocumentScore(const std::vector<cv::Point>& contour, const cv::Size& image_size);
    
    // Document bounds extraction with corner detection
    bool extractDocumentBounds(const std::vector<cv::Point>& contour, 
                              const cv::Size& image_size,
                              DocumentBounds& bounds);
    
    // Corner point detection for rounded rectangles
    std::vector<cv::Point> findCornerPoints(const std::vector<cv::Point>& contour);
    std::vector<cv::Point> selectFourCorners(const std::vector<cv::Point>& points);
    std::vector<cv::Point> sortCornerPoints(const std::vector<cv::Point>& points);
    
    // Detection parameters optimized for ISO/IEC 7810 ID-1 documents
    double canny_threshold1_;
    double canny_threshold2_;
    double min_contour_area_ratio_;  // Minimum area as ratio of image area
    double max_contour_area_ratio_;  // Maximum area as ratio of image area
    double approx_epsilon_factor_;
    double target_aspect_ratio_;     // ISO/IEC 7810 ID-1 aspect ratio (1.586)
    double aspect_ratio_tolerance_;  // Allowed deviation from target aspect ratio
};

} // namespace preprocessing
} // namespace id_reader

#endif // ID_READER_ISO_ID1_DOCUMENT_DETECTOR_H