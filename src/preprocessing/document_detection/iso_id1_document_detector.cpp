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
 * that follow the AAMVA DL/ID Card Design Standard and ISO/IEC 18013 specifications.
 */

#include "iso_id1_document_detector.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <algorithm>

namespace id_reader {
namespace preprocessing {

ISO_ID1_DocumentDetector::ISO_ID1_DocumentDetector() {
    // Tuned parameters for ISO/IEC 7810 ID-1 format documents
    canny_threshold1_ = 10;         // Much lower for subtle edges
    canny_threshold2_ = 30;         // Lower thresholds for small images
    min_contour_area_ratio_ = 0.002; // 0.2% of image area minimum
    max_contour_area_ratio_ = 0.99;  // 99% of image area maximum (allow nearly full image for cropped documents)
    approx_epsilon_factor_ = 0.01;   // More precise for rounded corners
    aspect_ratio_tolerance_ = 0.4;   // 40% tolerance for perspective distortion and photo angles
    
    // ISO/IEC 7810 ID-1 standard aspect ratio (width/height)
    target_aspect_ratio_ = 1.586;   // ID-1 format: 85.6mm x 53.98mm (credit card size)
}

ISO_ID1_DocumentDetector::~ISO_ID1_DocumentDetector() = default;

bool ISO_ID1_DocumentDetector::detectDocument(const cv::Mat& input_image, DocumentBounds& bounds) {
    if (input_image.empty()) {
        return false;
    }

    // Adaptive scaling for modern smartphone cameras (often 4K+ resolution)
    cv::Mat working_image;
    double scale_factor = 1.0;
    
    // Determine optimal working resolution based on input size
    int target_width = 1200;  // Sweet spot for detection accuracy vs performance
    if (input_image.cols > target_width) {
        scale_factor = static_cast<double>(target_width) / input_image.cols;
        cv::resize(input_image, working_image, cv::Size(0, 0), scale_factor, scale_factor, cv::INTER_AREA);
    } else {
        working_image = input_image.clone();
    }

    // Adaptive parameters based on image size
    adaptParametersForImageSize(working_image.size());

    // Multi-stage detection approach for ID-1 format documents
    cv::Mat preprocessed;
    if (!preprocessImageForID1Document(working_image, preprocessed)) {
        return false;
    }

    std::vector<std::vector<cv::Point>> contours;
    if (!findDocumentContours(preprocessed, contours)) {
        return false;
    }

    std::vector<cv::Point> best_contour;
    if (!findBestDocumentContour(contours, working_image.size(), best_contour)) {
        return false;
    }

    // Scale contour back to original image size if we scaled down
    if (scale_factor != 1.0) {
        for (auto& point : best_contour) {
            point.x = static_cast<int>(point.x / scale_factor);
            point.y = static_cast<int>(point.y / scale_factor);
        }
    }

    return extractDocumentBounds(best_contour, input_image.size(), bounds);
}

bool ISO_ID1_DocumentDetector::preprocessImageForID1Document(const cv::Mat& input, cv::Mat& output) {
    cv::Mat gray, enhanced, blurred, edges;
    
    // Convert to grayscale
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else if (input.channels() == 4) {
        cv::cvtColor(input, gray, cv::COLOR_BGRA2GRAY);
    } else {
        gray = input.clone();
    }
    
    // Enhanced contrast processing for varying lighting conditions
    cv::Mat enhanced;
    cv::createCLAHE(2.0, cv::Size(8, 8))->apply(gray, enhanced);
    
    // Adaptive blurring based on image size
    int blur_size = std::max(3, static_cast<int>(std::min(input.rows, input.cols) / 400));
    if (blur_size % 2 == 0) blur_size++; // Ensure odd number
    cv::GaussianBlur(enhanced, blurred, cv::Size(blur_size, blur_size), 0);
    
    // Adaptive Canny thresholding based on image statistics
    cv::Scalar mean_val, stddev_val;
    cv::meanStdDev(blurred, mean_val, stddev_val);
    double dynamic_low = std::max(10.0, mean_val[0] - stddev_val[0]);
    double dynamic_high = std::min(200.0, mean_val[0] + stddev_val[0]);
    
    cv::Canny(blurred, edges, dynamic_low, dynamic_high);
    
    // Adaptive morphological operations based on image size
    int kernel_size = std::max(2, static_cast<int>(std::min(input.rows, input.cols) / 800));
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernel_size, kernel_size));
    cv::morphologyEx(edges, output, cv::MORPH_CLOSE, kernel);
    
    return true;
}

bool ISO_ID1_DocumentDetector::findDocumentContours(const cv::Mat& edge_image, std::vector<std::vector<cv::Point>>& contours) {
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(edge_image, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    if (contours.empty()) {
        return false;
    }
    
    // Calculate dynamic area thresholds based on image size
    double image_area = edge_image.rows * edge_image.cols;
    double min_area = image_area * min_contour_area_ratio_;
    double max_area = image_area * max_contour_area_ratio_;
    
    // Filter contours by area and remove edge-touching contours
    contours.erase(
        std::remove_if(contours.begin(), contours.end(),
            [min_area, max_area, &edge_image](const std::vector<cv::Point>& contour) {
                double area = cv::contourArea(contour);
                if (area < min_area || area > max_area) {
                    return true;
                }
                
                // For real photos, allow edge-touching contours since photos are often cropped tight
                // For perfectly cropped documents, the boundary IS the document
                cv::Rect bounds = cv::boundingRect(contour);
                
                // Only reject full-image contours if they don't have document-like aspect ratio
                if (bounds.x == 0 && bounds.y == 0 && 
                    bounds.width == edge_image.cols && bounds.height == edge_image.rows) {
                    double aspect = static_cast<double>(bounds.width) / bounds.height;
                    // If it looks like a document aspect ratio, keep it (it's probably a cropped document)
                    if (aspect >= 1.2 && aspect <= 2.2) {
                        return false;  // Keep this contour
                    }
                    return true;  // Reject non-document aspect ratios
                }
                
                return false;
            }),
        contours.end()
    );
    
    return !contours.empty();
}

bool ISO_ID1_DocumentDetector::findBestDocumentContour(const std::vector<std::vector<cv::Point>>& contours, 
                                                   const cv::Size& image_size,
                                                   std::vector<cv::Point>& best_contour) {
    double best_score = 0;
    int best_contour_index = -1;
    
    for (size_t i = 0; i < contours.size(); ++i) {
        // Approximate contour to reduce points
        std::vector<cv::Point> approx;
        double epsilon = approx_epsilon_factor_ * cv::arcLength(contours[i], true);
        cv::approxPolyDP(contours[i], approx, epsilon, true);
        
        // Calculate score for this contour
        double score = calculateDocumentScore(approx, image_size);
        
        if (score > best_score) {
            best_score = score;
            best_contour_index = i;
            best_contour = approx;
        }
    }
    
    return best_contour_index != -1 && best_score > 0.1; // Lower minimum score threshold
}

double ISO_ID1_DocumentDetector::calculateDocumentScore(const std::vector<cv::Point>& contour, const cv::Size& image_size) {
    if (contour.size() < 4) {
        return 0.0;
    }
    
    double score = 0.0;
    
    // 1. Area score (documents should be reasonable portion of image)
    double area = cv::contourArea(contour);
    double image_area = image_size.width * image_size.height;
    double area_ratio = area / image_area;
    
    double area_score = 0.0;
    if (area_ratio >= 0.002 && area_ratio <= 0.99) {
        if (area_ratio >= 0.01 && area_ratio <= 0.7) {
            // Optimal range for document within larger image
            area_score = 1.0;
        } else if (area_ratio > 0.85) {
            // High score for perfectly cropped documents (like our sample)
            area_score = 0.9;
        } else {
            // Reduced score but still acceptable
            area_score = 0.5;
        }
    }
    score += area_score * 0.25;
    
    // 2. Aspect ratio score (ISO/IEC 7810 ID-1 standard: 1.586:1)
    cv::Rect bounds = cv::boundingRect(contour);
    double aspect_ratio = static_cast<double>(bounds.width) / bounds.height;
    double aspect_diff = std::abs(aspect_ratio - target_aspect_ratio_) / target_aspect_ratio_;
    
    double aspect_score = 0.0;
    if (aspect_diff <= aspect_ratio_tolerance_) {
        aspect_score = 1.0 - aspect_diff / aspect_ratio_tolerance_;
    }
    score += aspect_score * 0.4;  // Increase weight of aspect ratio since it's most reliable
    
    // 3. Shape regularity score (prefer rectangular shapes)
    double shape_score = 0.0;
    if (contour.size() == 4) {
        shape_score = 1.0; // Perfect quadrilateral
    } else if (contour.size() >= 4 && contour.size() <= 8) {
        shape_score = 0.8; // Close to rectangular
    } else if (contour.size() > 8 && contour.size() <= 12) {
        shape_score = 0.5; // Rounded rectangle
    }
    score += shape_score * 0.15;  // Reduce weight since many real documents have rounded corners
    
    // 4. Position score (documents are usually centered, not at edges)
    cv::Point2f center;
    float radius;
    cv::minEnclosingCircle(contour, center, radius);
    
    cv::Point2f image_center(image_size.width / 2.0f, image_size.height / 2.0f);
    double distance_from_center = cv::norm(center - image_center);
    double max_distance = cv::norm(cv::Point2f(image_size.width / 2.0f, image_size.height / 2.0f));
    double position_score = 1.0 - (distance_from_center / max_distance);
    score += position_score * 0.2;  // Keep position weight the same
    
    return score;
}

bool ISO_ID1_DocumentDetector::extractDocumentBounds(const std::vector<cv::Point>& contour, 
                                                 const cv::Size& image_size,
                                                 DocumentBounds& bounds) {
    if (contour.size() < 4) {
        return false;
    }
    
    std::vector<cv::Point> corners;
    if (contour.size() == 4) {
        corners = contour;
    } else {
        // For rounded rectangles, find the 4 corner points
        corners = findCornerPoints(contour);
    }
    
    if (corners.size() != 4) {
        return false;
    }
    
    // Sort corners consistently (top-left, top-right, bottom-right, bottom-left)
    std::vector<cv::Point> sorted_corners = sortCornerPoints(corners);
    
    // Convert to normalized coordinates
    bounds.x1 = static_cast<float>(sorted_corners[0].x) / image_size.width;
    bounds.y1 = static_cast<float>(sorted_corners[0].y) / image_size.height;
    bounds.x2 = static_cast<float>(sorted_corners[1].x) / image_size.width;
    bounds.y2 = static_cast<float>(sorted_corners[1].y) / image_size.height;
    bounds.x3 = static_cast<float>(sorted_corners[2].x) / image_size.width;
    bounds.y3 = static_cast<float>(sorted_corners[2].y) / image_size.height;
    bounds.x4 = static_cast<float>(sorted_corners[3].x) / image_size.width;
    bounds.y4 = static_cast<float>(sorted_corners[3].y) / image_size.height;
    
    // Calculate confidence based on document score
    bounds.confidence = calculateDocumentScore(contour, image_size);
    
    return true;
}

std::vector<cv::Point> ISO_ID1_DocumentDetector::findCornerPoints(const std::vector<cv::Point>& contour) {
    if (contour.size() <= 4) {
        return contour;
    }
    
    // Find convex hull to simplify shape
    std::vector<cv::Point> hull;
    cv::convexHull(contour, hull);
    
    if (hull.size() <= 4) {
        return hull;
    }
    
    // Use Douglas-Peucker algorithm to find corner points
    std::vector<cv::Point> corners;
    double epsilon = 0.02 * cv::arcLength(hull, true);
    cv::approxPolyDP(hull, corners, epsilon, true);
    
    // If we still have too many points, take the 4 most separated ones
    if (corners.size() > 4) {
        corners = selectFourCorners(corners);
    }
    
    return corners;
}

std::vector<cv::Point> ISO_ID1_DocumentDetector::selectFourCorners(const std::vector<cv::Point>& points) {
    if (points.size() <= 4) {
        return points;
    }
    
    // Find the 4 extreme points (leftmost, rightmost, topmost, bottommost)
    auto leftmost = std::min_element(points.begin(), points.end(),
        [](const cv::Point& a, const cv::Point& b) { return a.x < b.x; });
    auto rightmost = std::max_element(points.begin(), points.end(),
        [](const cv::Point& a, const cv::Point& b) { return a.x < b.x; });
    auto topmost = std::min_element(points.begin(), points.end(),
        [](const cv::Point& a, const cv::Point& b) { return a.y < b.y; });
    auto bottommost = std::max_element(points.begin(), points.end(),
        [](const cv::Point& a, const cv::Point& b) { return a.y < b.y; });
    
    std::vector<cv::Point> corners = {*leftmost, *rightmost, *topmost, *bottommost};
    
    // Remove duplicates and ensure we have 4 unique points
    std::sort(corners.begin(), corners.end(), 
              [](const cv::Point& a, const cv::Point& b) {
                  return a.x < b.x || (a.x == b.x && a.y < b.y);
              });
    corners.erase(std::unique(corners.begin(), corners.end()), corners.end());
    
    if (corners.size() >= 4) {
        corners.resize(4);
    }
    
    return corners;
}

std::vector<cv::Point> ISO_ID1_DocumentDetector::sortCornerPoints(const std::vector<cv::Point>& points) {
    if (points.size() != 4) {
        return points;
    }
    
    // Calculate centroid
    cv::Point centroid(0, 0);
    for (const auto& point : points) {
        centroid.x += point.x;
        centroid.y += point.y;
    }
    centroid.x /= 4;
    centroid.y /= 4;
    
    // Sort points by angle from centroid
    std::vector<std::pair<cv::Point, double>> point_angles;
    for (const auto& point : points) {
        double angle = std::atan2(point.y - centroid.y, point.x - centroid.x);
        point_angles.push_back({point, angle});
    }
    
    std::sort(point_angles.begin(), point_angles.end(),
              [](const std::pair<cv::Point, double>& a, const std::pair<cv::Point, double>& b) {
                  return a.second < b.second;
              });
    
    // Order: top-left, top-right, bottom-right, bottom-left
    std::vector<cv::Point> sorted_points(4);
    
    // Find which point is closest to each corner
    cv::Point image_tl(0, 0); // Assume image starts at (0,0)
    
    double min_dist_tl = std::numeric_limits<double>::max();
    int tl_idx = 0;
    
    for (int i = 0; i < 4; i++) {
        double dist = cv::norm(point_angles[i].first - image_tl);
        if (dist < min_dist_tl) {
            min_dist_tl = dist;
            tl_idx = i;
        }
    }
    
    // Arrange in clockwise order starting from top-left
    for (int i = 0; i < 4; i++) {
        sorted_points[i] = point_angles[(tl_idx + i) % 4].first;
    }
    
    return sorted_points;
}

void ISO_ID1_DocumentDetector::setCannyThresholds(double threshold1, double threshold2) {
    canny_threshold1_ = threshold1;
    canny_threshold2_ = threshold2;
}

void ISO_ID1_DocumentDetector::setAreaRatios(double min_ratio, double max_ratio) {
    min_contour_area_ratio_ = min_ratio;
    max_contour_area_ratio_ = max_ratio;
}

void ISO_ID1_DocumentDetector::setTargetAspectRatio(double ratio, double tolerance) {
    target_aspect_ratio_ = ratio;
    aspect_ratio_tolerance_ = tolerance;
}

void ISO_ID1_DocumentDetector::adaptParametersForImageSize(const cv::Size& image_size) {
    int min_dimension = std::min(image_size.width, image_size.height);
    int max_dimension = std::max(image_size.width, image_size.height);
    
    // Adaptive thresholds based on image resolution
    if (min_dimension < 400) {
        // Low resolution images (older phones, web cameras)
        canny_threshold1_ = 30;
        canny_threshold2_ = 90;
        min_contour_area_ratio_ = 0.05;  // Allow larger minimum area for small images
        max_contour_area_ratio_ = 0.95;
        approx_epsilon_factor_ = 0.02;   // Less precise for low-res
        aspect_ratio_tolerance_ = 0.5;   // More tolerance for low quality
    } else if (min_dimension < 800) {
        // Medium resolution (1-2MP, older smartphones)
        canny_threshold1_ = 25;
        canny_threshold2_ = 75;
        min_contour_area_ratio_ = 0.01;
        max_contour_area_ratio_ = 0.90;
        approx_epsilon_factor_ = 0.015;
        aspect_ratio_tolerance_ = 0.4;
    } else if (min_dimension < 1500) {
        // High resolution (4-8MP, most modern phones)
        canny_threshold1_ = 20;
        canny_threshold2_ = 60;
        min_contour_area_ratio_ = 0.005;
        max_contour_area_ratio_ = 0.85;
        approx_epsilon_factor_ = 0.01;
        aspect_ratio_tolerance_ = 0.35;
    } else {
        // Very high resolution (12MP+, flagship phones, professional cameras)
        canny_threshold1_ = 15;
        canny_threshold2_ = 45;
        min_contour_area_ratio_ = 0.002;
        max_contour_area_ratio_ = 0.80;
        approx_epsilon_factor_ = 0.008;
        aspect_ratio_tolerance_ = 0.3;
    }
    
    // Additional adjustments for very wide images (panoramic or landscape orientation)
    double aspect_ratio = static_cast<double>(max_dimension) / min_dimension;
    if (aspect_ratio > 2.5) {
        // Likely a wide landscape photo, document might be smaller in frame
        min_contour_area_ratio_ *= 0.5;
        aspect_ratio_tolerance_ *= 1.2;
    }
}

} // namespace preprocessing
} // namespace id_reader