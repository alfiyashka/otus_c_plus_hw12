#ifndef MAP_REDUCE
#define MAP_REDUCE

#include <string>
#include <vector>
#include <fstream>
#include <set>
#include "Utils.hpp"

using sections_t = std::vector<std::pair<int, int>>;
using section_t = std::pair<int, int>; // offsets

using element_t = std::string;

// map functor
struct Map
{
public:
  Map(const std::string &path, const int mnum) noexcept
      : m_path(path), m_mnum(mnum) {}
  UserFunctor operator()() const { return map(split()); }

private:
  sections_t split() const;
  /**
   * map data
   */
  UserFunctor map(const sections_t &sections) const;
  std::ifstream::pos_type filesize() const;
  /**
   * map threads work
   */
  UserCycleFunctor mapWork(const section_t &section) const;

  const int m_mnum;         // map threads count
  const std::string m_path; // full path to a file to parse
};

// reduce functor
struct Reduce
{
public:
  Reduce(const int rnum) : m_rnum(rnum) {}

  void operator()(const Map &map) const
  {
    reduce(shuffle(map()));
  }

private:
  UserFunctor shuffle(const UserFunctor &res) const;

  /**
   * reduce threads work
   */
  void reduceWork(const UserCycleFunctor &data) const;

  std::string generateReduceFileNameWithCurrentTime() const;

  /**
   * reduces data
   */
  void reduce(const UserFunctor &res) const;
  const int m_rnum; // reduce threads count
};

class MapReduce
{
public:
  MapReduce(const int mnum, const int rnum, const std::string& path) noexcept
     : m_path(path), m_mnum(mnum), m_rnum(rnum){}
  void run()
  {
    const Map m(m_path, m_mnum);
    const Reduce r(m_rnum);
    r(m);
  }
private:
  const int m_mnum;         // map threads count
  const std::string m_path; // full path to a file to parse
  const int m_rnum; // reduce threads count
};

#endif