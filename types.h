

#ifndef UNTITLED_TYPES_H
#define UNTITLED_TYPES_H
enum ClusterType : unsigned char {
    mainCluster,
    firstCluster,
    nextCluster
};

struct FirstClusterHeader {
    ClusterType clusterType;
    u_int32_t length;
    u_int32_t nextPtr;
};
struct NextClusterHeader {
    ClusterType clusterType;
    u_int32_t nextPtr;
};
struct MainClusterHeader {
    ClusterType clusterType;
    u_int32_t fileSize;
    u_int32_t firstDataPtr;
    u_int32_t toReleasePtr;
    u_int32_t toReleaseClustersCnt;
    u_int32_t firstFreePtr;
    u_int32_t backUpPtr;
    u_int32_t queueLen;
    u_int32_t clusterSize;
};
union Header {
    FirstClusterHeader firstCluster;
    NextClusterHeader nextCluster;
    MainClusterHeader mainCluster;
};


#endif //UNTITLED_TYPES_H
