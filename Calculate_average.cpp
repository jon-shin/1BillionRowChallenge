#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <algorithm>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <cmath>
#include <thread>
#include <vector>
#include <iomanip>
using namespace std;
using namespace std::chrono;

class Stats{
    public: 
        double min;
        double sum;
        double max;
        double count;
        Stats(){
            min = 100000000.0;
            max = -100000000.0;
            sum = 0.0;
            count = 0.0; 
        }
        friend ostream& operator<<(ostream& os, const Stats& stats){
          double mean = std::round((stats.sum *10)/stats.count)/10;
          os  << stats.min << "/" << std::fixed << std::setprecision(1) << mean << "/" << stats.max;
          return os;
        }
};

void doChunk(char * file_data, size_t size, unordered_map<string, Stats> &mp){
  string s, city;
  double temp;
  char * start_pos = file_data; 
  while(start_pos < file_data + size){
    char * end_pos = strchr(start_pos, '\n');
    if (end_pos == nullptr) {
      break;
    }
    s = string(start_pos, end_pos - start_pos);
    istringstream line(s);
    if (getline(line, city, ';') && (line >> temp)) {
        Stats& stats = mp[city];
        if(temp < stats.min){
            stats.min = temp; 
        }
        if(temp > stats.max){
            stats.max = temp;
        }
        stats.sum += temp;
        stats.count++;
    }
    start_pos = end_pos + 1;
  }
  
}

//self-note: file is "../1brc/measurements.txt"
int main(int argc, char *argv[]){
  int fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    perror("Could not open file");
    return 1;
  }
  struct stat sb;
  if (fstat(fd, &sb) == -1) {
    perror("Could not get file size");
    close(fd);
    return 1;
  }
  char *file_data = static_cast<char*>(mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
  if (file_data == MAP_FAILED) {
    perror("Could not map file data");
    close(fd); 
    return 1;
  }
  size_t threadCount = 8;
  size_t chunkSize = sb.st_size / threadCount;
  vector<thread> threads;
  vector<unordered_map<string, Stats>> statMaps(threadCount);
  
  for(size_t i = 0; i < threadCount; ++i){
    char *offset = file_data + (i * chunkSize);
    size_t size = (i == threadCount - 1) ? sb.st_size - i * chunkSize : chunkSize;
    threads.emplace_back(doChunk, offset, size, ref(statMaps[i]));
  }
  
  for (auto& thread : threads) {
    thread.join();
  }

  unordered_map<string, Stats> combinedStats;
  for(auto& statMap: statMaps){
    for(auto& s: statMap){
      Stats& stats = combinedStats[s.first];
        if(s.second.min < stats.min){
            stats.min = s.second.min; 
        }
        if(s.second.max > stats.max){
            stats.max = s.second.max;
        }
        stats.sum += s.second.sum;
        stats.count+= s.second.count;
    }
  }
  vector<string> cities;
  for (const auto& s : combinedStats) {
    cities.emplace_back(s.first);
  }
  std::sort(cities.begin(), cities.end());
  vector<string>::iterator it = cities.begin();
  cout << "{";
  while (it != cities.end()) {
    Stats& stat = combinedStats[*it];
    cout << *it << "=" << stat;
    if(++it != cities.end()){
      cout << ", ";
    }
  }
  cout << "}\n";

  munmap(file_data, sb.st_size);
  close(fd);
  return 0;
}