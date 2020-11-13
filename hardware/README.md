# Hardware Libraries and You (A Primer)

The core 8080 emulation is just that: emulation of an 8080.  By default, it doesn't know anything about _any_ external hardware.  It has no way to take inputs, or provide outputs, or interact meaningfully with anything outside of itself.  While this is sufficient for short programs where you can extract useful information from the disassembly, it quickly becomes useless when trying to run more substantive programs.  (Like Space Invaders.)

For that reason, there are several functions in the emulation for which you can substitute new definitions by providing a shared library.  By defining these, you can simulate whatever external devices you wish, with considerable flexibility as to means.

## Library Structure

To create a library, make a new subdirectory in the `hardware` subdirectory of the project root.  This will be the root of your library.  Within that new library root, create a `CMakeLists.txt` file, a `src` subdirectory, and an `include` subdirectory.  As a starting point, you may wish to copy the `none` directory into a new directory, and change the references to `none` in its `CMakeLists.txt` to match the new name.

Your new library will be automatically picked up and compiled by the project-level CMake, and the resulting library file deposited in the `hw` subdirectory of the build directory.

Should you wish to write a brand-new `CMakeLists.txt`, you should create a new `MODULE` target; this ensures that CMake produces a library suitable for loading at runtime (as opposed to via linking at link time.)

Similarly, if you add the variable `${INCLUDE_DIR}` to the library target's inlude directories, you will have access to the header files defined in the main project.  This is essentially mandatory, since the functions you will need to define will take arguments in the form of pointers-to-`struct`s which are defined in those headers.

A prototypical `CMakeLists.txt` would be as follows:

```
add_library(myLib MODULE)                                                         
                                                                                 
file(GLOB srcs                                                                   
        CONFIGURE_DEPENDS                                                        
        ${CMAKE_CURRENT_LIST_DIR}/src/*                                          
)                                                                                
                                                                                 
target_sources(myLib PRIVATE ${srcs})                                             
target_include_directories(myLib PRIVATE                                                                                                                        
        ${INCLUDE_DIR}                                                           
        ${CMAKE_CURRENT_LIST_DIR}/include                                        
        )                                                                        
target_compile_options(myLib PRIVATE ${FlagSettings})                             
```

This would result in a library composed of all the source files in your `src` subdirectory, with the ability to see your `include` subdirectory and the `include` subdirectory of the project root.  It would also have access to all the compile flags defined for the project, notably the `VERBOSE` flag used for controlling debug output.

## What You Actually Have to Define

Hardware libraries in the emulator _must_ provide certain functions, which must have specific names and specific function signatures.  Note that these _must_ be callable from C: while you can certainly write them in C++, or indeed whatever language you feel like, you must provide functions that are callable from C with these specific names.  In particular, consider that C++ mangles function names to facilitate overloading: as a result, to use C++ you must preface the function declaration with `extern "C"`.

The mandatory functions are as follows:
- `int hw_in(const uint8_t* opcode, struct cpu_state* cpu)`
- `int hw_out(const uint8_t* opcode, struct cpu_state* cpu)`
- `int hw_interrupt_hook(const uint8_t* opcode, struct cpu_state* cpu, int (*op_func)(const uint8_t*, struct cpu_state*))`
- `void* hw_init_struct()`
- `void hw_destroy_struct(void* hw_struct)`

We will look at each of these functions, and what they should or must provide, in turn.

### `hw_init_struct` and `hw_destroy_struct`

Most non-trivial hardware libraries will need to store state, whether that's keyboard information, graphics bitmaps, or anything else.

Whatever data you would like the CPU state to carry around, allocate and intialize it here.  Whatever pointer you return in `hw_init_struct` will be assigned to the `void* hardware_struct` member of the `struct cpu_state` used for keeping emulator state.  All functions which have access to that `struct cpu_state` (which includes all HW lib functions other than `hw_init_struct` itself) will be able to cast that `void*` as needed to access the data.

`hw_init_struct` is passed in as an argument a `struct system_resources` (defined in `cpu.h`,) which will contain pointers to all CPU resources which might conceivably need to be shared outside the CPU.  (Mutexes, the interrupt buffer, etc.)  Any resource you wish to use in your front end (if you have a front end) should be assigned to a pointer in this struct.

Conversely, `hw_destroy_struct` will receive a pointer to the struct returned by `hw_init_struct` as its argument during cleanup, when the program is exiting.  Anything you need to free or otherwise de-initialize should be cleaned up here.

One thing to bear in mind when setting up your data structure is thread-safety: if you define a front-end, it will run in a different thread than the core CPU emulation, while `hw_in` and `hw_out` and `hw_interrupt_hook` run in the CPU emulation thread, so your data structure may require mutexes or other synchronization primitives to protect itself.

### `hw_in` and `hw_out`

These two functions, once loaded by the emulator, will be used as the actual function definitions for the `IN` and `OUT` opcodes.  As such, they take the same argument structure as all other opcodes: a pointer to an array containing the opcode itself and its immediate argument(s), and a pointer to the CPU state struct.  Note that this struct will itself contain a `void*` pointer to your library data `struct`, allowing you access to whatever library-specific data you might need to play around with.

In the specific case of these opcodes, the opcode array will have a one-byte immediate value in index 1: the port to be written to or read from.  You can use `switch(opcode[1])` to conveniently provide proper code for each port you actually want to use.  `IN` and `OUT` read/write to/from the `A` register.

Once again, looking at the default `none` library is instructive:

```
int hw_in(const uint8_t* opcode, struct cpu_state* cpu)                          
{                                                                                
        assert(opcode[0] == 0b11011011;                                         
        (void) cpu;                                                              
        (void) opcode;                                                           
#ifdef VERBOSE                                                                   
        fprintf(stderr, "IN (Hardware: none)\n");                                
#endif                                                                                                                                                         
        return 10;                                                               
}
```
This is a minimum baseline for how these opcodes should behave: both should return `10`, and the convention for debug printing should be adhered to.  Beyond that, you can really do whatever you need to, just be aware that these functions will be called from within the CPU routine, which will have implications for inter-thread consistency if they touch data which your front-end thread (should one exist) also touches.

### `hw_interrupt_hook`

The function signature for `hw_interrupt_hook` is: 
```
int hw_interrupt_hook(const uint8_t* opcode, 
	struct cpu_state* cpu, 
	int (*op_func)(const uint8_t*, struct cpu_state*))
```

The third argument is the function signature of an opcode function: when an interrupt is caught by the CPU emulation, `hw_interrupt_hook` will be called and passed both the opcode array and the cpu state as with any opcode call, but it will also be passed a copy of the function associated with that opcode.  This might seem peculiar, but consider that the main `8080` program does not export symbols, nor does it make any particular assumptions about what the hardware library will wish to do when an interrupt is caught.

By passing in everything needed to run an opcode _and_ the opcode function to be run, the `hw_interrupt_hook` function can do whatever it needs to before, after, or _instead_ of actually executing the opcode.  

The minimal interrupt hook function body would simply be:

`return op_func(opcode, cpu);`

(Assuming that you choose the same parameter names.)

Note that we return the opcode function's own return value; this is used by the CPU loop for timekeeping: the return value of an opcode is its cost in cycles.

This will execute the interrupt without any hardware specific functionality.  Anything else you wish do have occur on receipt of an interrupt (whether every interrupt or some particular subset of opcodes) can be added to this function.  In the `taito` library, for example, this function updates the front-end video buffer when the CPU receives `RST 1` or `RST 2`.

## What You May Optionally Define

### `front_end`

If you wish your program to have any kind of graphical front-end, or really interact with the user in any way other than via the terminal, you will probably want to define `front_end`.  `front_end`'s signature is `void* front_end(void*)`.  If such a function is defined, it will be run in a separate `pthread` immediately after CPU emulation begins.  Its argument will be the hardware struct you initialized in `hw_struct_init`, so whatever data structures you have established will be available.

A few caveats: because the front end is a separate thread, be mindful of thread safety with your data.  Bear in mind that the front end is _not_ passed a copy of the CPU state struct.  While you can add pointers to resources intended for sharing, such as the interrupt buffer and its associated mutex/pthread conditional, it is not intended that you should have direct access to CPU registers or other internal state.  (The emulation's memory space is not intended for direct access from the front end, due to the overhead of false sharing, but you can and may need to set up a buffer to copy arbitrarily large sections of memory at appropriate junctures.)

Also, because the main executable does not export symbols, you cannot call any function defined there.

Beyond that, you may do whatever you like.

### Any other function you want.

While the above functions are the only ones that will be called by the main `8080` executable, you can of course define other functions for _these_ functions to call.

## Nested Libraries

It is possible to split these definitions across more than one library, or to make one central library which several more specific libraries will rely on.  For example, our `si` library relies on the a more generalized `taito` library: this allows us to use the `taito` library for a variety of games built on the same general cabinet architecture, while still varying the control schemes and other game-specific information.  We do this by linking the `si` library against `taito` at link time: because library dependencies are transitive, when `si` is opened, `taito` will also be loaded into memory, and function definitions in either library can be accessed through the same `dl_open()` handle.  This also works between the libraries: because `si` exports its symbols, `taito` can use functions defined there, as long as they are declared as weak symbols in its own header files.  This interdependency essentially creates another API for definining `taito`-based games.

As long as all the required function definitions are available after opening your library, you may organize them however you wish.

## Using your New Library

Once you have created a new subdirectory and added the appropriate `CMakeLists.txt`, the project CMake will simply pick your library up and add it to the `hw` subdirectory of the build directory.  You can then use it by simply using the `--hw myLib` switch when invoking `8080`.