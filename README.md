# Huffman Coding

## Build
Make sure you have installed `g++` and `make`.
Computer must be run in **little-endian** mode.

Has been compiled and tested on:
- arm64
    - `Apple clang version 14.0.3`
    - `g++-12 (Homebrew GCC 12.2.0) 12.2.0`
- x86_64
    - `g++ (GCC) 12.1.1 20220628 (Red Hat 12.1.1-3)`

Then simply run following to build the application.
```shell
make
```
### Modes
If you want to build with other functions, you can build it with corresponding commands.

| Options       | Command      | Description                                                               |
|---------------|--------------|---------------------------------------------------------------------------|
| Default       | `make`       | It will print out current progress.                                       |
| Timer         | `make timer` | It will measure the timing of the application.                            |
| Debug         | `make debug` | It will print out more information.                                       |
| Calculate PMF | `make pmf`   | It will produce a `pmf.csv` file with pmf of the source file every 40 MB. |

## Run
To run the program, just execute it with filename as the parameter.
```shell
./hfc filename
```

To change how many bits to be grouped up, please edit `main.cpp`
```cpp
int main() {
    test<uint32_t>("32-bit", argv[1]); // Substitute uint32_t to desired length
}
```