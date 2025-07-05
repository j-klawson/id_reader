/*
 * Universal ID Reader - Cross-platform ID document scanner
 * Copyright (C) 2025 J. Keith Lawson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "document_detector.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <algorithm>

namespace id_reader {
namespace preprocessing {

DocumentDetector::DocumentDetector() {
    // Default parameters for document detection
    canny_threshold1_ = 50;
    canny_threshold2_ = 150;
    min_contour_area_ = 10000;
    max_contour_area_ = 500000;
    approx_epsilon_factor_ = 0.02;
}

DocumentDetector::~DocumentDetector() = default;

bool DocumentDetector::detectDocument(const cv::Mat& input_image, DocumentBounds& bounds) {
    if (input_image.empty()) {
        return false;
    }

    cv::Mat preprocessed;
    if (!preprocessImage(input_image, preprocessed)) {
        return false;
    }

    std::vector<std::vector<cv::Point>> contours;
    if (!findContours(preprocessed, contours)) {
        return false;
    }

    std::vector<cv::Point> best_contour;
    if (!findBestDocumentContour(contours, best_contour)) {
        return false;
    }

    return extractDocumentBounds(best_contour, input_image.size(), bounds);
}

bool DocumentDetector::preprocessImage(const cv::Mat& input, cv::Mat& output) {
    cv::Mat gray, blurred, edges;
    
    // Convert to grayscale
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else if (input.channels() == 4) {
        cv::cvtColor(input, gray, cv::COLOR_BGRA2GRAY);
    } else {
        gray = input.clone();
    }
    
    // Apply Gaussian blur to reduce noise
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);
    
    // Edge detection using Canny
    cv::Canny(blurred, edges, canny_threshold1_, canny_threshold2_);
    
    // Morphological operations to close gaps in edges
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(edges, output, cv::MORPH_CLOSE, kernel);
    
    return true;
}

bool DocumentDetector::findContours(const cv::Mat& edge_image, std::vector<std::vector<cv::Point>>& contours) {
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(edge_image, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    if (contours.empty()) {
        return false;
    }
    
    // Filter contours by area
    contours.erase(
        std::remove_if(contours.begin(), contours.end(),
            [this](const std::vector<cv::Point>& contour) {
                double area = cv::contourArea(contour);
                return area < min_contour_area_ || area > max_contour_area_;
            }),
        contours.end()
    );
    
    return !contours.empty();
}

bool DocumentDetector::findBestDocumentContour(const std::vector<std::vector<cv::Point>>& contours, 
                                               std::vector<cv::Point>& best_contour) {
    double max_area = 0;
    int best_contour_index = -1;
    
    for (size_t i = 0; i < contours.size(); ++i) {
        // Approximate contour to reduce number of points
        std::vector<cv::Point> approx;
        double epsilon = approx_epsilon_factor_ * cv::arcLength(contours[i], true);
        cv::approxPolyDP(contours[i], approx, epsilon, true);
        
        // Look for quadrilaterals (4 corners)
        if (approx.size() == 4) {
            double area = cv::contourArea(approx);
            if (area > max_area) {
                max_area = area;
                best_contour_index = i;
                best_contour = approx;
            }
        }
    }
    
    // If no quadrilateral found, use the largest contour
    if (best_contour_index == -1 && !contours.empty()) {
        for (size_t i = 0; i < contours.size(); ++i) {
            double area = cv::contourArea(contours[i]);
            if (area > max_area) {
                max_area = area;
                best_contour_index = i;
            }
        }
        
        if (best_contour_index != -1) {
            // Approximate the largest contour
            double epsilon = approx_epsilon_factor_ * cv::arcLength(contours[best_contour_index], true);
            cv::approxPolyDP(contours[best_contour_index], best_contour, epsilon, true);
        }
    }
    
    return best_contour_index != -1;
}

bool DocumentDetector::extractDocumentBounds(const std::vector<cv::Point>& contour, 
                                             const cv::Size& image_size,
                                             DocumentBounds& bounds) {
    if (contour.size() < 4) {
        return false;
    }
    
    // If we have exactly 4 points, use them directly
    if (contour.size() == 4) {
        // Sort points to get consistent ordering (top-left, top-right, bottom-right, bottom-left)
        std::vector<cv::Point> sorted_points = sortCornerPoints(contour);
        
        bounds.x1 = static_cast<float>(sorted_points[0].x) / image_size.width;
        bounds.y1 = static_cast<float>(sorted_points[0].y) / image_size.height;
        bounds.x2 = static_cast<float>(sorted_points[1].x) / image_size.width;
        bounds.y2 = static_cast<float>(sorted_points[1].y) / image_size.height;
        bounds.x3 = static_cast<float>(sorted_points[2].x) / image_size.width;
        bounds.y3 = static_cast<float>(sorted_points[2].y) / image_size.height;
        bounds.x4 = static_cast<float>(sorted_points[3].x) / image_size.width;
        bounds.y4 = static_cast<float>(sorted_points[3].y) / image_size.height;
    } else {
        // Find bounding rectangle and use its corners
        cv::Rect bounding_rect = cv::boundingRect(contour);
        
        bounds.x1 = static_cast<float>(bounding_rect.x) / image_size.width;
        bounds.y1 = static_cast<float>(bounding_rect.y) / image_size.height;
        bounds.x2 = static_cast<float>(bounding_rect.x + bounding_rect.width) / image_size.width;
        bounds.y2 = static_cast<float>(bounding_rect.y) / image_size.height;
        bounds.x3 = static_cast<float>(bounding_rect.x + bounding_rect.width) / image_size.width;
        bounds.y3 = static_cast<float>(bounding_rect.y + bounding_rect.height) / image_size.height;
        bounds.x4 = static_cast<float>(bounding_rect.x) / image_size.width;
        bounds.y4 = static_cast<float>(bounding_rect.y + bounding_rect.height) / image_size.height;
    }
    
    // Calculate confidence based on contour properties
    bounds.confidence = calculateConfidence(contour, image_size);
    
    return true;
}

std::vector<cv::Point> DocumentDetector::sortCornerPoints(const std::vector<cv::Point>& points) {
    if (points.size() != 4) {
        return points;
    }
    
    std::vector<cv::Point> sorted_points(4);
    
    // Calculate centroid
    cv::Point centroid(0, 0);
    for (const auto& point : points) {
        centroid.x += point.x;
        centroid.y += point.y;
    }
    centroid.x /= 4;
    centroid.y /= 4;
    
    // Sort points by their position relative to centroid
    std::vector<std::pair<cv::Point, int>> point_quadrants;
    for (const auto& point : points) {
        int quadrant = 0;
        if (point.x >= centroid.x && point.y < centroid.y) quadrant = 0; // top-right
        else if (point.x < centroid.x && point.y < centroid.y) quadrant = 1; // top-left
        else if (point.x < centroid.x && point.y >= centroid.y) quadrant = 2; // bottom-left
        else quadrant = 3; // bottom-right
        
        point_quadrants.push_back({point, quadrant});
    }
    
    std::sort(point_quadrants.begin(), point_quadrants.end(),
              [](const std::pair<cv::Point, int>& a, const std::pair<cv::Point, int>& b) {
                  return a.second < b.second;
              });
    
    // Reorder: top-left, top-right, bottom-right, bottom-left
    sorted_points[0] = point_quadrants[1].first; // top-left
    sorted_points[1] = point_quadrants[0].first; // top-right
    sorted_points[2] = point_quadrants[3].first; // bottom-right
    sorted_points[3] = point_quadrants[2].first; // bottom-left
    
    return sorted_points;
}

float DocumentDetector::calculateConfidence(const std::vector<cv::Point>& contour, const cv::Size& image_size) {
    if (contour.empty()) {
        return 0.0f;
    }
    
    double contour_area = cv::contourArea(contour);
    double image_area = image_size.width * image_size.height;
    double area_ratio = contour_area / image_area;
    
    // Confidence based on area ratio (documents should occupy reasonable portion of image)
    float area_confidence = 0.0f;
    if (area_ratio >= 0.1 && area_ratio <= 0.8) {
        area_confidence = 1.0f - std::abs(0.4f - static_cast<float>(area_ratio)) / 0.4f;
    }
    
    // Confidence based on contour approximation quality
    double perimeter = cv::arcLength(contour, true);
    std::vector<cv::Point> approx;
    cv::approxPolyDP(contour, approx, 0.02 * perimeter, true);
    
    float shape_confidence = 0.0f;
    if (approx.size() == 4) {
        shape_confidence = 1.0f;
    } else if (approx.size() >= 3 && approx.size() <= 6) {
        shape_confidence = 0.7f;
    } else {
        shape_confidence = 0.3f;
    }
    
    return (area_confidence + shape_confidence) / 2.0f;
}

void DocumentDetector::setCannyThresholds(double threshold1, double threshold2) {
    canny_threshold1_ = threshold1;
    canny_threshold2_ = threshold2;
}

void DocumentDetector::setContourAreaRange(double min_area, double max_area) {
    min_contour_area_ = min_area;
    max_contour_area_ = max_area;
}

void DocumentDetector::setApproximationEpsilon(double epsilon_factor) {
    approx_epsilon_factor_ = epsilon_factor;
}

} // namespace preprocessing
} // namespace id_reader