//
// Created by mordimer on 11.03.17.
//

#ifndef PROJEKTMAGISTERSKI_CLUSTERING_H
#define PROJEKTMAGISTERSKI_CLUSTERING_H

#include <istream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "include/models/Cluster.h"
#include <unordered_set>
#include "include/models/Plane.h"
#include <include/utils/constants.h>

class Clustering {
private:

    std::priority_queue<Cluster> priorityQueueDistance;
    double cutSimilarity = 0;

    const static double MAX_ANGLE_THRESHOLD;

    static double getDistanceBetweenTwoPoints(cv::Point_<double> point1, cv::Point_<double> point2);
    static double getDistanceBetweenTwoPlanes(const Plane &firstPlane, const Plane &secondPlane);
    static double getAngleBetweenTwoPlanes(const Plane &firstPlane, const Plane &secondPlane);
    static double getSimilarityOfTwoPlanes(const Plane &firstPlane, const Plane &secondPlane);

    static double getDistanceBetweenPointAndPlane(Plane plane, Vector3d point);


    double findMinDistance(std::pair<int,int>& pairedIds);
    void findPartsInClusters(const std::vector<std::vector<int>>& clusters, const std::pair<int,int>& pairedIds, std::pair<int,int>& clustersIds) const;
    void computeDistanceMatrix(const std::vector<Plane> &planesVec, std::vector<std::vector<double>>& distanceMatrix);
    bool mergeTwoClusters(std::vector<std::vector<int>>& clusters, const std::pair<int,int>& clustersIds, const std::vector<std::vector<double>>& distanceMatrix) const;
    double computeMaxDist(const std::vector<std::vector<int>>& clusters, const std::pair<int,int>& clustersIds, const std::vector<std::vector<double>>& distanceMatrix) const;
    void getClusteredPlaneGroup(const std::vector<std::vector<int>> clusters, const std::vector<Plane> &planesVec, vector<vector<Plane>> &clusteredPlanes);

public:
    void selectParts(const std::vector<Plane> &planesVec, std::vector<std::vector<Plane>> &clusteredPlanes);

    static vector<Plane> getAveragedPlanes(vector<vector<Plane>>& clusteredPlanes);

    void setCutSimilarity(double cutSimilarity);
};


#endif //PROJEKTMAGISTERSKI_CLUSTERING_H