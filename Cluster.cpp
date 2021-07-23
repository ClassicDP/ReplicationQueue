
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

ClusterType *Cluster::_clusterType() const {
    return (ClusterType *) buffer;
}

uint32_t *Cluster::nextClusterPtr() const {
    if (*_clusterType() == ClusterType::firstCluster) {
        return &(header->firstCluster.nextPtr);
    } else {
        return &(header->nextCluster.nextPtr);
    }
}

uint8_t *Cluster::checksum() {
    if (*_clusterType() == ClusterType::firstCluster) {
        return &(header->firstCluster.checksum);
    } else {
        return &(header->nextCluster.checksum);
    }
}

Cluster::Cluster(ClusterType clusterType) {
    buffer = new int8_t[QueueFile::clusterSize];
    header = (Header *) buffer;
    *_clusterType() = clusterType;
}

u_int32_t Cluster::dataSize() {
    return QueueFile::clusterSize - headerSize();
}

void Cluster::write(uint32_t ptr) {
    pwrite64(QueueFile::fileDescriptor, buffer, QueueFile::clusterSize, ptr);
}

void Cluster::read(uint32_t ptr) {
    pread64(QueueFile::fileDescriptor, buffer, QueueFile::clusterSize, ptr);
}

void Cluster::setData(char *msg, uint32_t offset) {
    auto msgLen = sizeof(msg);
    auto restSize = msgLen - offset;
    auto dataLen = dataSize();
    *checksum() = 0;
    u_int32_t _headerSize = headerSize();
    for (uint32_t i = 0; i<restSize < dataLen ? restSize : dataLen; i++) {
        auto dataByte = *(msg + _headerSize + i);
        *checksum()^=dataByte;
         *(msg + _headerSize + i) = dataByte;
    }
}

Cluster::Cluster(int64_t ptr) {
    read(ptr);
}


