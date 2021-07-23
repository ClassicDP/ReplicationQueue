
#ifndef UNTITLED_CLUSTER_H
#define UNTITLED_CLUSTER_H


#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "types.h"


class Cluster {
public:
    Header *header;
    int8_t *buffer;

    Cluster(int64_t ptr);

    void setData (char *msg, uint32_t offset);
    ClusterType *_clusterType() const;
    uint32_t *nextClusterPtr() const;
    uint8_t *checksum();
    u_int32_t headerSize();
    u_int32_t dataSize();
    Cluster(ClusterType clusterType);
    ~Cluster() {
        delete buffer;
    };


    void read(uint32_t ptr);

    void write(uint32_t ptr);
};


#endif //UNTITLED_CLUSTER_H
