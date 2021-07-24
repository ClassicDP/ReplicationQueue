
#ifndef UNTITLED_CLUSTER_H
#define UNTITLED_CLUSTER_H


#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include "types.h"


class Cluster {
public:
    uint32_t readFromPtr;
    Header *header;
    char *buffer;

    Cluster(u_int32_t ptr);

    uint32_t setData(std::__cxx11::basic_string<char> &msg, uint32_t offset);
    uint32_t getData(std::string &msg, uint32_t offset);


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
    void write();

    void safeWrite (uint32_t ptr);
    void safeWrite ();

};


#endif //UNTITLED_CLUSTER_H
