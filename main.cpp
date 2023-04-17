#include <fstream>
#include <iostream>
#include <string>

#include "huffman.cpp"

using namespace std;

template <class T>
void test(string name, string filename) {
    cout << name << endl;
    huffman<T> h;
    h.compress(filename);

    huffman<T> hd;
    hd.decompress(filename);
    cout << endl;
}

int main(int argc, char *argv[]) {
    // TODO Use arguments!!

    // test<uint8_t>("8-bit", argv[1]);
    // test<uint16_t>("16-bit", argv[1]);
    test<uint32_t>("32-bit", argv[1]);
    // test<uint64_t>("64-bit", argv[1]);

    return 0;
}