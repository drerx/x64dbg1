#ifndef TRACEFILEREADER_H
#define TRACEFILEREADER_H

#include "Bridge.h"
#include <QFile>
#include <atomic>

class QFileSystemWatcher;
class TraceFileParser;
class TraceFilePage;

class TraceFileReader : public QObject
{
    Q_OBJECT
public:
    TraceFileReader(QObject* parent = NULL);
    bool Open(const QString & fileName);
    void Close();
    bool isError();
    int Progress();

    unsigned long long Length();

    REGDUMP Registers(unsigned long long index);
    void OpCode(unsigned long long index, unsigned char* buffer, int* opcodeSize);
    DWORD ThreadId(unsigned long long index);
    int MemoryAccessCount(unsigned long long index);
    void MemoryAccessInfo(unsigned long long index, duint* address, duint* oldMemory, duint* newMemory, bool* isValid);

signals:
    void parseFinished();

public slots:
    void parseFinishedSlot();
    void fileUpdatedSlot(const QString & fileName);

private:
    typedef std::pair<unsigned long long, unsigned long long> Range;
    struct RangeCompare //from addrinfo.h
    {
        bool operator()(const Range & a, const Range & b) const //a before b?
        {
            return a.second < b.first;
        }
    };

    QFile traceFile;
    QFileSystemWatcher* streamMonitor;
    unsigned long long length;
    std::vector<std::pair<unsigned long long, Range>> fileIndex; //index;<file offset;length>
    std::atomic<int> progress;
    bool error;
    TraceFilePage* lastAccessedPage;
    unsigned long long lastAccessedIndexOffset;
    friend class TraceFileParser;
    friend class TraceFilePage;

    TraceFileParser* parser;
    std::map<Range, TraceFilePage, RangeCompare> pages;
    TraceFilePage* getPage(unsigned long long index, unsigned long long* base);
};

#endif //TRACEFILEREADER_H