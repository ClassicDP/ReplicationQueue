

#include "QueueFile.h"
#include "MainCluster.cpp"
#include "StackList.cpp"
#include "DynamicArray.h"

int QueueFile::fileDescriptor;


QueueFile::QueueFile(char const *fileName, uint32_t fileClusterSize) {
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

void QueueFile::putMsg(DynamicArray<char> &msg) {
    auto cnt = clustersPerMessage(msg);
    DynamicArray<Cluster *> chain(cnt);
    uint32_t msgOffset = 0;
    chain[0] = new Cluster(ClusterType::firstCluster);
    chain[0]->header->firstCluster.lenMsg = msg.size;
    msgOffset += chain[0]->setData(msg, msgOffset);
    for (int i = 1; i < cnt; i++) {
        chain[i] = new Cluster(ClusterType::nextCluster);
        msgOffset += chain[i]->setData(msg, msgOffset);
    }
    safeWrite(chain);
//    for (int  i = 0; i< cnt; i++) {
//        delete (chain[i]);
//    }
}

DynamicArray<char> QueueFile::takeMsg() {
    auto DataPtr = mainCluster->header->DataPtr;
    if (DataPtr == mainCluster->header->fileSize) return DynamicArray<char>((uint32_t)0);
    auto cluster = new Cluster(DataPtr);
    auto lenMsg = cluster->header->firstCluster.lenMsg;
    DynamicArray<char> msg(lenMsg);
    auto cnt = clustersPerMessage(msg);
    DynamicArray<Cluster *> chain(cnt);
    chain[0] = cluster;
    auto msgOffset = cluster->getData(msg, 0);
    for (int i = 1; i < cnt; i++) {
        chain[i] = new Cluster(*chain[i - 1]->nextClusterPtr());
        msgOffset += chain[i]->getData(msg, msgOffset);
    }
    auto trancateFrom = mainCluster->header->fileSize - mainCluster->header->clusterSize;
    auto lastInside = cnt;
    while (--lastInside >= 0 && chain[lastInside]->clusterPtr == trancateFrom)
        trancateFrom -= mainCluster->header->clusterSize;
    trancateFrom += mainCluster->header->clusterSize;
    if (lastInside < cnt - 1) {
        *chain[lastInside]->nextClusterPtr() = trancateFrom;
    }
    mainCluster->header->firstFreePtr = chain[0]->clusterPtr;
    mainCluster->header->DataPtr = *chain[cnt - 1]->nextClusterPtr();
    mainCluster->write();
    fsync(QueueFile::fileDescriptor);
    truncate(filName, trancateFrom);
    for (int i = 0; i < cnt; i++) delete chain[i];
    return msg;
}

uint32_t QueueFile::clustersPerMessage(DynamicArray<char> &msg) const {
    uint32_t size = mainCluster->header->clusterSize;
    uint32_t firstHLen = sizeof(FirstClusterHeader);
    uint32_t nextHLen = sizeof(NextClusterHeader);
    uint32_t msgLen = msg.size;
    uint32_t a = (msgLen - (size - firstHLen));
    uint32_t b = size - nextHLen;
    return 1 + a / b + (a % b ? 1 : 0);
}

void QueueFile::takeMsg(uint32_t ptr) {

}

void QueueFile::safeWrite(DynamicArray<Cluster *> &chain) {
    newClustersPtr = mainCluster->header->fileSize;
    freeClustersPtr = mainCluster->header->firstFreePtr;
    auto lastInside = -1;
    for (int i = 0; i < chain.size; i++) {
        if (freeClustersPtr < mainCluster->header->fileSize) {
            auto cluster = new Cluster(freeClustersPtr);
            chain[i]->clusterPtr = freeClustersPtr;
            freeClustersPtr = *cluster->nextClusterPtr();
            *chain[i]->nextClusterPtr() = freeClustersPtr;
            lastInside = i;
            delete cluster;
        } else {
            chain[i]->clusterPtr = newClustersPtr;
            newClustersPtr += clusterSize;
            *chain[i]->nextClusterPtr() = newClustersPtr;
        }
    }
    uint32_t saveClusterPtr;
    for (auto i = lastInside + 1; i < chain.size; i++) {
        saveClusterPtr = chain[i]->write();
    }
    mainCluster->header->backupPtr = saveClusterPtr;
    for (auto i = 0; i <= lastInside; i++) {
        auto saveCluster = new SaveCluster(chain[i]->clusterPtr, clusterSize);
        saveClusterPtr = saveCluster->write(saveClusterPtr);
        delete saveCluster;
    }
    mainCluster->write();
    fsync(QueueFile::fileDescriptor);
    for (auto i = 0; i <= lastInside; i++) {
        chain[i]->write();
    }
    mainCluster->header->fileSize = mainCluster->header->backupPtr;
    mainCluster->header->backupPtr = 0;
    mainCluster->header->queueLen++;
    mainCluster->write();
    fsync(QueueFile::fileDescriptor);
    truncate(filName, mainCluster->header->fileSize);
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
    pwrite64(QueueFile::fileDescriptor, buf + sizeof(SaveClusterHeader),
             header->saveDataSize, header->dataPtr);
}
