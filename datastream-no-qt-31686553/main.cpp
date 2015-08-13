#include <vector>
#include <memory>
#include <cstdint>
#include <cassert>

class QDataStream {
public:
    typedef std::vector<uint8_t> Storage;
    enum ByteOrder { BigEndian, LittleEndian };
    enum Status { Ok, ReadPastEnd };
private:
    std::shared_ptr<const Storage> m_ptr;
    const Storage * m_data;
    size_t m_idx;
    Status m_status;
    ByteOrder m_byteOrder, m_systemOrder;
    static ByteOrder systemByteOrder() {
        const uint32_t t = 1;
        return (reinterpret_cast<const uint8_t*>(&t)) ? LittleEndian : BigEndian;
    }
    bool has(size_t count) const { return m_idx + count <= m_data->size(); }
    template <typename T> QDataStream & read(T & i) {
        if (has(sizeof(T)) && Ok == m_status) {
            T result = *reinterpret_cast<const T*>(&(*m_data)[m_idx]);
            m_idx += sizeof(T);
            if (m_byteOrder != m_systemOrder) {
                T tmp = 0;
                for (uint8_t i = 0; i < sizeof(T); ++i) {
                    tmp = (tmp << 8) | (result & 0xFF);
                    result = result >> 8;
                }
                i = tmp;
            } else
                i = result;
        } else {
            m_status = ReadPastEnd;
        }
        return *this;
    }
public:
    QDataStream(const std::vector<uint8_t> * data) :
        m_data(data), m_idx(0), m_status(Ok),
        m_byteOrder(BigEndian), m_systemOrder(systemByteOrder()) {}
    QDataStream(std::shared_ptr<Storage> data) :
        m_ptr(data), m_data(m_ptr.get()), m_idx(0), m_status(Ok),
        m_byteOrder(BigEndian), m_systemOrder(systemByteOrder()) {}
    QDataStream(std::unique_ptr<Storage> && data) :
        m_ptr(data.release()), m_data(m_ptr.get()), m_idx(0), m_status(Ok),
        m_byteOrder(BigEndian), m_systemOrder(systemByteOrder()) {}
    QDataStream(Storage && data) :
        m_ptr(new Storage(std::move(data))), m_data(m_ptr.get()),
        m_idx(0), m_status(Ok), m_byteOrder(BigEndian), m_systemOrder(systemByteOrder()) {}
    bool atEnd() const { return m_idx == m_data->size(); }
    QDataStream & operator>>(int8_t & i) {
        return read(i);
    }
    QDataStream & operator>>(int16_t & i) {
        return read(i);
    }
    QDataStream & operator>>(int32_t & i) {
        return read(i);
    }
    QDataStream & operator>>(uint8_t & i) {
        return read(i);
    }
    QDataStream & operator>>(uint16_t & i) {
        return read(i);
    }
    QDataStream & operator>>(uint32_t & i) {
        return read(i);
    }
    void setByteOrder(ByteOrder b) { m_byteOrder = b; }
    ByteOrder byteOrder() const { return m_byteOrder; }
};

int main()
{
    std::vector<uint8_t> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    uint32_t val;
    QDataStream s_be(&v);
    s_be >> val;
    assert(val == 0x01020304); // big endian
    QDataStream s_le(&v);
    s_le.setByteOrder(QDataStream::LittleEndian);
    s_le >> val;
    assert(val == 0x04030201); // little endian
    return 0;
}
