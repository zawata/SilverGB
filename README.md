# SilverGB
Because it's not quite gold!

## Summary
SilverBoy is gameboy emulator I'm writing to apply the knowledge and concepts of my senior-year 
class regarding CPU execution piplines, and my general knowledge of embedded development.

I'm writing this code(and commenting it with what I learn) to hopefully interest other people who 
were like me about 6 years ago when I first tried to start this project(as a sophmore in highschool ha!): 
minor knowledge of programming but major interest in doing something cool.

The emulator features a GUI created with "Dear ImGui" which is an absolutely fantastic Gui Library.  
It utilizes SDL for media interfaces(Input, Audio, Window Managment) and OpenGL for graphics.  
It attempts to remain compilable on all Major Operating Systems(as I'm using SDL and OpenGL for OS interfacing)
but I'm developing and testing it on Linux so that is where it's guaranteed to work.  

## Progress:  
✔️ = Done  
➕ = In Progress  
🚫 = Not Started  

➕GUI  
&nbsp;&nbsp;&nbsp;&nbsp;✔️File Handling  
&nbsp;&nbsp;&nbsp;&nbsp;✔️Main Window  
&nbsp;&nbsp;&nbsp;&nbsp;➕Debug Windows  
&nbsp;&nbsp;&nbsp;&nbsp;➕Prettification  
➕CPU  
&nbsp;&nbsp;&nbsp;&nbsp;✔️OpCodes  
&nbsp;&nbsp;&nbsp;&nbsp;✔️Timers  
&nbsp;&nbsp;&nbsp;&nbsp;✔️Interrupt Handling  
&nbsp;&nbsp;&nbsp;&nbsp;➕IO Registers  
&nbsp;&nbsp;&nbsp;&nbsp;✔️Memory Layout  
➕Cartridge  
&nbsp;&nbsp;&nbsp;&nbsp;✔️Header Parsing  
&nbsp;&nbsp;&nbsp;&nbsp;🚫Memory Blocks  
➕Sound  
&nbsp;&nbsp;&nbsp;&nbsp;✔️SDL Audio Interface  
&nbsp;&nbsp;&nbsp;&nbsp;✔️Registers  
&nbsp;&nbsp;&nbsp;&nbsp;➕Channel 1  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕programmable timer  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕length counter  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕Volume Envelope  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫Frequency Sweep  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕Duty Cycle Generator  
&nbsp;&nbsp;&nbsp;&nbsp;➕Channel 2  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕programmable timer  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕length counter  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕Volume Envelope  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕Duty Cycle Generator  
&nbsp;&nbsp;&nbsp;&nbsp;🚫Channel 3  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕Wave RAM  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫programmable timer  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕length counter  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🚫Volume Shift  
&nbsp;&nbsp;&nbsp;&nbsp;✔️Channel 4  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️configurable timer  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;✔️LFSR PRNG  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕length counter  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;➕Volume Envelope  
🚫Video  
&nbsp;&nbsp;&nbsp;&nbsp;🚫Video Stuff  
🚫Input  
&nbsp;&nbsp;&nbsp;&nbsp;🚫Input Stuff  

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
