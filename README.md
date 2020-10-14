# 8080 Emulator and Space Invaders

### Project Quickstart
- Clone this repository
- Within the project directory, create a build subdirectory. (i.e. Debug or Release) 
- Navigate into your subdirectory, and run either of the following:
    - `cmake -DCMAKE_BUILD_TYPE=Release ..`
    - `cmake -DCMAKE_BUILD_TYPE=Debug ..`
- The Debug build target will add realtime disassembly and CPU state output on `stderr`.  
- Run `make`.  Once a build tree is established for a build type, any changes to the actual source files will be picked up by make.
- To run the emulator, run `8080 <ROM>` where ROM is the name of the rom file you want the emulator to execute.
- You can create a test ROM file like this: `echo -e -n \\x26\\x01\\x2e\\x01\\x36 > rom`
- Run `make test` to run unit tests.
### Speed Benchmarking and Speed Adjustment
To get CPU speed benchmarking output on `stderr`, define `BENCHMARK`:
- `make C_FLAGS="-DBENCHMARK"`

(Note that it may be `CFLAGS` rather than `C_FLAGS`, depending on your version of `make`.)

By default, CPU benchmark output will be printed every 2^23 cycles. (~8 million.)  You can override this by passing in a replacement value for `BENCH_INTERVAL`.
- `make C_FLAGS="-DBENCHMARK -DBENCH_INTERVAL=1000000"`

The above command would result in benchmark output every million cycles, which would be twice per second at 2 MHz.

By default, the CPU will be built with an emulated cycle time of 500ns, which equates to 2 MHz.  You can adjust this by passing in a replacement value for `CYCLE_TIME`
- `make C_FLAGS="-DCYCLE_TIME=250"`

The above command would result in a 4Mhz emulation.

Defines can, of course, be mixed:
- `make C_FLAGS="-DCYCLE_TIME=100 -DBENCHMARK -DBENCH_INTERVAL=2000000"`

Do note that (by far) the most expensive portion of the normal CPU loop is the timing portion.  Even with a `CYCLE_TIME` of zero, the timing logic will still be traversed.  The main use of the benchmarking is to gauge the accuracy of the emulated speed, not to maximize it.  We may add a true unthrottled mode later; experimentation has shown a ~4x speed increase which may have its uses.

Note also that if `VERBOSE` is set while benchmarking, the overhead of all the printing calls could well drive effective emulation speed below 2MHz on slower systems.

### Contributing Guidelines
- Branch from master and make a pull request when ready to review.
- Make sure to run `clang-format --style=file -i include/* src/* test/*`
- Tag reviewers and get approval from at least one reviewer.
- Squash and merge to master (one commit message per branch/PR)


### Reference materials and resources
This section contains some helpful reference materials and resources for the Intel-8080 processor and Space Invaders
* [Intel 8080 Wikipedia page](https://en.wikipedia.org/wiki/Intel_8080)
* [Intel 8080 Assembly Language Programming Manual](https://altairclone.com/downloads/manuals/8080%20Programmers%20Manual.pdf)
* [Intel 8080 Opcodes overview](http://www.piclist.com/techref/intel/8080.htm)
* [Intel 8080 Opcodes detailed](https://pastraiser.com/cpu/i8080/i8080_opcodes.html)
* [Architecture of the Intel 8080](https://www.elprocus.com/know-about-architecture-of-the-intel-8080-microprocessor/)
* [Space Invaders Wikipedia page](https://en.wikipedia.org/wiki/Space_Invaders)
* [Classic Gaming Space Invaders (play online and more)](http://www.classicgaming.cc/classics/space-invaders/)
* [Course materials for CS-450 Computer Graphics](https://web.engr.oregonstate.edu/~mjb/cs550/)
