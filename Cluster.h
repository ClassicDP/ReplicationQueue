
#ifndef UNTITLED_CLUSTER_H
#define UNTITLED_CLUSTER_H


#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "types.h"


class Cluster {
private:
    ClusterType clusterType;

public:
    Header *header;
    int8_t *buffer;
    uint32_t size;

    Cluster(int64_t ptr) {

    }

    void setData (char *msg, uint32_t offset);
    ClusterType *_clusterType();
    uint32_t *nextClusterPtr();
    u_int32_t headerSize();
    u_int32_t dataSize();
    Cluster(uint32_t size, ClusterType clusterType);
    ~Cluster() {
        delete buffer;
    };


    void read(uint32_t ptr);

    void write(uint32_t ptr);
};


#endif //UNTITLED_CLUSTER_H
