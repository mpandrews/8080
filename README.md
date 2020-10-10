# 8080 Emulator and Space Invaders

### Project Quickstart
- Clone this repository
- Within the project directory, create a build subdirectory. (i.e. Debug or Release) 
- Navigate into your subdirectory, and run either of the following:
    - `cmake -DCMAKE_BUILD_TYPE=Release ..`
    - `cmake -DCMAKE_BUILD_TYPE=Debug ..`
- Run `make`.  Once a build tree is established for a build type, any changes to the actual source files will be picked up by make.
- To run the emulator, run `8080 <ROM>` where ROM is the name of the rom file you want the emulator to execute.
- You can create a test ROM file like this: `echo -e -n \\x26\\x01\\x2e\\x01\\x36\\xff\\x46 > rom`
- Run `make test` to run unit tests. 

### Contributing Guidelines
- Branch from master and make a pull request when ready to review.
- Make sure to run `clang-format` to format the files.
- Make sure to add unit tests to your PR.
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
