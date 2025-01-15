#ifndef UTILS_HPP
#define UTILS_HPP

#include <utility>
#include <vector>
#include <set>
#include <string>
#include <cstring>

struct DataComparatorByLengthAndValue
{
    bool operator() (const std::string& ldata, const std::string& rdata) const 
    {
        if (ldata.length() < rdata.length())
        {
            return true;
        }
        else if (ldata.length() == rdata.length())
        {
            return std::strcmp(ldata.c_str(), rdata.c_str()) != 0;
        }
        return false;
    }

};
// common functor to parse cycle work
class UserCycleFunctor
{
public:
    using dataMap_t = std::multiset<std::string, DataComparatorByLengthAndValue>;

    void operator()(const std::string& data)
    {
        m_dataMap.insert(data);
    }

    const dataMap_t& result() const { return m_dataMap; }
    void clear() { m_dataMap.clear(); }
private:
    dataMap_t m_dataMap;

};

// common functor to collect all cycles work
class UserFunctor
{
public:
    using dataMap_t = std::vector<UserCycleFunctor>;

    void operator()(const UserCycleFunctor& cycleMapFunctor)
    {
        m_dataMap.emplace_back(cycleMapFunctor);
    }

    const dataMap_t& result() const { return m_dataMap; }
private:
    dataMap_t m_dataMap;

};



#endif