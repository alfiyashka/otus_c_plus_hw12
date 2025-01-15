#include "MapReduce.hpp"
#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <sstream>
#include "Utils.hpp"

std::ifstream::pos_type Map::filesize() const
{
    std::ifstream in(m_path.c_str(), std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

sections_t Map::split() const 
{
    const auto fileSize = filesize();
    if (fileSize <= 0)
    {
        std::cerr << "The file is empty or does not exist" << std::endl;
        return sections_t();
    }
    const auto chunkSize = fileSize / m_mnum;
    sections_t sections;

    std::ifstream in(m_path.c_str(), std::ifstream::ate | std::ifstream::binary);
    char symbol;
    auto firstOffset = 0;
    auto lastOffset = 0;
    int cycle = 1;

    do
    {
        const int nextLastOffset = chunkSize * cycle;
        if (lastOffset > nextLastOffset + chunkSize) // went over the current chunk
        {
            cycle++;
            continue;
        }
        lastOffset = nextLastOffset;
        in.seekg(lastOffset);
        in.read(&symbol, 1);
        while (symbol != '\n') // align to \n
        {
            if (in.read(&symbol, 1).eof())
            {
                break;
            }
            lastOffset ++;
        }
        if (lastOffset > fileSize)
        {
            lastOffset = fileSize;
        }
        sections.emplace_back(firstOffset, lastOffset);
        cycle++;

        firstOffset = lastOffset + 1;

    } while (lastOffset < fileSize && m_mnum >= cycle);
    
    return sections;
}

UserCycleFunctor Map::mapWork(const section_t& section) const
{
    std::ifstream in(m_path.c_str(), std::ifstream::ate | std::ifstream::binary);
    in.seekg(section.first);
    int readSize = 0;
    UserCycleFunctor mapFunctor;
    std::string data;
    const auto needRead = section.second - section.first;
    while (readSize < needRead)
    {
        if (needRead <= 0)
        {
            break;
        }
        if (std::getline(in, data).fail())
        {
            break;
        }
        readSize += data.length() + 1;
        mapFunctor(data);
    }
    return mapFunctor;
}

UserFunctor Map::map(const sections_t& sections) const
{
    std::vector<std::future<UserCycleFunctor>> futures;
    for (const auto& section : sections)
    {
        //mapWork(section);
        futures.emplace_back(std::async(std::launch::async,
            &Map::mapWork, this, section));
    }
    UserFunctor res;
    for (auto &f : futures)
    {
        if (f.valid())
            res(f.get());
    }
    return res;
}

UserFunctor Reduce::shuffle(const UserFunctor& res) const
{
    using dataMap_t = std::multiset<std::string>;
    dataMap_t dataMap;
    for (const auto& iData : res.result())
    {
        dataMap.insert(iData.result().begin(), iData.result().end());
    }
    const auto chunkSize = dataMap.size() / m_rnum;
    UserFunctor resShuffle;
    UserCycleFunctor cycleRes;
    int i = 0;
    int iCycle = 1;
    std::string lastParsedData;
    for (const auto& iData : dataMap)
    {
        if (i >= iCycle * chunkSize 
          && iData != lastParsedData) //try to set same data into same container
        {
            
            while (i >= (iCycle + 1) * chunkSize) //went over 
            {
                iCycle++;
            }
            resShuffle(cycleRes);
            cycleRes.clear();
            iCycle++;
        }
        cycleRes(iData);
        i++;
        lastParsedData = iData;
    }
    resShuffle(cycleRes);
    return resShuffle;
}


std::string Reduce::generateReduceFileNameWithCurrentTime() const
{
    std::ostringstream idStream;
    idStream << "Thread_" << std::this_thread::get_id();

    const auto threadId = idStream.str();

    const auto current_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return "reduce" + std::to_string(current_time) + threadId + ".log";
}


void Reduce::reduceWork(const UserCycleFunctor& data) const
{

    const auto filename = generateReduceFileNameWithCurrentTime();
    std::ofstream file;
    file.open(filename, std::fstream::out);

    for (const auto &dataToWrite : data.result())
    {
        file << dataToWrite << std::endl;
    }
    file.close();

    return;
}

std::string Reduce::reduce(const UserFunctor &func) const
{
    const auto &shuffleResult = func.result();
    std::vector<std::future<void>> futures;
    UserCycleFunctor::dataMap_t reduceRes;
    for (const auto& iRes : shuffleResult)
    {
        futures.emplace_back(std::async(std::launch::async,
                                        &Reduce::reduceWork, this, iRes));
        auto isShuffleResultSet = iRes.result();
        if (!isShuffleResultSet.empty())
        {
            reduceRes.insert(*isShuffleResultSet.begin());
        }
    }
    for (auto &f : futures)
    {
        if (f.valid())
            f.get();
    }
    if (!reduceRes.empty())
    {
        return *reduceRes.begin();
    }
    else
    {
        return "";
    }
}