#include "UniqueIdGenerator.hpp"

std::mutex UniqueIdGenerator::m_mutex;
std::unordered_set<int32_t> UniqueIdGenerator::m_ids;

auto UniqueIdGenerator::generate_random_unique_id() -> int32_t
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    // static std::uniform_int_distribution<int32_t> dis(0, std::numeric_limits<int32_t>::max());
    static std::uniform_int_distribution<int32_t> dis(0, 9999);

    std::unique_lock lock{m_mutex, std::defer_lock};

    int32_t id{-1};
    bool alreadyTaken{true};

    do 
    {
        id = dis(gen);

        lock.lock();
        alreadyTaken = m_ids.contains(id);
        lock.unlock();
    } 
    while (alreadyTaken || id < 0);

    lock.lock();
    m_ids.insert(id);
    lock.unlock();

    return id;
}
