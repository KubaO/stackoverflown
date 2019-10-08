#include <algorithm>
#include <cassert>
#include <cmath>
#include <future>
#include <iostream>
#include <numeric>
#include <vector>

using real = float;

struct Vector { 
    real x, y, z;
    constexpr real len() const {
        return sqrt(x*x + y*y + z*z);
    }
    constexpr Vector operator-(const Vector &o) const {
        return {x-o.x, y-o.y, z-o.z};
    }
};

struct Particle {
    Vector pos, vel;
};

constexpr static real distance(const Particle &a, const Particle &b) {
    return (b.pos - a.pos).len();
}

real potential_diff(const Particle &a, const Particle &b) {
    constexpr real A = 2.0f, B = 3.0f, C = 12.0f;
    const real d = distance(a, b);
    const real d6 = d*d*d*d*d*d;
    const real d12 = d6*d6;
    // As d decreases, the higher the power, the sooner it will round to zero.
    return
        (d12 != 0.f) ? A/d12 - B/d6 + C/d :
        (d6 != 0.f) ? -B/d6 + C/d :
        (d != 0.f) ? C/d : 0.f;
}

struct System {
    using particle_storage = std::vector<Particle>;
    using const_particle_iterator = particle_storage::const_iterator;
    particle_storage particles;
    std::vector<real> potential_diffs;
    
    struct PotentialEnergyCalculator {
        const_particle_iterator const cbegin, cend;
        real operator()() const {
            thread_local static std::vector<real> potential_diffs;
            potential_diffs.resize(0);
            // calculate potential differences from this particle to those after it
            for (auto ip2 = std::next(cbegin); ip2 != cend; ++ip2)
                potential_diffs.push_back(potential_diff(*cbegin, *ip2));
            // sort the potential differences in ascending order
            std::sort(potential_diffs.begin(), potential_diffs.end());
            // add potential differences - in ascending order for stability
            return std::accumulate(potential_diffs.cbegin(), potential_diffs.cend(), 0.f);
        }
    };
    
    real calculate_potential_energy(std::launch policy = std::launch::deferred) {
        // obtain potential differences for all particles
        std::vector<std::future<real>> potential_diff_futures;
        potential_diff_futures.reserve(particles.size());
        
        for (auto ip1 = particles.cbegin(); ip1 != particles.cend(); ++ip1) {
            potential_diff_futures.emplace_back(std::async([ip1, ip2 = particles.cend()]{ 
                return PotentialEnergyCalculator{ip1, ip2}(); 
            }));
        }

        potential_diffs.resize(0);
        potential_diffs.reserve(particles.size());
        for (auto &f : potential_diff_futures)
            potential_diffs.push_back(f.get());
        // arrange in ascending order
        std::sort(potential_diffs.begin(), potential_diffs.end());
        // add potiential differences - in ascending order for stability
        return std::accumulate(potential_diffs.cbegin(), potential_diffs.cend(), 0.f);
    }
};

void test(bool diagn) {
    System sys;
    sys.particles.resize(3);
    auto &ps = sys.particles;
    ps[0].pos = {1,0,0};
    ps[1].pos = {1,1,0};
    ps[2].pos = {0,0,1};
    real const p_dst01 = distance(ps[0], ps[1]);
    real const p_dst02 = distance(ps[0], ps[2]);
    real const p_dst12 = distance(ps[1], ps[2]);
    if (diagn)
        std::cout << "dst01=" << p_dst01 << " dst02=" << p_dst02 << " dst12=" << p_dst12 << std::endl;
    assert(p_dst01 == 1.f);
    assert(p_dst02 == sqrtf(2.f));
    assert(p_dst12 == sqrtf(3.f));
    real const p_en01 = potential_diff(ps[0], ps[1]);
    real const p_en02 = potential_diff(ps[0], ps[2]);
    real const p_en12 = potential_diff(ps[1], ps[2]);
    real const p_sum = p_en12 + p_en02 + p_en01;
    
    real const p_en = sys.calculate_potential_energy();
    
    if (diagn)
        std::cout 
            << "en01=" << p_en01 << " en02=" << p_en02 << " en12=" << p_en12
            << "\nen_sum=" << p_sum << " en=" << p_en << std::endl;
    assert(p_sum == p_en);
}

int main()
{
    test(false);
}
