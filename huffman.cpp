#include "huffman.h"

#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <cmath>

#define BUFFER_SIZE 1048576

void printHex(uint8_t data) {
    std::cout << std::hex << std::uppercase << (data < 10 ? "0" : "\0") << (unsigned)data << std::endl;
}

template <class T>
void huffman<T>::loadSource(const std::string filename) {

#ifndef TIMER
    std::cout << "[STATS] Now loading the file." << std::endl;
#endif

    std::ifstream is(filename, std::ifstream::binary);

    if (is.fail()) {
        std::cerr << "[ERROR] Cannot open the file." << std::endl;
        exit(1);
    }

    is.seekg(0, is.end);
    originalLen = is.tellg();
    is.seekg(0, is.beg);

    sourceLen = originalLen / sizeof(T) + ((originalLen % sizeof(T) > 0) ? 1 : 0);
    sourceData = new T[sourceLen]();

    // sourceData = (T *)calloc(sourceLen, sizeof(T));

    is.read((char *)sourceData, originalLen);

    if (originalLen != is.gcount()) {
        std::cerr << "[ERROR] File does not read successfully" << std::endl;
    }
}

template <class T>
void huffman<T>::buildTable() {
    if (sourceData == nullptr) {
        std::cerr << "[ERROR] No file is loaded" << std::endl;
        exit(1);
    }

#ifndef TIMER
    std::cout << "[STATS] Now building the table." << std::endl;
#endif

#ifdef PMF
    std::map<T, std::vector<std::uint64_t> > pmfHistory;
    std::vector<uint64_t> pmfHeader;
#endif

#ifdef TIMER
    std::cout << "[TIMER] Start counting frequencies of source file for... " << std::flush;
    clock_t startTimer = clock();
#endif

    T *nowPointer = sourceData;
    for (size_t i = 0; i < sourceLen; ++i) {

#ifdef PMF
        if ((i * sizeof(T)) % (40 * 1024 * 1024) == 0) {
            pmfHeader.push_back(i * sizeof(T));
            for (auto &it : numberOfTimes) {
                if (pmfHistory[it.first].size() == 0) pmfHistory[it.first].resize(pmfHeader.size() - 1, 0);
                pmfHistory[it.first].push_back(it.second);
            }
        }
#endif

        numberOfTimes[*nowPointer++]++;
    }

#ifdef PMF
    pmfHeader.push_back(sourceLen * sizeof(T));
    for (auto &it : numberOfTimes) {
        if (pmfHistory[it.first].size() == 0) pmfHistory[it.first].resize(pmfHeader.size() - 1, 0);
        pmfHistory[it.first].push_back(it.second);
    }

    std::ofstream os("pmf.csv", std::ofstream::out);
    os << "_,";
    for (auto &it : pmfHeader) os << it << ",";
    os << "\n";
    for (auto &it : pmfHistory) {
        os << "_" << std::hex << std::uppercase << (uint64_t)it.first << "," << std::dec;
        for (auto &jt : it.second) {
            os << jt << ",";
        }
        os << "\n";
    }
    os.close();
#endif

#ifdef TIMER
    std::cout << std::fixed << ((double)clock() - startTimer) / CLOCKS_PER_SEC << " seconds." << std::endl;
#endif

    // Calculate entropy
    long double entropy = 0;
    long double sum = sourceLen;
    for (auto it: numberOfTimes) {
        entropy += (long double)it.second / sourceLen * log2l((long double)sourceLen / it.second);
    }
    std::cout << "[STATS] The entropy of the file is " << std::fixed << std::setprecision(2) << entropy << " bits." << std::endl;


#ifdef TIMER
    std::cout << "[TIMER] Start constructing huffman tree for... " << std::flush;
    startTimer = clock();
#endif

    std::vector<node<T> > nodes;
    std::priority_queue<std::pair<uint64_t, size_t>, std::vector<std::pair<uint64_t, size_t> >, std::greater<> > pq;

    for (auto &it : numberOfTimes) {
        pq.emplace(it.second, nodes.size());
        nodes.emplace_back(it.first);
    }

    while (pq.size() > 1) {
        auto [aW, aIdx] = pq.top();
        pq.pop();
        auto [bW, bIdx] = pq.top();
        pq.pop();

        pq.emplace(aW + bW, nodes.size());
        nodes.emplace_back(0, aIdx, bIdx);
    }

    DFS(nodes, codeTable, nodes.size() - 1, "");

#ifdef TIMER
    std::cout << std::fixed << ((double)clock() - startTimer) / CLOCKS_PER_SEC << " seconds." << std::endl;
#endif

#ifdef DEBUG
    std::cout << "[DEBUG] Data in coding table." << std::endl;
    for (auto &it : codeTable) {
        std::cout << std::hex << std::uppercase << (unsigned)it.first << "\t" << it.second << std::endl;
    }
#endif
}

template <class T>
void huffman<T>::DFS(std::vector<node<T> > &nodes, std::unordered_map<T, std::string> &m, size_t index, std::string encoded) {
    if (nodes[index].left != nodes[index].right) {
        DFS(nodes, m, nodes[index].left, encoded + "0");
        DFS(nodes, m, nodes[index].right, encoded + "1");
    } else {
        m[nodes[index].value] = encoded;
    }
}

// Encode data and write to the file.
template <class T>
void huffman<T>::encode(std::string filename) {
    // Encode data and write to the file.
    std::string strEncoded;
    std::ofstream os(filename + ".hc", std::ostream::binary);

#ifndef TIMER
    std::cout << "[STATS] Now encoding the file." << std::endl;
#endif

#ifdef TIMER
    std::cout << "[TIMER] Start encoding and writing to file for... " << std::flush;
    clock_t startTimer = clock();
#endif

    uint64_t encodedBitLen = 0;

    for (int i = 0; i < sourceLen; ++i) {
        strEncoded += codeTable[sourceData[i]];

        if (strEncoded.size() > BUFFER_SIZE) {
            // std::cout << strEncoded.substr(0, 48) << std::endl;
            for (int j = 0; j < BUFFER_SIZE; j += 64) {
                encodedBitLen += 64;
                std::bitset<64> bs(strEncoded.substr(j, 64));
                uint64_t tmp = swapByteOrder((uint64_t)bs.to_ullong());
                os.write((char *)&tmp, 8);
            }
            strEncoded.erase(0, BUFFER_SIZE);
        }
    }

    encodedBitLen += strEncoded.size();
    while (strEncoded.size() % 8 != 0) strEncoded += '0';

    for (int j = 0; j < strEncoded.size(); j += 8) {
        std::bitset<8> bs(strEncoded.substr(j, 8));
        uint8_t tmp = bs.to_ullong();
        os.write((char *)&tmp, 1);
    }

#ifdef TIMER
    std::cout << std::fixed << ((double)clock() - startTimer) / CLOCKS_PER_SEC << " seconds." << std::endl;
#endif

    // Write coding table to the file.
    // TODO For easier implementation, the file doesn't compact enough to be evaluate in the compression performance.
    std::ofstream osMap(filename + ".hct", std::ostream::binary);
    osMap << sizeof(T) << " " << originalLen << " " << encodedBitLen << "\n";
    for (auto &it : codeTable) {
        osMap << std::hex << std::uppercase << (uint64_t)it.first << " " << it.second << "\n";
    }
    std::cout << "[STATS] The size of the table is " << osMap.tellp() << " bytes." << std::endl;

    osMap.close();

    std::cout << "[STATS] Original size: " << std::dec << sourceLen * sizeof(T) << " bytes.\n"
              << "[STATS] Compressed size: " << os.tellp() << " bytes.\n"
              << "[STATS] Compression rate: " << std::fixed << std::setprecision(2) << (double)os.tellp() / (sourceLen * sizeof(T)) * 100 << "%" << std::endl;

    os.close();
}

template <class T>
void huffman<T>::expectLength() {
    double ans = 0;

    for (auto &it : codeTable) {
        ans += it.second.size() * ((double)numberOfTimes[it.first] / sourceLen);
    }
    std::cout << "[STATS] Expect length of codewords: " << ans << std::endl;
}

template <class T>
void huffman<T>::loadTable(std::string filename) {

#ifndef TIMER
    std::cout << "[STATS] Now loading the table." << std::endl;
#endif

#ifdef TIMER
    std::cout << "[TIMER] Start loading table for... " << std::flush;
    clock_t startTimer = clock();
#endif

    std::ifstream is(filename);

    if (is.fail()) {
        std::cerr << "[ERROR] Cannot open the file." << std::endl;
        exit(1);
    }

    uint64_t encodedSize;
    is >> std::dec >> encodedSize >> originalLen >> encodedBitLen;
    if (sizeof(T) != encodedSize) {
        std::cerr << "[ERROR] The encoded size of the table doesn't match." << std::endl;
        exit(1);
    }
    uint64_t alphabet;
    std::string codeword;
    while (is >> std::hex >> alphabet >> codeword) {
        decodeTable[codeword] = alphabet;
    }
    is.close();

#ifdef TIMER
    std::cout << std::fixed << ((double)clock() - startTimer) / CLOCKS_PER_SEC << " seconds." << std::endl;
#endif

}

template <class T>
void huffman<T>::decode(std::string filename) {
    std::ofstream os(filename, std::ofstream::binary);
    std::string buffer;

#ifndef TIMER
    std::cout << "[STATS] Now decoding the file." << std::endl;
#endif

#ifdef TIMER
    std::cout << "[TIMER] Start decoding and writing to file for... " << std::endl;
    clock_t startTimer = clock();
#endif

    uint64_t bitLen = 0;
    uint64_t byteLen = 0;
    for (int i = 0; i < sourceLen; ++i) {
        std::bitset<sizeof(T) * 8> bs((uint64_t)swapByteOrder(sourceData[i]));
        buffer += bs.to_string();

        // TODO If the codeword is longer than BUFFER_SIZE, it may stuck at the buffer.
        if (buffer.size() > BUFFER_SIZE) {
            uint64_t start = 0, len = 0;
            while (start + len < BUFFER_SIZE) {
                len++;
                if (decodeTable.count(buffer.substr(start, len))) {
                    os.write((char *)&(decodeTable[buffer.substr(start, len)]), sizeof(T));
                    byteLen += sizeof(T);
                    start += len;
                    bitLen += len;
                    len = 0;
                }
            }
    
#ifndef TIMER
            std::cout << "[STATS] Decoding... " << std::fixed << std::setprecision(2) << (double)byteLen / originalLen * 100 << "%\r" << std::flush;
#endif

            buffer = buffer.erase(0, start);
        }
    }

    int start = 0, len = 0;
    while (start + len < buffer.size() && encodedBitLen > bitLen && originalLen > byteLen) {
        len++;
        if (decodeTable.count(buffer.substr(start, len))) {
            if (originalLen - byteLen < sizeof(T)) {
                os.write((char *)&(decodeTable[buffer.substr(start, len)]), originalLen - byteLen);
                byteLen += originalLen - byteLen;
            } else {
                os.write((char *)&(decodeTable[buffer.substr(start, len)]), sizeof(T));
                byteLen += sizeof(T);
            }
            start += len;
            bitLen += len;
            len = 0;
        }
    }
    buffer = buffer.erase(0, start);

#ifdef TIMER
    std::cout << std::fixed << ((double)clock() - startTimer) / CLOCKS_PER_SEC << " seconds." << std::endl;
#endif

    if (encodedBitLen != bitLen) {
        std::cerr << "[ERROR] Cannot decode properly. Either encoded file or table is broken." << std::endl;
        os.close();
        exit(1);
    }

    os.close();
}

template <class T>
uint8_t huffman<T>::swapByteOrder(uint8_t c) {
    return c;
}

template <class T>
uint16_t huffman<T>::swapByteOrder(uint16_t us) {
    return (us >> 8) |
           (us << 8);
}

template <class T>
uint32_t huffman<T>::swapByteOrder(uint32_t ui) {
    return (ui >> 24) |
           ((ui << 8) & 0x00FF0000) |
           ((ui >> 8) & 0x0000FF00) |
           (ui << 24);
}

template <class T>
uint64_t huffman<T>::swapByteOrder(uint64_t ull) {
    return (ull >> 56) |
           ((ull << 40) & 0x00FF000000000000) |
           ((ull << 24) & 0x0000FF0000000000) |
           ((ull << 8) & 0x000000FF00000000) |
           ((ull >> 8) & 0x00000000FF000000) |
           ((ull >> 24) & 0x0000000000FF0000) |
           ((ull >> 40) & 0x000000000000FF00) |
           (ull << 56);
}

template <class T>
void huffman<T>::compress(std::string filename) {
    loadSource(filename);
    buildTable();
    expectLength();
    encode(filename);
}

template <class T>
void huffman<T>::decompress(std::string filename) {
    loadSource(filename + ".hc");
    loadTable(filename + ".hct");
    decode(filename + "_decoded");
}