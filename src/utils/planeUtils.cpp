#include "include/utils/planeUtils.h"

namespace planeUtils {
    vector<pair<Plane, Plane>> getSimilarPlanes(const vector<Plane> &previousFrame, const vector<Plane> &currentFrame) {

        vector<pair<Plane, Plane>> toReturn;
        vector<PlaneSimilarity> planeSimilarityVec;

        for (unsigned int i = 0; i < previousFrame.size(); ++i) {
            for (unsigned int j = 0; j < currentFrame.size(); ++j) {
                planeSimilarityVec.push_back(PlaneSimilarity(previousFrame.at(i), currentFrame.at(j), i, j));
            }
        }

        sort(planeSimilarityVec.begin(), planeSimilarityVec.end());

        for (PlaneSimilarity &outerPlaneSimilarity : planeSimilarityVec) {
            if (!outerPlaneSimilarity.isAnyOfFramesTaken()) {
                if (outerPlaneSimilarity.isSimilarityValid()) {
                    if (outerPlaneSimilarity.isAngleBetweenPlanedValid()) {
                        toReturn.push_back(pair<Plane, Plane>(outerPlaneSimilarity.getLastFrame(),
                                                              outerPlaneSimilarity.getCurrentFrame()));

                        for (PlaneSimilarity &innerPlaneSimilarity : planeSimilarityVec) {
                            if (innerPlaneSimilarity.isOneOfIndexesEqual(outerPlaneSimilarity)) {
                                innerPlaneSimilarity.setFramesAsTaken();
                            }
                        }
                    }
                } else {
                    break;
                }
            }
        }


        return toReturn;
    };

    Mat getRGBFrameMat(libfreenect2::Registration *registration, libfreenect2::Frame *undistorted,
                       libfreenect2::Frame *registered) {
        Mat toReturn(Size(registered->width, registered->height), CV_8UC3);
        toReturn = 0;
        for (int row = 0; row < registered->height; ++row) {
            for (int col = 0; col < registered->width; ++col) {
                float x, y, z, color;
                registration->getPointXYZRGB(undistorted, registered, row, col, x, y, z, color);
                const uint8_t *p = reinterpret_cast<uint8_t *>(&color);
                if (!isnanf(z)) {
                    Vec3b color1(p[0], p[1], p[2]);
                    toReturn.at<Vec3b>(Point(col, row)) = color1;
                }
            }
        }
        return toReturn;
    }

    void visualizeSimilarPlanes(vector<pair<Plane, Plane>> &similarPlanes, const Mat &previousImage,
                                const Mat &currentImage, int limitPoints) {
        Size previousImageSize = previousImage.size();
        Size currentImageSize = currentImage.size();
        Mat merged(previousImageSize.height, previousImageSize.width + currentImageSize.width, CV_8UC3);
        Mat left(merged, Rect(0, 0, previousImageSize.width, previousImageSize.height));
        previousImage.copyTo(left);
        Mat right(merged, Rect(previousImageSize.width, 0, currentImageSize.width, currentImageSize.height));
        currentImage.copyTo(right);

        RNG rng(12345);
        int pointNumber = 0;
        for (pair<Plane, Plane> pair : similarPlanes) {

            ImageCoords previousImageCoords = pair.first.getImageCoords();
            ImageCoords currentImageCoords = pair.second.getImageCoords();

            Point previousPlanePoint = Point(previousImageCoords.getCenterX(), previousImageCoords.getCenterY());
            Point currentPlanePoint = Point(previousImageSize.width + currentImageCoords.getCenterX(),
                                            currentImageCoords.getCenterY());

            int size = previousImageCoords.getAreaSize() / 2;
            Scalar color = Scalar(rng.uniform(150, 255), rng.uniform(150, 255), rng.uniform(150, 255));

            circle(merged, previousPlanePoint, size, color, 2);
            circle(merged, currentPlanePoint, size, color, 2);
            line(merged, previousPlanePoint, currentPlanePoint, color, 2);

            double angle = pair.first.getAngleBetweenTwoPlanes(pair.second);
            int colorDiff = abs(pair.first.getColor().getHue() - pair.second.getColor().getHue());
            stringstream stream;
            stream << fixed << setprecision(1) << angle;
            Point centerPoint = ((previousPlanePoint + currentPlanePoint) / 2);
            centerPoint.x = centerPoint.x - 50;
            putText(merged, "Angle: " + stream.str() + " Color diff: " + to_string(colorDiff), centerPoint,
                    FONT_HERSHEY_SIMPLEX, 0.5, color, 2);

            pointNumber++;
            if (pointNumber >= limitPoints) break;
        }

        imshow("Merged", merged);

        //imwrite("../images/similar" + to_string(SCREENSHOT_HELPER++) +".png", merged);

        waitKey(1);
    }

    void mergePlanes(vector<Plane> &planeVector) {
        if (planeVector.size() == 0) return;
        vector<vector<Plane>> clusteredPLanes;
        Clustering clustering;
        clustering.setCutSimilarity(CLUSTERING_MAX_DISTANCE_THRESHOLD);
        clustering.selectParts(planeVector, clusteredPLanes);
        planeVector = Clustering::getAveragedPlanes(clusteredPLanes);
    }

    void displayClusteredPlanes(ImagePair &imagePair, vector<Plane> planes) {
        if (planes.size() == 0) return;
        vector<vector<Plane>> clusteredPLanes;
        Clustering clustering;
        clustering.setCutSimilarity(CLUSTERING_MAX_DISTANCE_THRESHOLD);
        clustering.selectParts(planes, clusteredPLanes);
        //Clustering::getClusteredPlaneGroup(plane, clusteredPLanes);
        int i = 0;
        for (auto singleCluster : clusteredPLanes) {
            ++i;
            for (auto planeInCluster : singleCluster) {
                putText(imagePair.getRgb(), to_string(i), Point(planeInCluster.getImageCoords().getCenterX(),
                                                                planeInCluster.getImageCoords().getCenterY()),
                        FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);
            }
        }
        //imshow("Clustered planes", imagePair.getRgb());
        //waitKey();
    }
}