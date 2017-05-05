//
// Created by stramek on 26.02.17.
//

#include "include/models/ImageLoader.h"

/**
 * RGB photos should by places inside dataset_photos/rgb
 *
 * DEPTH photos inside dataset_photos/depth
 *
 * Names of photos should be 0.png, 1.png, 2.png etc.
 *
 * @param numberOfPhotos determines how much rgb and depth photos we want to load.
 *
 */
ImageLoader::ImageLoader(int numberOfPhotos) : numberOfPhotos(numberOfPhotos - 1) {
    currentPhoto = 0;
}

/**
 * @return
 * ImagePair object which contains loaded rgb and depth photo.
 */
ImagePair ImageLoader::getNextPair() {
    return loadNextImage();
}

ImagePair ImageLoader::loadNextImage() {
    if (currentPhoto > numberOfPhotos) currentPhoto = 0;

    String rgbImagePath = "../dataset_photos//rgb//" + to_string(currentPhoto) + ".png";
    String depthImagePath = "../dataset_photos//depth//" + to_string(currentPhoto) + ".png";

    ImagePair imagePair;
    imagePair.setRgb(imread(rgbImagePath));
    imagePair.setDepth(imread(depthImagePath, CV_LOAD_IMAGE_ANYDEPTH));
    ++currentPhoto;

    if (imagePair.getRgb().empty()) {
        throw runtime_error("Could not find rgb image: " + rgbImagePath);
    } else if (imagePair.getDepth().empty()) {
        throw runtime_error("Could not find depth image: " + depthImagePath);
    }

    return imagePair;
}