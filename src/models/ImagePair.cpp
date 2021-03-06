//
// Created by stramek on 26.02.17.
//

#include "include/models/ImagePair.h"

ImagePair::ImagePair() {}

ImagePair::ImagePair(const Mat &rgb, const Mat &depth) : rgb(rgb), depth(depth) {}


/**
 * @return RGB Mat image
 */
const Mat &ImagePair::getRgb() const {
    return rgb;
}

/**
 * @return Depth Mat image
 */
const Mat &ImagePair::getDepth() const {
    return depth;
}

void ImagePair::setRgb(const Mat &rgb) {
    ImagePair::rgb = rgb;
}

void ImagePair::setDepth(const Mat &depth) {
    ImagePair::depth = depth;
}

int ImagePair::getDepthAt(int x, int y) {
    return depth.at<uint16_t>(x, y);
}

bool ImagePair::isEmpty()const {
    return rgb.empty() || depth.empty();
}
