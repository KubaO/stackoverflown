// https://github.com/KubaO/stackoverflown/tree/master/questions/mmap-boost-40308164
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/filesystem.hpp>
#include <cassert>
#include <cstdint>
#include <fstream>

namespace bip = boost::interprocess;

void fill(const char * fileName, size_t size) {
    using element_type = uint64_t;
    assert(size % sizeof(element_type) == 0);
    std::ofstream().open(fileName); // create an empty file
    boost::filesystem::resize_file(fileName, size);
    auto mapping = bip::file_mapping{fileName, bip::read_write};
    auto mapped_rgn = bip::mapped_region{mapping, bip::read_write};
    const auto mmaped_data = static_cast<element_type*>(mapped_rgn.get_address());
    const auto mmap_bytes = mapped_rgn.get_size();
    const auto mmap_size = mmap_bytes / sizeof(*mmaped_data);
    assert(mmap_bytes == size);

    element_type n = 0;
    for (auto p = mmaped_data; p < mmaped_data+mmap_size; ++p)
       *p = n++;
}

int main() {
   const uint64_t G = 1024ULL*1024ULL*1024ULL;
   fill("tmp.bin", 1*G);
}
