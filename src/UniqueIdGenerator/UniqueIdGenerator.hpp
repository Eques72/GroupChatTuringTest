#pragma once

#include <mutex>
#include <cstdint>
#include <unordered_set>
#include <random>


class UniqueIdGenerator
{
public:

    static auto generate_random_unique_id() -> int32_t;

private:
    
    static std::mutex m_mutex;
    static std::unordered_set<int32_t> m_ids;

};
