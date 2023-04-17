#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>

#ifndef HUFFMAN_H
#define HUFFMAN_H

template <class T>
struct node {
    T value;
    size_t left;
    size_t right;
    node (T Value=0, size_t Left=0, size_t Right=0): value(Value), left(Left), right(Right) {}
};

template <class T>
class huffman {
   private:
    void DFS(std::vector<node<T> > &, std::unordered_map<T, std::string> &, size_t, std::string);
    uint8_t swapByteOrder(uint8_t);
    uint16_t swapByteOrder(uint16_t);
    uint32_t swapByteOrder(uint32_t);
    uint64_t swapByteOrder(uint64_t);
    T *sourceData = nullptr;
    uint64_t sourceLen = 0;

    T *encodedData = nullptr;
    uint64_t encodedLen = 0;
    uint64_t encodedBitLen = 0;
    uint64_t originalLen = 0;

    std::unordered_map<T, std::string> codeTable;
    std::unordered_map<T, uint64_t> numberOfTimes;
    std::unordered_map<std::string, T> decodeTable;

   public:
    void loadSource(const std::string filename);
    void buildTable();
    void expectLength();
    void encode(std::string);
    void loadTable(std::string);
    void decode(std::string);

    void compress(std::string);
    void decompress(std::string);
};

#endif