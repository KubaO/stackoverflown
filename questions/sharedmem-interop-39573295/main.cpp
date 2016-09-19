// https://github.com/KubaO/stackoverflown/tree/master/questions/sharedmem-interop-39573295
#include <QtCore>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <stdexcept>
#include <string>

struct Data {
    int a = 1;
    bool b = true;
    char c = 'S';
    bool operator==(const Data & o) const { return o.a == a && o.b == b && o.c == c; }
    static void compare(const void * a, const void * b) {
        auto data1 = reinterpret_cast<const Data*>(a);
        auto data2 = reinterpret_cast<const Data*>(b);
        Q_ASSERT(*data1 == *data2);
    }
};

void check(bool ok, const char * msg, const char * detail) {
    if (ok) return;
    std::string str{msg};
    str.append(": ");
    str.append(detail);
    throw std::runtime_error{str};
}
void check(int f, const char * msg) { check(f != -1, msg, strerror(errno)); }
void check(void * f, const char * msg) { check(f != MAP_FAILED, msg, strerror(errno)); }
void check(bool rc, const QSharedMemory & shm, const char * msg) { check(rc, msg, shm.errorString().toLocal8Bit()); }
void check(bool rc, const QFile & file, const char * msg) { check(rc, msg, file.errorString().toLocal8Bit()); }

struct noncopyable { Q_DISABLE_COPY(noncopyable) noncopyable() {} };
struct ShmId : noncopyable {
    int id;
    ShmId(int id) : id{id} {}
    ~ShmId() { if (id != -1) shmctl(id, IPC_RMID, NULL); }
};
struct ShmPtr : noncopyable {
    void * ptr;
    ShmPtr(void * ptr) : ptr{ptr} {}
    ~ShmPtr() { if (ptr != (void*)-1) shmdt(ptr); }
};
struct Handle : noncopyable {
    int fd;
    Handle(int fd) : fd{fd} {}
    ~Handle() { if (fd != -1) close(fd); }
};

void ipc_shm_test() {
    QTemporaryFile shmFile;
    check(shmFile.open(), shmFile, "shmFile.open");

    // SYSV SHM
    auto nativeKey = QFile::encodeName(shmFile.fileName());
    auto key = ftok(nativeKey.constData(), qHash(nativeKey, 'Q'));
    check(key, "ftok");
    ShmId id{shmget(key, sizeof(Data), IPC_CREAT | 0600)};
    check(id.id, "shmget");
    ShmPtr ptr1{shmat(id.id, NULL, 0)};
    check(ptr1.ptr, "shmat");
    new (ptr1.ptr) Data;

    // Qt
    QSharedMemory shm;
    shm.setNativeKey(shmFile.fileName());
    check(shm.attach(QSharedMemory::ReadOnly), shm, "shm.attach");
    auto ptr2 = shm.constData();

    Data::compare(ptr1.ptr, ptr2);
}

void mmap_test() {
    QTemporaryFile shmFile;
    check(shmFile.open(), "shmFile.open");
    shmFile.write({sizeof(Data), 0});
    check(true, shmFile, "shmFile.write");
    check(shmFile.flush(), shmFile, "shmFile.flush");

    // SYSV MMAP
    Handle fd{open(QFile::encodeName(shmFile.fileName()), O_RDWR)};
    check(fd.fd, "open");
    auto ptr1 = mmap(NULL, sizeof(Data), PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd.fd, 0);
    check(ptr1, "mmap");
    new (ptr1) Data;

    // Qt
    auto ptr2 = shmFile.map(0, sizeof(Data));

    Data::compare(ptr1, ptr2);
}

int main() {
    try {
        ipc_shm_test();
        mmap_test();
    }
    catch (const std::runtime_error & e) {
        qWarning() << e.what();
        return 1;
    }
    return 0;
}
