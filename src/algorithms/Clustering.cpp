//
// Created by mordimer on 11.03.17.
//

#include "include/algorithms/Clustering.h"

double Clustering::getSimilarityOfTwoPlanes(const Plane &firstPlane, const Plane &secondPlane) {
    double angleBetweenTwoPlanes = firstPlane.getAngleBetweenTwoPlanes(secondPlane);
    if (abs(angleBetweenTwoPlanes) < CLUSTERING_MAX_ANGLE_THRESHOLD) {
        return planeUtils::getDistanceBetweenTwoPlanes(firstPlane, secondPlane);
    }
    return std::numeric_limits<double>::max();
}

vector<Plane> Clustering::getAveragedPlanes(vector<vector<Plane>> &clusteredPlanes) {
    vector<Plane> averagedPlanesVec;
    for (auto &planesFromCluster : clusteredPlanes) {
        vector<Point3D> points_new;

        double averagedD = 0;
        int averagedHue = 0;
        int averagedSaturation = 0;
        int averagedValue = 0;
        vector<Point3D> mergedPlanePoints;
        vector<ImageCoords> mergedPlaneImageCoordsVec;
        Vector3d averagedNormalVec = Vector3d::Zero();
        for (auto &plane : planesFromCluster) {
            averagedNormalVec += plane.getPlaneNormalVec();
            averagedD += plane.getD();
            averagedHue += plane.getColor().getHue();
            averagedSaturation += plane.getColor().getSaturation();
            averagedValue += plane.getColor().getValue();

            points_new.insert(points_new.end(),plane.getPoints().begin(), plane.getPoints().end());
            //cout<<"Points size: "<<points_new.size()<<endl;

            vector<Point3D> points = plane.getPoints();
            vector<ImageCoords> imageCoordsVec = plane.getImageCoordsVec();
            mergedPlanePoints.insert(mergedPlanePoints.end(), points.begin(), points.end());
            mergedPlaneImageCoordsVec.insert(mergedPlaneImageCoordsVec.end(), imageCoordsVec.begin(),
                                             imageCoordsVec.end());
        }
        averagedNormalVec = averagedNormalVec / planesFromCluster.size();
        averagedD = averagedD / planesFromCluster.size();
        averagedHue = averagedHue / (int) planesFromCluster.size();
        averagedSaturation = averagedSaturation / (int) planesFromCluster.size();
        averagedValue = averagedValue / (int) planesFromCluster.size();
//        Plane averagedPlane(averagedNormalVec, averagedD, mergedPlanePoints, mergedPlaneImageCoordsVec,
//                            HSVColor((uint8_t) averagedHue, (uint8_t) averagedSaturation, (uint8_t) averagedValue));
        //averagedPlanesVec.push_back(averagedPlane);

        PcaPlaneDetector pcaPlaneDetector;
        Plane averagedPlane = pcaPlaneDetector.getPlane(points_new, mergedPlaneImageCoordsVec.at(0), nullptr, true);
        averagedPlanesVec.push_back(averagedPlane);
    }
    return averagedPlanesVec;
}


////////////////////////////

void Clustering::selectParts(const std::vector<Plane> &planesVec, std::vector<std::vector<Plane>> &clusteredPlanes) {
    std::vector<std::vector<double>> distanceMatrix(planesVec.size(), std::vector<double>(planesVec.size()));
    //computeDistanceMatrix(dictionary, hierarchy, distanceMatrix, transformMatrix);
    computeDistanceMatrix(planesVec, distanceMatrix);

    std::vector<std::vector<int>> clusters(planesVec.size());
    for (size_t i = 0; i < clusters.size(); i++) {
        clusters[i].push_back((int) i);
    }

    for (size_t i = 0; i < (planesVec.size() - 1) * planesVec.size() / 2; i++) {

        std::pair<int, int> pairedIds;
        double minDist = findMinDistance(pairedIds);

        if (minDist >= cutSimilarity)
            break;

        //merge two centroids
        std::pair<int, int> clustersIds;
        findPartsInClusters(clusters, pairedIds, clustersIds);

        if (clustersIds.first != clustersIds.second) {
            mergeTwoClusters(clusters, clustersIds, distanceMatrix);
        }
    }

    getClusteredPlaneGroup(clusters, planesVec, clusteredPlanes);
}

void Clustering::computeDistanceMatrix(const std::vector<Plane> &planesVec,
                                       std::vector<std::vector<double>> &distanceMatrix) {
    while (!priorityQueueDistance.empty()) priorityQueueDistance.pop();
    size_t startId(0), endId(planesVec.size());
    for (size_t idA = startId; idA < endId; idA++) {
        for (size_t idB = idA; idB < endId; idB++) {
            double dist(0);
            if (idA == idB) {
                distanceMatrix[idB][idA] = 0;
            } else {

                dist = getSimilarityOfTwoPlanes(planesVec.at(idA), planesVec.at(idB));
                distanceMatrix[idA][idB] = dist;
                distanceMatrix[idB][idA] = dist;
                Cluster cluster;
                cluster.setDistanceBetweenLinks(dist);
                cluster.setLinkedIndexes(std::make_pair(idA, idB));
                priorityQueueDistance.push(cluster);
            }
        }
    }
}

/// find min distance int the distance matrix
double Clustering::findMinDistance(std::pair<int, int> &pairedIds) {
    Cluster cluster = priorityQueueDistance.top();
    double minDist = cluster.getDistanceBetweenLinks();
    pairedIds = cluster.getLinkedIndexes();
    priorityQueueDistance.pop();

    return minDist;
}

void Clustering::setCutSimilarity(double cutSimilarity) {
    Clustering::cutSimilarity = cutSimilarity;
}

/// find clusters ids to which contain specyfic parts (pairedIds)
void
Clustering::findPartsInClusters(const std::vector<std::vector<int>> &clusters, const std::pair<int, int> &pairedIds,
                                std::pair<int, int> &clustersIds) const {
    bool found[2] = {false, false};
    for (size_t i = 0; i < clusters.size(); i++) {
        for (auto &id : clusters[i]) {
            if (id == pairedIds.first) {
                clustersIds.first = (int) i;
                found[0] = true;
            }
            if (id == pairedIds.second) {
                clustersIds.second = (int) i;
                found[1] = true;
            }
            if (found[0] && found[1]) break;
        }
        if (found[0] && found[1]) break;
    }
}

/// merge two clusters
bool Clustering::mergeTwoClusters(std::vector<std::vector<int>> &clusters, const std::pair<int, int> &clustersIds,
                                  const std::vector<std::vector<double>> &distanceMatrix) const {
    double maxDist = computeMaxDist(clusters, clustersIds, distanceMatrix);
    if (maxDist < cutSimilarity) {
        if (clustersIds.first < clustersIds.second) {
            // merge clusters
            clusters[clustersIds.first].insert(clusters[clustersIds.first].end(), clusters[clustersIds.second].begin(),
                                               clusters[clustersIds.second].end());
            // remove second cluster
            clusters.erase(clusters.begin() + clustersIds.second);
        } else {
            // merge clusters
            clusters[clustersIds.second].insert(clusters[clustersIds.second].end(), clusters[clustersIds.first].begin(),
                                                clusters[clustersIds.first].end());
            // remove second cluster
            clusters.erase(clusters.begin() + clustersIds.first);
        }
        return true;
    } else
        return false;
}

/// compute max distance between centroid of the first cluster and all parts in the second cluster
double Clustering::computeMaxDist(const std::vector<std::vector<int>> &clusters, const std::pair<int, int> &clustersIds,
                                  const std::vector<std::vector<double>> &distanceMatrix) const {
    double maxDist = std::numeric_limits<double>::min();

    for (const auto &partIdA : clusters[clustersIds.first]) {
        for (const auto &partIdB : clusters[clustersIds.second]) {
            double dist;
            if (partIdA > partIdB)//because up-triangle elements in distance matrix are cleaned
                dist = distanceMatrix[partIdA][partIdB];
            else
                dist = distanceMatrix[partIdB][partIdA];
            if (dist > maxDist) {
                maxDist = dist;
            }
        }
    }
    return maxDist;
}

void
Clustering::getClusteredPlaneGroup(const std::vector<std::vector<int>> clusters, const std::vector<Plane> &planesVec,
                                   vector<vector<Plane>> &clusteredPlanes) {
    for (auto planesIndexesInOneCluster : clusters) {
        vector<Plane> singleCluster;
        for (auto &index : planesIndexesInOneCluster) {
            Plane singleClusterPlane = planesVec.at(index);
            singleCluster.push_back(singleClusterPlane);
        }
        clusteredPlanes.push_back(singleCluster);
    }
}