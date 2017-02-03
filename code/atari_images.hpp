#ifndef __ATARI_IMAGES_HPP__
#define __ATARI_IMAGES_HPP__

#include <ale_interface.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <string>
#include <ctime>
#include <unistd.h>

using namespace cv;
using namespace ale;

const uint32_t kImgHeight = 210;
const uint32_t kImgWidth = 160;
const uint32_t kCropSize = 84;
const uint32_t kImgSize = 84 * 84;

inline void convertNTSC(const vector<pixel_t> &raw, cv::Mat &rgb, int height, int width) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            ALEInterface::getRGB(raw[i * width + j],
                                 rgb.at<cv::Vec3b>(i, j)[2],
                                 rgb.at<cv::Vec3b>(i, j)[1],
                                 rgb.at<cv::Vec3b>(i, j)[0]);
        }
    }
}

void imgResize(cv::Mat &origin, cv::Mat &result, int height, int width) {
    cv::resize(origin, result, cv::Size(width, height));
}

void RGBToGray(cv::Mat &rgb, cv::Mat &gray) {
    cv::cvtColor(rgb, gray, CV_BGR2GRAY);
}

void GrayToArray(cv::Mat gray, unsigned char *array, int height, int width) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            array[i * width + j] = gray.at < unsigned char > (i, j);
        }
    }
}

class FrameBuffer {
public:
    unsigned char *frames_;
    unsigned char *buf_;

    const uint32_t channels_;

    cv::Mat *rgb_image;

    FrameBuffer(int channels) : channels_(channels) {
        frames_ = new unsigned char[channels_ * kCropSize * kCropSize];
        buf_ = new unsigned char[kCropSize * kCropSize];
        rgb_image = new cv::Mat(kImgHeight, kImgWidth, CV_8UC3);
        initialize();
    }

    ~FrameBuffer() {
        delete[] frames_;
        delete[] buf_;
    }

    const unsigned char *stackedFrames() const {
        return frames_;
    }

    const unsigned char *currentFrame() const {
        return frames_ + (channels_ - 1) * kImgSize;
    }

    void initialize() {
        memset(frames_, 0, sizeof(unsigned char) * channels_ * kCropSize * kCropSize);
        memset(buf_, 0, sizeof(unsigned char) * kCropSize * kCropSize);
    }

    void pushFrame(const ALEScreen &frame) {
        cv::Mat resized(kCropSize, kCropSize, CV_8UC3);
        cv::Mat gray(kCropSize, kCropSize, CV_8UC1);

        convertNTSC(frame.getArray(), *rgb_image, kImgHeight, kImgWidth);
        imgResize(*rgb_image, resized, kCropSize, kCropSize);
        RGBToGray(resized, gray);
        GrayToArray(gray, buf_, kCropSize, kCropSize);
        // Move from right to left and add current frame at the end
        if (channels_ > 1) {
            memmove(frames_, frames_ + kImgSize, (channels_ - 1) * kImgSize);
        }
        memcpy(frames_ + (channels_ - 1) * kImgSize, buf_, kImgSize);
    }

    void writeToFile(const char *file_name) {
        try {
            imwrite(file_name, *rgb_image);
        }
        catch (Exception& exception1){
            cout << "oups\n";
            throw exception1;
        }
    }
};

#endif