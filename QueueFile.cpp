

#include "QueueFile.h"
#include "MainCluster.cpp"
#include "StackList.cpp"

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

void QueueFile::putMsg(char *msg) {
    auto cnt = clustersPerMessage(msg);
    Cluster *chain[cnt];
    chain[0] = new Cluster(ClusterType::firstCluster);
    for (int i = 1; i < cnt; i++) {
        chain[i] = new Cluster(ClusterType::nextCluster);
    }
    uint32_t nextPtr = mainCluster->header->firstFreePtr;
    uint32_t msgOffset = 0;
    for (int i = 0; i < cnt; i++) {
        if (nextPtr != mainCluster->header->fileSize) {
            chain[i]->read(nextPtr);
            nextPtr = *chain[i]->nextClusterPtr();
        } else {
            nextPtr += mainCluster->header->clusterSize;
        }
        msgOffset += chain[i]->setData(msg, msgOffset);
        *chain[i]->nextClusterPtr() = nextPtr;
        chain[i]->safeWrite();
    }
    mainCluster->safeWrite();
    safeWriteComplete();
    for (int i = 0; i < cnt; i++) {
        delete chain[i];
    }
}

char *QueueFile::takeMsg() {
    auto DataPtr = mainCluster->header->DataPtr;
    if (DataPtr == mainCluster->header->fileSize) return nullptr;
    auto cluster = new Cluster(DataPtr);
    auto lenMsg = cluster->header->firstCluster.length;
    auto msg = new char[lenMsg];
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
    while (--endOfInnerChain >= 0 && chain[endOfInnerChain]->readFromPtr == trancateFrom)
        trancateFrom -= mainCluster->header->clusterSize;
    trancateFrom += mainCluster->header->clusterSize;
    if (endOfInnerChain < cnt - 1) {
        *chain[endOfInnerChain]->nextClusterPtr() = trancateFrom;
        chain[endOfInnerChain]->safeWrite();
    }
    mainCluster->header->firstFreePtr = chain[0]->readFromPtr;
    mainCluster->header->DataPtr = *chain[cnt - 1]->nextClusterPtr();
    mainCluster->safeWrite();
    safeTruncate(trancateFrom);
    for (int i = 0; i< cnt ; i++) delete chain[i];
    return msg;
}

uint16_t QueueFile::clustersPerMessage(const char *msg) const {
    uint16_t size = mainCluster->header->clusterSize;
    uint16_t firstHLen = sizeof(FirstClusterHeader);
    uint16_t nextHLen = sizeof(NextClusterHeader);
    uint16_t msgLen = sizeof(msg);
    uint16_t a = (msgLen - (size - firstHLen));
    uint16_t b = size - nextHLen;
    return 1 + a / b + a % b ? 1 : 0;
}

void QueueFile::takeMsg(uint32_t ptr) {

}

void QueueFile::safeWrite(uint32_t ptr, u_int32_t size, char *buf) {
    u_int32_t fileSize = mainCluster->header->fileSize;
    auto restFileSize = fileSize - ptr;
    // if write field of file need to backup
    if (restFileSize > 0) {
        // will deleting in .safeWriteComplete()
        size = (size < restFileSize) ? size : restFileSize;
        auto diskBuf = new u_int8_t[size + sizeof(SafeBufHeader)];
        if (mainCluster->header->backupPtr == 0) {
            mainCluster->header->firstFreePtr = fileSize;
            mainCluster->header->backupPtr = fileSize;
            mainCluster->write();
            fsync(fileDescriptor);
        }
        mainCluster->header->fileSize += sizeof(diskBuf);
        // using diskBuf for backup data
        pread64(fileDescriptor, diskBuf + sizeof(SafeBufHeader), size, ptr);
        auto diskBufHeader = (SafeBufHeader *) diskBuf;
        diskBufHeader->dataSize = size;
        diskBufHeader->dataPtr = ptr;
        diskBufHeader->size = sizeof(diskBuf);
        pwrite64(fileDescriptor, diskBuf, sizeof(diskBuf), fileSize);
        delete[] diskBuf;
    }
    // using same memBuf for stacking data to future write
    if (buf) {
        auto memBuf = new u_int8_t[size + sizeof(SafeBufHeader)];
        memcpy(memBuf + sizeof(SafeBufHeader), buf, size);
        stackList.push(memBuf);
    }
    mainCluster->write();
    fsync(fileDescriptor);
}

void QueueFile::safeWriteComplete() {
    while (auto memBuf = stackList.pop()) {
        auto diskBufHeader = (SafeBufHeader *) memBuf;
        pwrite64(fileDescriptor, memBuf + sizeof(SafeBufHeader),
                 diskBufHeader->dataSize, diskBufHeader->dataPtr);
        // was created in .safeWrite()
        delete[] memBuf;
    }
    fsync(fileDescriptor);

    mainCluster->header->fileSize = mainCluster->header->backupPtr;
    mainCluster->header->backupPtr = 0;
    mainCluster->write();
    fsync(fileDescriptor);

    // safeWriteComplete disk operations!!

}

void QueueFile::safeTruncate(u_int32_t fileSize) {
    mainCluster->header->fileSize = fileSize;
    mainCluster->safeWrite(0);
    safeWriteComplete();
    truncate(filName, fileSize);
}
