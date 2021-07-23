

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
    mainCluster->header->firstDataPtr = fileClusterSize;
    mainCluster->header->backupPtr = 0;
    mainCluster->write();
    fsync(fileDescriptor);
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
    uint16_t nextPtr = mainCluster->header->firstFreePtr;
    uint32_t msgPtr = 0;
    for (int i = 0; i < cnt; i++) {
        uint16_t ptr = nextPtr;
        if (nextPtr != mainCluster->header->fileSize) {
            nextPtr = *Cluster(ptr).nextClusterPtr();
        } else {
            nextPtr += mainCluster->header->clusterSize;
        }
        chain[i]->setData(msg, msgPtr);
        *chain[i]->nextClusterPtr() = nextPtr;
        chain[i]->write(ptr);
        ptr += chain[i]->dataSize();
    }
    for (int i = 0; i < cnt; i++) {
        delete chain[i];
    }
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

void QueueFile::safeWrite(uint32_t ptr, u_int32_t size, u_int8_t *buf) {
    u_int32_t fileSize = mainCluster->header->fileSize;
    // will deleting in .safeWriteComplete()
    auto diskBuf = new u_int8_t[size + sizeof(DiskBufHeader)];
    if (mainCluster->header->backupPtr == 0) {
        mainCluster->header->firstFreePtr = fileSize;
        mainCluster->header->backupPtr = fileSize;
        mainCluster->write();
        fsync(fileDescriptor);
    }
    mainCluster->header->fileSize += sizeof(diskBuf);
    // using diskBuf for backing data
    pread64(fileDescriptor, diskBuf + sizeof(DiskBufHeader), size, ptr);
    auto diskBufHeader = (DiskBufHeader *) diskBuf;
    diskBufHeader->dataSize = size;
    diskBufHeader->dataPtr = ptr;
    diskBufHeader->size = sizeof(diskBuf);
    pwrite64(fileDescriptor, diskBuf, sizeof(diskBuf), fileSize);
    // using same diskBuf for stacking data to future write (after backup old data)
    if (buf) {
        // that is, it (diskBuf) is used for two purposes
        memcpy(diskBuf + sizeof(DiskBufHeader), buf, size);
        stackList.push(diskBuf);
    }
    mainCluster->write();
    fsync(fileDescriptor);
}

void QueueFile::safeWriteComplete() {
    while (auto buffer = stackList.pop()) {
        auto diskBufHeader = (DiskBufHeader *) buffer;
        pwrite64(fileDescriptor, buffer + sizeof(DiskBufHeader),
                 diskBufHeader->dataSize, diskBufHeader->dataPtr);
        // was created in .safeWrite()
        delete[] buffer;
    }

    mainCluster->header->fileSize = mainCluster->header->backupPtr;
    mainCluster->header->backupPtr = 0;
    mainCluster->write();
    fsync(fileDescriptor);

    // safeWriteComplete disk operations!!

}
