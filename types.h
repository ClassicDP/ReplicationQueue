

#ifndef UNTITLED_TYPES_H
#define UNTITLED_TYPES_H
enum ClusterType : unsigned char {
    mainCluster,
    firstCluster,
    nextCluster
};

struct FirstClusterHeader {
    ClusterType clusterType;
    u_int8_t checksum;
    u_int32_t length;
    u_int32_t nextPtr;
};
struct NextClusterHeader {
    ClusterType clusterType;
    u_int8_t checksum;
    u_int32_t nextPtr;
};
struct MainClusterHeader {
    ClusterType clusterType;
    u_int32_t fileSize;
    u_int32_t DataPtr;
    u_int32_t toReleasePtr;
    u_int32_t toReleaseClustersCnt;
    u_int32_t firstFreePtr;
    u_int32_t queueLen;
    u_int32_t clusterSize;
    u_int32_t backupPtr;
};
struct SafeBufHeader {
    // DiskBuf size
    u_int32_t size;
    // cluster size
    u_int32_t dataSize;
    // position of cluster in file
    u_int32_t dataPtr;
};
union Header {
    FirstClusterHeader firstCluster;
    NextClusterHeader nextCluster;
    MainClusterHeader mainCluster;
};


#endif //UNTITLED_TYPES_H
