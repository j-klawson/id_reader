/*
 * Universal ID Reader - Cross-platform ID document scanner
 * Copyright (C) 2025 J. Keith Lawson
 *
 * Synthetic Test Image Generator
 * Creates test images that mimic real ID documents without containing
 * any personal information or real document features.
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <random>

class SyntheticTestGenerator {
public:
    // Standard document dimensions (in pixels for generation)
    struct DocumentSize {
        int width;
        int height;
        std::string name;
    };
    
    // Standard ID card sizes
    static const DocumentSize ID_CARD;           // 85.6 × 53.98mm (Credit card size)
    static const DocumentSize DRIVERS_LICENSE;   // Same as ID card
    static const DocumentSize PASSPORT_PAGE;     // 125 × 88mm (B7 size)
    
    SyntheticTestGenerator() : rng_(std::random_device{}()) {}
    
    // Generate a synthetic document image
    cv::Mat generateDocument(const DocumentSize& size, 
                            const cv::Scalar& bg_color = cv::Scalar(240, 240, 240),
                            const cv::Scalar& doc_color = cv::Scalar(255, 255, 255),
                            bool add_noise = false,
                            bool add_text_blocks = true);
    
    // Generate test scenarios with different challenges
    cv::Mat generateWithRotation(const DocumentSize& size, double angle_degrees);
    cv::Mat generateWithPerspective(const DocumentSize& size, float perspective_factor = 0.1f);
    cv::Mat generateWithLighting(const DocumentSize& size, float lighting_variation = 0.3f);
    cv::Mat generateWithBackground(const DocumentSize& size, const std::string& background_type);
    cv::Mat generateWithBlur(const DocumentSize& size, float blur_amount = 2.0f);
    
    // Generate batch test images
    void generateTestSuite(const std::string& output_dir);
    
private:
    std::mt19937 rng_;
    
    void addTextBlocks(cv::Mat& image, const cv::Rect& doc_area);
    void addNoise(cv::Mat& image, float intensity = 0.1f);
    void addLogo(cv::Mat& image, const cv::Rect& doc_area);
    cv::Mat createBackground(const cv::Size& size, const std::string& type);
};

// Document size definitions (scale factor applied for pixel dimensions)
const SyntheticTestGenerator::DocumentSize SyntheticTestGenerator::ID_CARD = {427, 270, "ID_Card"};
const SyntheticTestGenerator::DocumentSize SyntheticTestGenerator::DRIVERS_LICENSE = {427, 270, "Drivers_License"};
const SyntheticTestGenerator::DocumentSize SyntheticTestGenerator::PASSPORT_PAGE = {500, 352, "Passport_Page"};

cv::Mat SyntheticTestGenerator::generateDocument(const DocumentSize& size, 
                                                const cv::Scalar& bg_color,
                                                const cv::Scalar& doc_color,
                                                bool add_noise,
                                                bool add_text_blocks) {
    // Create background image (larger than document)
    cv::Size bg_size(size.width + 200, size.height + 200);
    cv::Mat image(bg_size, CV_8UC3, bg_color);
    
    // Calculate document position (centered)
    cv::Point doc_pos((bg_size.width - size.width) / 2, (bg_size.height - size.height) / 2);
    cv::Rect doc_rect(doc_pos, cv::Size(size.width, size.height));
    
    // Draw document rectangle
    cv::rectangle(image, doc_rect, doc_color, -1);
    
    // Add border
    cv::rectangle(image, doc_rect, cv::Scalar(180, 180, 180), 2);
    
    // Add content if requested
    if (add_text_blocks) {
        addTextBlocks(image, doc_rect);
    }
    
    // Add logo placeholder
    addLogo(image, doc_rect);
    
    // Add noise if requested
    if (add_noise) {
        addNoise(image);
    }
    
    return image;
}

cv::Mat SyntheticTestGenerator::generateWithRotation(const DocumentSize& size, double angle_degrees) {
    cv::Mat base_image = generateDocument(size);
    
    // Get rotation matrix
    cv::Point2f center(base_image.cols / 2.0f, base_image.rows / 2.0f);
    cv::Mat rotation_matrix = cv::getRotationMatrix2D(center, angle_degrees, 1.0);
    
    // Apply rotation
    cv::Mat rotated_image;
    cv::warpAffine(base_image, rotated_image, rotation_matrix, base_image.size());
    
    return rotated_image;
}

cv::Mat SyntheticTestGenerator::generateWithPerspective(const DocumentSize& size, float perspective_factor) {
    cv::Mat base_image = generateDocument(size);
    
    // Define source points (document corners in base image)
    cv::Point doc_pos((base_image.cols - size.width) / 2, (base_image.rows - size.height) / 2);
    std::vector<cv::Point2f> src_points = {
        cv::Point2f(doc_pos.x, doc_pos.y),                           // Top-left
        cv::Point2f(doc_pos.x + size.width, doc_pos.y),              // Top-right
        cv::Point2f(doc_pos.x + size.width, doc_pos.y + size.height), // Bottom-right
        cv::Point2f(doc_pos.x, doc_pos.y + size.height)              // Bottom-left
    };
    
    // Define destination points (with perspective distortion)
    float offset = size.width * perspective_factor;
    std::vector<cv::Point2f> dst_points = {
        cv::Point2f(doc_pos.x + offset, doc_pos.y),                  // Top-left
        cv::Point2f(doc_pos.x + size.width - offset, doc_pos.y),     // Top-right
        cv::Point2f(doc_pos.x + size.width, doc_pos.y + size.height), // Bottom-right
        cv::Point2f(doc_pos.x, doc_pos.y + size.height)              // Bottom-left
    };
    
    // Apply perspective transformation
    cv::Mat perspective_matrix = cv::getPerspectiveTransform(src_points, dst_points);
    cv::Mat perspective_image;
    cv::warpPerspective(base_image, perspective_image, perspective_matrix, base_image.size());
    
    return perspective_image;
}

cv::Mat SyntheticTestGenerator::generateWithLighting(const DocumentSize& size, float lighting_variation) {
    cv::Mat base_image = generateDocument(size);
    
    // Create lighting gradient
    cv::Mat lighting_mask(base_image.size(), CV_32F);
    for (int y = 0; y < lighting_mask.rows; ++y) {
        for (int x = 0; x < lighting_mask.cols; ++x) {
            float distance_factor = std::sqrt(std::pow(x - lighting_mask.cols/2, 2) + 
                                            std::pow(y - lighting_mask.rows/2, 2)) / 
                                  std::sqrt(std::pow(lighting_mask.cols/2, 2) + 
                                          std::pow(lighting_mask.rows/2, 2));
            lighting_mask.at<float>(y, x) = 1.0f - (lighting_variation * distance_factor);
        }
    }
    
    // Apply lighting variation
    cv::Mat result;
    base_image.convertTo(result, CV_32FC3);
    
    std::vector<cv::Mat> channels;
    cv::split(result, channels);
    for (auto& channel : channels) {
        cv::multiply(channel, lighting_mask, channel);
    }
    cv::merge(channels, result);
    
    result.convertTo(result, CV_8UC3);
    return result;
}

cv::Mat SyntheticTestGenerator::generateWithBackground(const DocumentSize& size, const std::string& background_type) {
    cv::Mat background = createBackground(cv::Size(size.width + 200, size.height + 200), background_type);
    cv::Mat document = generateDocument(size, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));
    
    // Blend document onto background
    cv::Mat result = background.clone();
    cv::Point doc_pos((result.cols - size.width) / 2, (result.rows - size.height) / 2);
    cv::Rect doc_rect(doc_pos, cv::Size(size.width, size.height));
    
    cv::Mat doc_region = document(cv::Rect(doc_pos, cv::Size(size.width, size.height)));
    doc_region.copyTo(result(doc_rect));
    
    return result;
}

cv::Mat SyntheticTestGenerator::generateWithBlur(const DocumentSize& size, float blur_amount) {
    cv::Mat base_image = generateDocument(size);
    cv::Mat blurred_image;
    
    int kernel_size = static_cast<int>(blur_amount * 2) * 2 + 1; // Ensure odd number
    cv::GaussianBlur(base_image, blurred_image, cv::Size(kernel_size, kernel_size), blur_amount);
    
    return blurred_image;
}

void SyntheticTestGenerator::addTextBlocks(cv::Mat& image, const cv::Rect& doc_area) {
    // Add synthetic text blocks (rectangles representing text areas)
    std::uniform_int_distribution<> x_dist(doc_area.x + 20, doc_area.x + doc_area.width - 100);
    std::uniform_int_distribution<> y_dist(doc_area.y + 20, doc_area.y + doc_area.height - 40);
    std::uniform_int_distribution<> width_dist(60, 120);
    std::uniform_int_distribution<> height_dist(8, 15);
    
    // Add several text block placeholders
    for (int i = 0; i < 5; ++i) {
        cv::Rect text_rect(x_dist(rng_), y_dist(rng_), width_dist(rng_), height_dist(rng_));
        cv::rectangle(image, text_rect, cv::Scalar(50, 50, 50), -1);
    }
}

void SyntheticTestGenerator::addNoise(cv::Mat& image, float intensity) {
    cv::Mat noise(image.size(), CV_8UC3);
    cv::randu(noise, cv::Scalar::all(0), cv::Scalar::all(255));
    
    cv::Mat result;
    cv::addWeighted(image, 1.0 - intensity, noise, intensity, 0, result);
    result.copyTo(image);
}

void SyntheticTestGenerator::addLogo(cv::Mat& image, const cv::Rect& doc_area) {
    // Add a simple geometric logo placeholder
    cv::Point logo_center(doc_area.x + doc_area.width - 60, doc_area.y + 40);
    cv::circle(image, logo_center, 25, cv::Scalar(100, 150, 200), -1);
    cv::circle(image, logo_center, 25, cv::Scalar(80, 130, 180), 2);
}

cv::Mat SyntheticTestGenerator::createBackground(const cv::Size& size, const std::string& type) {
    cv::Mat background(size, CV_8UC3);
    
    if (type == "plain") {
        background.setTo(cv::Scalar(200, 200, 200));
    } else if (type == "textured") {
        // Create a simple texture pattern
        for (int y = 0; y < size.height; ++y) {
            for (int x = 0; x < size.width; ++x) {
                int value = 180 + (x + y) % 40;
                background.at<cv::Vec3b>(y, x) = cv::Vec3b(value, value, value);
            }
        }
    } else if (type == "gradient") {
        for (int y = 0; y < size.height; ++y) {
            int value = 150 + (y * 100) / size.height;
            cv::line(background, cv::Point(0, y), cv::Point(size.width, y), cv::Scalar(value, value, value));
        }
    } else {
        background.setTo(cv::Scalar(220, 220, 220)); // Default
    }
    
    return background;
}

void SyntheticTestGenerator::generateTestSuite(const std::string& output_dir) {
    std::cout << "Generating synthetic test suite in: " << output_dir << std::endl;
    
    // Create output directory
    std::string mkdir_cmd = "mkdir -p " + output_dir;
    system(mkdir_cmd.c_str());
    
    int image_count = 0;
    
    // Test different document types
    std::vector<DocumentSize> doc_types = {ID_CARD, DRIVERS_LICENSE, PASSPORT_PAGE};
    
    for (const auto& doc_type : doc_types) {
        std::string type_prefix = doc_type.name;
        
        // Basic document
        cv::Mat basic = generateDocument(doc_type);
        cv::imwrite(output_dir + "/" + type_prefix + "_basic.jpg", basic);
        image_count++;
        
        // Rotated versions
        for (double angle : {-15.0, -5.0, 5.0, 15.0, 30.0}) {
            cv::Mat rotated = generateWithRotation(doc_type, angle);
            cv::imwrite(output_dir + "/" + type_prefix + "_rotated_" + 
                       std::to_string(static_cast<int>(angle)) + ".jpg", rotated);
            image_count++;
        }
        
        // Perspective versions
        for (float perspective : {0.05f, 0.1f, 0.2f}) {
            cv::Mat perspective_img = generateWithPerspective(doc_type, perspective);
            cv::imwrite(output_dir + "/" + type_prefix + "_perspective_" + 
                       std::to_string(static_cast<int>(perspective * 100)) + ".jpg", perspective_img);
            image_count++;
        }
        
        // Lighting variations
        for (float lighting : {0.1f, 0.3f, 0.5f}) {
            cv::Mat lighting_img = generateWithLighting(doc_type, lighting);
            cv::imwrite(output_dir + "/" + type_prefix + "_lighting_" + 
                       std::to_string(static_cast<int>(lighting * 100)) + ".jpg", lighting_img);
            image_count++;
        }
        
        // Background variations
        for (const std::string& bg : {"plain", "textured", "gradient"}) {
            cv::Mat bg_img = generateWithBackground(doc_type, bg);
            cv::imwrite(output_dir + "/" + type_prefix + "_bg_" + bg + ".jpg", bg_img);
            image_count++;
        }
        
        // Blur variations
        for (float blur : {1.0f, 2.0f, 3.0f}) {
            cv::Mat blur_img = generateWithBlur(doc_type, blur);
            cv::imwrite(output_dir + "/" + type_prefix + "_blur_" + 
                       std::to_string(static_cast<int>(blur)) + ".jpg", blur_img);
            image_count++;
        }
    }
    
    std::cout << "Generated " << image_count << " test images" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string output_dir = "test_temp";
    
    if (argc > 1) {
        output_dir = argv[1];
    }
    
    std::cout << "Synthetic Test Image Generator" << std::endl;
    std::cout << "Generating test images for ID document detection..." << std::endl;
    
    SyntheticTestGenerator generator;
    generator.generateTestSuite(output_dir);
    
    std::cout << "Test suite generation complete!" << std::endl;
    std::cout << "Images saved to: " << output_dir << std::endl;
    
    return 0;
}