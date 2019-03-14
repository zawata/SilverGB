# SilverGB
Because it's not quite gold!

## Summary
SilverGB is gameboy emulator I'm writing to apply the knowledge and concepts of my senior-year 
class regarding CPU execution pipelines, and my general knowledge of embedded development.

I'm writing this code(and commenting it with what I learn) to hopefully interest other people who 
were like me about 6 years ago when I first tried to start this project(as a sophmore in highschool ha!): 
minor knowledge of programming but major interest in doing something cool.

The emulator features a GUI created with "Dear ImGui" which is an absolutely fantastic GUI Library.  
It utilizes SDL for media interfaces(Input, Audio, Window Managment) and OpenGL for graphics.  
It attempts to remain compilable on all Major Operating Systems(as I'm using SDL and OpenGL for OS interfacing)
but I'm developing and testing it on Linux so that is where it's "guaranteed" to work.(note the quotes...)  

## Progress:  
✔️ = Done  
➕ = In Progress  
🚫 = Not Started  

➕GUI  
&nbsp;&nbsp;&nbsp;&nbsp;✔️File Handling  
&nbsp;&nbsp;&nbsp;&nbsp;✔️Main Window  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️View Port  
&nbsp;&nbsp;&nbsp;&nbsp;➕Debug Windows  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️Debug Mode  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️CPU Registers Window  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️IO Registers Window  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫Disassembly Window  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫Stack Viewer Window  
&nbsp;&nbsp;&nbsp;&nbsp;➕Shortcuts  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️Keyboard Handling  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️Shortcut Processing  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫Dynamic Shortcut Assignment  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫Shortcut Configuration  
&nbsp;&nbsp;&nbsp;&nbsp;➕Prettification  
➕CPU  
&nbsp;&nbsp;&nbsp;&nbsp;✔️OpCodes  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️OpCode Disassembly  
&nbsp;&nbsp;&nbsp;&nbsp;✔️Timers  
&nbsp;&nbsp;&nbsp;&nbsp;✔️Interrupt Handling  
&nbsp;&nbsp;&nbsp;&nbsp;➕IO Registers  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️Core Registers  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕Input Registers  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️Sound Registers  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️Video Registers  
&nbsp;&nbsp;&nbsp;&nbsp;✔️Memory Layout  
➕Cartridge  
&nbsp;&nbsp;&nbsp;&nbsp;✔️Header Parsing  
&nbsp;&nbsp;&nbsp;&nbsp;➕Memory Bank Controllers  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️ROM(+RAM)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️MBC1  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕️MBC2  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕️MBC3  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫Others  
➕Sound  
&nbsp;&nbsp;&nbsp;&nbsp;➕(See the `sound_impl` branch)  
➕Video  
&nbsp;&nbsp;&nbsp;&nbsp;✔️VRAM  
&nbsp;&nbsp;&nbsp;&nbsp;➕Pixel Fifo  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️OAM Search  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕VRAM Process  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️Background  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫Window  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️Tile Fetching  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕️Sprite Fetching  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;️✔️HBLANK  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;️✔️VBLANK  
&nbsp;&nbsp;&nbsp;&nbsp;➕Display timing  
&nbsp;&nbsp;&nbsp;&nbsp;➕DMA  
🚫Input  
&nbsp;&nbsp;&nbsp;&nbsp;🚫Input Stuff  

## Features

Blargg Rom Tests:  
&nbsp;&nbsp;&nbsp;&nbsp;✔️ cpu_instrs  
&nbsp;&nbsp;&nbsp;&nbsp;✔️ halt_bug  

Gekkio's Acceptance Tests:  
&nbsp;&nbsp;&nbsp;&nbsp;- timer  
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
