#include <QCoreApplication>
#include <QImage>
#include <QByteArray>

class AssertNoImageModifications {
    const char * m_bits;
    int m_len;
    quint16 m_checksum;
    Q_DISABLE_COPY(AssertNoImageModifications)
public:
    explicit AssertNoImageModifications(const QImage & image) :
        m_bits(reinterpret_cast<const char*>(image.bits())),
        m_len(image.byteCount())
    {
        Q_ASSERT((m_checksum = qChecksum(m_bits, m_len)) || true);
    }
    ~AssertNoImageModifications() {
        Q_ASSERT(m_checksum == qChecksum(m_bits, m_len));
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    const QImage i;
    AssertNoImageModifications ani1(i);

    return a.exec();
}
