
#ifndef VRANDOM 
#define VRANDOM 
 
#include "vlogger.h"
#include "vtypes.h"
#include <random>
#include <limits>
#include <cmath>
 
namespace rng {

    namespace detail {
        // One engine per thread. Seeded the first time it's used on that thread.
        inline std::mt19937& engine() {
            thread_local std::mt19937 eng{ std::random_device{}() };
            return eng;
        }
    } 
     
    // Reseed this thread's engine.
    inline void seed(std::mt19937::result_type s) {
        detail::engine().seed(s);
    }
     
    
    // Random i32 in [min, max].
    inline i32 randi32(i32 min, i32 max) {
        if (min > max) std::swap(min, max);
        std::uniform_int_distribution<i32> dist(min, max);
        return dist(detail::engine());
    }
     
    // Random i64 in [min, max].
    inline i64 randi64(i64 min, i64 max) {
        if (min > max) std::swap(min, max);
        std::uniform_int_distribution<i64> dist(min, max);
        return dist(detail::engine());
    }
     
    // Random f32 in [min, max].
    inline f32 randf32(f32 min, f32 max) {
        if (min > max) std::swap(min, max);
        if (!std::isfinite(min) || !std::isfinite(max)) {
            V_LOG_ERROR("NOT FINITE");
        }
        std::uniform_real_distribution<f32> dist(
            min, std::nextafter(max, std::numeric_limits<f32>::max()));
        return dist(detail::engine());
    }
     
    // Random f64 in [min, max].
    inline f64 randf64(f64 min, f64 max) {
        if (min > max) std::swap(min, max);
        if (!std::isfinite(min) || !std::isfinite(max)) {
            V_LOG_ERROR("NOT FINITE");
        }
        std::uniform_real_distribution<f64> dist(
            min, std::nextafter(max, std::numeric_limits<f64>::max()));
        return dist(detail::engine());
    }

}
 
#endif // RANDOM_UTILS_H
 
