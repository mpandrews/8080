# ROMs and Read-Only Masks

## ROM Files
At its core, a 'ROM' file is just raw binary data which will be loaded into the emulator's memory space.

Many emulators (notably MAME,) use a separate file for each memory chip (typically 1KB or 2KB) in the specific system being emulated, and hardcode in the offsets at which each file is to be loaded.  We do not take that approach; for our emulator, each ROM is a single file.  If the file represents multiple chips with RAM between them, that RAM should be zero-bytes in the ROM file.  Rephrased, each byte of a ROM should be at the same offset within the ROM file as you would like it to be in emulated memory.

Similarly, execution in our emulator will always start at address zero.  If a program expects to be at `0x100`, then the ROM should be padded with zeros and a jump to `0x100` placed at the beginning.  (Though leaving out the jump will work fine, since `0x00` is `NOP`.)

This allows for a certain amount of flexibility in adding new ROMs: the emulator does not need to know anything at all about them.

Emulation within our system always happens in 64KB of emulated memory: ROM files do not need to account for every byte.  The ROM file will be loaded begining at address zero, and if the file is less than 64KB the balance of memory will simply be zeroed out.  (On a real system, memory would be in an undefined state.)  If the ROM file is more than 64KB, the excess will be ignored.

## ROM Masking

By default, all emulated memory will be treated as read-write.  This means that if you simply add a new ROM and run it, it will actually be stored in RAM.  Sometimes this is desirable: if you wish to emulate a home computer, rather than an arcade machine, program memory would likely be mutable.

However, there are a number of circumstances where write-masking the program area of the emulator is necessary.  Ozma Wars, for example, will routinely clobber its own program if allowed to.  (One can temporarily rename its mask file to see what this looks like in practice.)

As such, we provide a mechanism for preventing writes to memory regions, the `.mask` file.  Whenever a ROM file is opened, the emulator will check to see if the same directory contains an accompanying file with the same name, but `.mask` appended.  If one is found, that mask file is used to generate a mask which will be used to filter attempted writes to emulated memory.

Note that `.mask` files are binary data, not string data.

In its basic operation, the mask divides (shifts, actually) the address of each memory write by some constant, and then uses that to index into an array of values to see whether the write is permissible.

### The Sizing Byte

The first byte of the mask file indicates how large each masking region will be.  You can think of these masking regions as representing memory chips, or as being akin to pages.  The valid possibilities are in the range `[0x0,0x10]`.

Each time a write is attempted, the address to be written will be right-shifted by this value.  Thus, if the value is `0xA`, the 16-bit memory address will be right shifted by 10 bits, resulting in a value in the range `[0,64)`.  (Note that right-shifting by 10 can also be though of as dividing by `2^10`, which is 1,024.)  Any address in the first kilobyte of memory will be `0` after shifting, any address in the second kilobyte will be `1` after shifting, and so on.

This value is then used to index into an array, and the value found will determine whether the write will be allowed to occur.

At the extremes, a sizing byte of `0x0` will result in no shift at all, and a masking array of the same size (64 kilobytes) as the emulated memory itself.  Each and every byte of memory, therefore, will have its own write-protection status.  A sizing byte of `0x10`, on the other hand, will result in a masking array with a single index, and all of memory will be either read-write or read-only.

More useful are values in between: `0xA` will provide a page size of 1KB, `0xB` will provide 2KB, and `0x9` will provide 512 bytes.

The exact formula for page size is two raised to the power of the sizing byte.

### The Masking Array

Once the sizing byte has been read, the emulator will allocate an array of the size indicated.  Returning again to our example of `0xA` as the sizing byte, this would result in a 64 entry array.  The formula is `2^(16 - shift)` or `2^16 / 2^shift` where `shift` is the value of the sizing byte.

Each subsequent byte of the mask file will be used to set the permissions for a page.  The second byte of the file will set index 0 of the array, the third byte will set index 1 of the array, and so on.  A zero value will result in read-write permission for that page, a non-zero value will result in read-only permissions.

The mask need not provide a value for every index: if the file only provides values for part of memory, the balance will be made read-write.

### An Example

`ozma`, the rom for Ozma Wars, is accompanied by `ozma.mask`.  The contents of `ozma.mask` are:

`0B 01 01 01 01 00 00 00 00 01 01`

Note that `.mask` files are binary data, not string data: `ozma.mask` is 11 bytes in size.

`2^0xb` is 2,048, so our page size is 2 kilobytes.  This means we will have a masking array of 32 entries, each controlling permissions for a 2KB region of memory.

The remainder of the file specifies the permissions of individual pages: the first four are marked read-only, then the next four are marked read-write, then two more are marked read-only.  Thus, memory addresses `[0x00, 0x2000)` and `[0x4000, 0x5000)` are read-only, while the rest of memory is read-write.

You may notice that Ozma Wars' mask could be made more succinct:

`0C 01 01 00 00 01`

The only reason this was not done is that the real Ozma Wars uses 2 kilobyte memory chips for both ROM and RAM, and using that page size makes the file a little more understandable to human readers familiar with the game.  The two files will have exactly the same effect on emulation, though of course using `0xb` represents a tragic, and regrettable, waste of 16 bytes on the free store.

Ozma Wars also demonstrates why a paginated masking behavior is necessary, as opposed to simply forbidding writes to any address lower than the last program byte: Ozma's stack space lives in the gap between the two ROM regions.  If `[0x2000, 0x4000)` were not writable, the game could not run.

### Default Behavior

If no `.mask` file is provided, the emulator will simply use a shift value of `0x10` and a single-element array containing zero.  Thus all of memory will be treated as a single writeable page.

### Multi-Byte Writes

Within the emulation, all multi-byte writes are treated as two single-byte writes.  While this does hurt efficiency slightly, it allows us to handle cases where one byte of a write might be in RAM and the other byte in ROM.  It also allows a single write to wrap around the memory space.  Thus writing two bytes to `0xffff` will write the low byte to `0xffff` and the high byte to `0x0000`, assuming that both of these bytes are permissible.  If either region is read-only, only that write is affected.

### Behavior on Forbidden Writes

If a write is blocked, the memory is simply left unchanged.  Emulation will otherwise continue as normal.  If `VERBOSE` is defined, as in a debug build, a diagnostic message will be printed.