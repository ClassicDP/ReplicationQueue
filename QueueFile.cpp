

#include "QueueFile.h"
#include "MainCluster.cpp"
#include "StackList.cpp"
#include "DynamicArray.h"

int QueueFile::fileDescriptor;


QueueFile::QueueFile(char *fileName, uint32_t fileClusterSize) {
    this->filName = fileName;
    clusterSize = fileClusterSize;
    remove(fileName);
    fileDescriptor = open(fileName, O_RDWR | O_CREAT, 0777);
    mainCluster = new MainCluster(fileClusterSize);
    mainCluster->header->clusterSize = fileClusterSize;
    mainCluster->header->fileSize = fileClusterSize;
    mainCluster->header->firstFreePtr = fileClusterSize;
    mainCluster->header->DataPtr = fileClusterSize;
    mainCluster->header->backupPtr = 0;
    mainCluster->write();
    fsync(fileDescriptor);
    QueueFile::queueFile = this;
}

QueueFile::~QueueFile() {
    close(fileDescriptor);
    delete mainCluster;
}

void QueueFile::putMsg(std::string &msg) {
    auto cnt = clustersPerMessage(msg);
    DynamicArray<Cluster *> chain(cnt);
//    Cluster *chain[cnt];
    uint32_t msgOffset = 0;
    chain[0] = new Cluster(ClusterType::firstCluster);
    msgOffset += chain[0]->setData(msg, msgOffset);
    for (int i = 1; i < cnt; i++) {
        chain[i] = new Cluster(ClusterType::nextCluster);
        msgOffset += chain[i]->setData(msg, msgOffset);
    }

}

std::string QueueFile::takeMsg() {
    auto DataPtr = mainCluster->header->DataPtr;
    if (DataPtr == mainCluster->header->fileSize) return "";
    auto cluster = new Cluster(DataPtr);
    auto lenMsg = cluster->header->firstCluster.length;
    std::string msg;
    msg.reserve(lenMsg);
    auto cnt = clustersPerMessage(msg);
    Cluster *chain[cnt];
    chain[0] = cluster;
    auto msgOffset = cluster->setData(msg, 0);
    for (int i = 1; i < cnt; i++) {
        chain[i] = new Cluster(*chain[i - 1]->nextClusterPtr());
        msgOffset += chain[i]->getData(msg, msgOffset);
    }
    auto trancateFrom = mainCluster->header->fileSize - mainCluster->header->clusterSize;
    auto endOfInnerChain = cnt;
    while (--endOfInnerChain >= 0 && chain[endOfInnerChain]->clusterPtr == trancateFrom)
        trancateFrom -= mainCluster->header->clusterSize;
    trancateFrom += mainCluster->header->clusterSize;
    if (endOfInnerChain < cnt - 1) {
        *chain[endOfInnerChain]->nextClusterPtr() = trancateFrom;
//        chain[endOfInnerChain]->safeWrite();
    }
    mainCluster->header->firstFreePtr = chain[0]->clusterPtr;
    mainCluster->header->DataPtr = *chain[cnt - 1]->nextClusterPtr();
//    mainCluster->safeWrite();
    safeTruncate(trancateFrom);
    for (int i = 0; i < cnt; i++) delete chain[i];
    return msg;
}

uint16_t QueueFile::clustersPerMessage(std::string &msg) const {
    uint16_t size = mainCluster->header->clusterSize;
    uint16_t firstHLen = sizeof(FirstClusterHeader);
    uint16_t nextHLen = sizeof(NextClusterHeader);
    uint16_t msgLen = msg.size();
    uint16_t a = (msgLen - (size - firstHLen));
    uint16_t b = size - nextHLen;
    return 1 + a / b + (a % b ? 1 : 0);
}

void QueueFile::takeMsg(uint32_t ptr) {

}

void QueueFile::safeWrite(DynamicArray<Cluster *> &chain) {
    newClustersPtr = mainCluster->header->fileSize;
    freeClustersPtr = mainCluster->header->firstFreePtr;
    int lastInside;
    auto backupStack = new StackList<Cluster *>;
    for (int i = 0; i < chain.size; i++) {
        if (freeClustersPtr < mainCluster->header->fileSize) {
            auto cluster = new Cluster(freeClustersPtr);
            chain[i]->clusterPtr = freeClustersPtr;
            freeClustersPtr = *cluster->nextClusterPtr();
            *chain[i]->nextClusterPtr() = freeClustersPtr;
            lastInside = i;
            backupStack->push(cluster);
        } else {
            chain[i]->clusterPtr = newClustersPtr;
            newClustersPtr += clusterSize;
            *chain[i]->nextClusterPtr() = newClustersPtr;
        }
    }
    for (auto i = lastInside + 1; i < chain.size; i++) {
//        chain[i].
    }
}

void QueueFile::safeWriteComplete() {
    auto fileSize = mainCluster->header->fileSize;
    while (auto memBuf = stackList.pop()) {
        auto diskBufHeader = (SaveCluster *) memBuf;
        pwrite64(fileDescriptor, memBuf + sizeof(SaveCluster),
                 diskBufHeader->dataSize, diskBufHeader->dataPtr);
        auto fSize = diskBufHeader->dataSize + diskBufHeader->dataPtr;
        fileSize = fileSize > fSize ? fileSize : fSize;
        // was created in .safeWrite()
        delete[] memBuf;
    }
    fsync(fileDescriptor);

    mainCluster->header->fileSize = fileSize;
    mainCluster->header->backupPtr = 0;
    mainCluster->write();
    fsync(fileDescriptor);

    // safeWriteComplete disk operations!!

}

void QueueFile::safeTruncate(u_int32_t fileSize) {
    mainCluster->header->fileSize = fileSize;
//    mainCluster->safeWrite(0);
    safeWriteComplete();
    truncate(filName, fileSize);
}

uint32_t SaveCluster::write(uint32_t ptr) {
    pwrite64(QueueFile::fileDescriptor, buf, header->saveClasterSize, ptr);
    return sizeof(SaveCluster) + ptr;
}


SaveCluster::SaveCluster(uint32_t ptr, uint32_t size) {
    buf = new char[sizeof(SaveClusterHeader) + size];
    header = reinterpret_cast<SaveClusterHeader *>(buf);
    header->saveDataSize = size;
    header->dataPtr = ptr;
    header->saveClasterSize = sizeof(SaveClusterHeader) + size;
    pread64(QueueFile::fileDescriptor, buf + sizeof(SaveClusterHeader), size, ptr);
}

SaveCluster::~SaveCluster() {
    delete[] buf;
}

SaveCluster::SaveCluster(uint32_t ptr) {
    auto size = new uint32_t;
    pread64(QueueFile::fileDescriptor, size, sizeof(uint32_t), ptr);
    buf = new char[*size];
    pread64(QueueFile::fileDescriptor, buf, *size, ptr);
    delete size;
}

void SaveCluster::restore() {
    pwrite64(QueueFile::fileDescriptor, buf + sizeof(SaveClusterHeader), header->saveDataSize, header->dataPtr);
}
