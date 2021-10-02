# SilverGB
Because it's not quite gold!

## Summary
SilverGB is gameboy emulator I'm writing to apply the knowledge and concepts of my senior-year class regarding CPU execution pipelines, and my general knowledge of embedded development.  

I'm writing this code(and commenting it with what I learn) to hopefully interest other people who were like me when I first tried to start this project(as a sophmore in highschool ha!): minor knowledge of programming but major interest in doing something cool.  

The emulator attempts to remain compilable on all Major Operating Systems(as I'm using SDL and OpenGL for OS interfacing).  

## Major TODOs

Slight TODOs are littered throughout the code, the bigger ones are placed here:  
 - M-Cycle accurate CPU rewrite(see `src/gb/new_cpu.c`)
   - Basic structure is done and designed, just need to actually implement all 499 instructions
 - Accurate PPU timing
   - Most of this isn't centrally documented but spread across 3-4 documents and dozens of test-roms. So yay.


## Progress:  
✔ = Done  
➕ = In Progress  
🚫 = Not Working  
\- = Not Tested  

✔ CPU  
➕ Cartridge  
&nbsp;&nbsp;&nbsp;&nbsp;✔ Header Parsing  
&nbsp;&nbsp;&nbsp;&nbsp;➕ Memory Bank Controllers  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ ROM(+RAM)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ MBC1  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕️ MBC2  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ MBC3  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕ RTC  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 Others  
➕ Sound  
&nbsp;&nbsp;&nbsp;&nbsp;➕ APU  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕ Square Channel 1  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 Frequency Sweep  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕️ Volume Envelope  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ Frequency Timer  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕ Square Channel 2  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕️ Volume Envelope  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ Frequency Timer  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 Wave Channel  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕ Noise Channel  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕️ Volume Envelope  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕️ Configurable Timer  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕️ LFSR  
&nbsp;&nbsp;&nbsp;&nbsp;✔ Async Audio Playback  
&nbsp;&nbsp;&nbsp;&nbsp;➕ Mixing?  
&nbsp;&nbsp;&nbsp;&nbsp;🚫 Volume Control  
➕ PPU  
&nbsp;&nbsp;&nbsp;&nbsp;✔ VRAM  
&nbsp;&nbsp;&nbsp;&nbsp;✔ Pixel Fifo  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ OAM Search  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ VRAM Process  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ Background  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ Window  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ Tile Fetching  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ Sprite Fetching  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;️✔ HBLANK  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;️✔ VBLANK  
&nbsp;&nbsp;&nbsp;&nbsp;➕ Display timing  
&nbsp;&nbsp;&nbsp;&nbsp;✔ DMA  
➕ Input  
&nbsp;&nbsp;&nbsp;&nbsp;✔ Input  
&nbsp;&nbsp;&nbsp;&nbsp;🚫 Interrupts  


## Features

Blargg Rom Tests:  
&nbsp;&nbsp;&nbsp;&nbsp;✔ cpu_instrs  
&nbsp;&nbsp;&nbsp;&nbsp;✔ halt_bug  

Gekkio's Acceptance Tests:  
&nbsp;&nbsp;&nbsp;&nbsp;➕ bits  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ mem_oam.gb  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ reg_f.gb  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 unused_hwio-GS.gb  
&nbsp;&nbsp;&nbsp;&nbsp;✔ instr  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ daa.gb  
&nbsp;&nbsp;&nbsp;&nbsp;🚫 interrupts  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 ie_push.gb  
&nbsp;&nbsp;&nbsp;&nbsp;➕ oam_dma  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ basic.gb  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 reg_read.gb  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 sources-dmgABCmgbS.gb  
&nbsp;&nbsp;&nbsp;&nbsp;➕ ppu  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 hblank_ly_scx_timing-GS.gb  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ intr_1_2_timing-GS.gb  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ intr_2_0_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕ intr_2_mode0_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 intr_2_mode0_timing_sprites.gb  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕ intr_2_mode3_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕ intr_2_oam_ok_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 lcdon_timing-dmgABCmgbS.gb  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 lcdon_write_timing-GS.gb  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 stat_irq_blocking.gb  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ stat_lyc_onoff.gb  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 vblank_stat_intr-GS.gb  
&nbsp;&nbsp;&nbsp;&nbsp;➕ timer  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ div_write  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 rapid_toggle  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ tim00  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ tim00_div_trigger  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ tim01  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ tim01_div_trigger  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ tim10  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ tim10_div_trigger  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ tim11  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔ tim11_div_trigger  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 tima_reload  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 tima_write_reloading  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫 tma_write_reloading  
&nbsp;&nbsp;&nbsp;&nbsp;- add_sp_e_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- boot_div2-S.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- boot_div-dmg0.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- boot_div-dmgABCmgb.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- boot_div-S.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- boot_hwio-dmg0.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- boot_hwio-dmgABCmgb.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- boot_hwio-S.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- boot_regs-dmg0.gb  
&nbsp;&nbsp;&nbsp;&nbsp;✔ boot_regs-dmgABC.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- boot_regs-mgb.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- boot_regs-sgb2.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- boot_regs-sgb.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- call_cc_timing2.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- call_cc_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- call_timing2.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- call_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- di_timing-GS.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- div_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;✔ ei_sequence.gb  
&nbsp;&nbsp;&nbsp;&nbsp;✔ ei_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;✔ halt_ime0_ei.gb  
&nbsp;&nbsp;&nbsp;&nbsp;🚫 halt_ime0_nointr_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- halt_ime1_timing2-GS.gb  
&nbsp;&nbsp;&nbsp;&nbsp;✔ halt_ime1_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;✔ if_ie_registers.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- intr_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- jp_cc_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- jp_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- ld_hl_sp_e_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;🚫 oam_dma_restart.gb  
&nbsp;&nbsp;&nbsp;&nbsp;🚫 oam_dma_start.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- oam_dma_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- pop_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- push_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- rapid_di_ei.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- ret_cc_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- reti_intr_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- reti_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- ret_timing.gb  
&nbsp;&nbsp;&nbsp;&nbsp;- rst_timing.gb  


## Compilation

Cmake is used for the build system.  
Vcpkg is used as a package manager where possible(make sure to `git submodule update --init --recursive`).  
`clang` is the preferred compiler(as it's my favorite) but `msvc` and `gcc` should work as well  
ninja is recommended as a generator but any generator will do.
### Windows

Install the following packages in vcpkg

```bash
./vcpkg install @../.vcpkg_deps.txt
./vcpkg install magnum[wglContext] --recurse

```
Note that this will default to x86-32. for 64bit pass `--triplet=x64-windows`  

Visual Studio  2019+ is probably the easiest solution as iut contains built in support for cmake.  


### Linux

Install the following packages in vcpkg

```bash
./vcpkg install @../.vcpkg_deps.txt
./vcpkg install magnum[GlxContext] --recurse

```

```bash
mkdir -p out/linux
cd out/linux

#Ninja
cmake ../.. -GNinja
cmake --build .

#Make
cmake ../..
make
```

## Questions
### Who?
Me

### What?
I'm writing a gameboy emulator.

### When?
In my very limited freetime.

### Where?
Here.

### Why?
Because I've always wanted too.

### How?
Mostly the Gameboy Programming Manual, and the gbdev wiki.
