
#include "Cluster.h"
#include "QueueFile.h"


uint32_t QueueFile::clusterSize;
QueueFile * QueueFile::queueFile;

u_int32_t Cluster::headerSize() {
    if (*_clusterType() == ClusterType::mainCluster)
        return sizeof(mainCluster);
    if (*_clusterType() == ClusterType::firstCluster)
        return sizeof(FirstClusterHeader);
    if (*_clusterType() == ClusterType::nextCluster)
        return sizeof(NextClusterHeader);
    return 0;
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
    buffer = new DynamicArray<char> (QueueFile::clusterSize);
    header = (Header *) buffer;
    *_clusterType() = clusterType;
}

u_int32_t Cluster::dataSize() {
    return QueueFile::clusterSize - headerSize();
}

void Cluster::write(uint32_t ptr) {
        pwrite64(QueueFile::fileDescriptor, buffer, QueueFile::clusterSize, ptr);
    clusterPtr = ptr;
}

void Cluster::read(uint32_t ptr) {
    pread64(QueueFile::fileDescriptor, buffer, QueueFile::clusterSize, ptr);
    clusterPtr = ptr;
}

uint32_t Cluster::setData(std::string &msg, uint32_t offset) {
    auto msgLen = msg.size();
    auto restSize = msgLen - offset;
    auto dataLen = dataSize();
    *checksum() = 0;
    u_int32_t _headerSize = headerSize();
    auto size = restSize < dataLen ? restSize : dataLen;
    for (uint32_t i = 0; i < size; i++) {
        auto dataByte = *(msg.c_str() + offset + i);
        *checksum() ^= dataByte;
        *(buffer->data + _headerSize + i) = dataByte;
    }
    return size;
}

uint32_t Cluster::getData(std::string &msg, uint32_t offset) {
    auto msgLen = msg.size();
    auto restSize = msgLen - offset;
    auto dataLen = dataSize();
    auto checkByte = 0;
    u_int32_t _headerSize = headerSize();
    auto size = restSize < dataLen ? restSize : dataLen;
    for (uint32_t i = 0; i < size; i++) {
        auto dataByte = *(buffer->data + _headerSize + i);
        checkByte ^= dataByte;
        msg[offset+i] = dataByte;
    }
    if (checkByte!=*checksum()) throw "File damaged!";
    return size;

}

Cluster::Cluster(u_int32_t ptr) {
    read(ptr);
}



void Cluster::write() {
    write(clusterPtr);
}



