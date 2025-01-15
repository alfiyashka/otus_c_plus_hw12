#include <iostream>

#include "src/MapReduce.hpp"

using namespace std;


int main(int argc, const char *argv[])
{
  if (argc <= 3)
  {
    std::cerr << "The required parameters(path, mnum, rnum) are not defined!" << std::endl;
    return -1;
  }
  try
  {
    const auto path = argv[1];
    const auto mnum = argv[2];
    const auto rnum = argv[3];
    
    MapReduce mapReduce(atoi(mnum), atoi(rnum), path);
    const auto mapReduceRes = mapReduce.run();
    std::cout<<"Result - '" << mapReduceRes << "'" <<std::endl;
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
  } 
  return 0;
}
