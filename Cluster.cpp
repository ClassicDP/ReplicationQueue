
#include "Cluster.h"
#include "QueueFile.h"


u_int32_t Cluster::headerSize() {
    if (*_clusterType() == ClusterType::mainCluster)
        return sizeof(mainCluster);
    if (*_clusterType() == ClusterType::firstCluster)
        return sizeof(FirstClusterHeader);
    if (*_clusterType() == ClusterType::nextCluster)
        return sizeof(NextClusterHeader);
}

uint32_t *Cluster::nextClusterPtr() {
    if (clusterType == ClusterType::firstCluster) {
        return &(header->firstCluster.nextPtr);
    } else {
        return &(header->nextCluster.nextPtr);
    }
}

ClusterType *Cluster::_clusterType() {
    return (ClusterType *) buffer;
}

Cluster::Cluster(uint32_t size, ClusterType clusterType): size(size) {
    buffer = new int8_t[size];
    header = (Header *) buffer;
    *_clusterType() = clusterType;
}

u_int32_t Cluster::dataSize() {
    return size - headerSize();
}

void Cluster::write(uint32_t ptr)  {
    pwrite64(QueueFile::fileDescriptor, buffer, size, ptr);
}

void Cluster::read(uint32_t ptr) {
    pread64(QueueFile::fileDescriptor, buffer, size, ptr);
}

void Cluster::setData(char *msg, uint32_t offset) {
    auto msgLen = sizeof (msg);
    auto restSize = msgLen - offset;
    auto dataLen = dataSize();
    memcpy (buffer, msg + headerSize(), restSize < dataLen ? restSize : dataLen );
}
