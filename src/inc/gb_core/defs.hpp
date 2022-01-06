#pragma once

#define IMAGE_FORMAT_SIZE 3 //rgb

#define GB_S_W    160                          // screen width
#define GB_S_H    144                          // screen height
#define GB_S_P    (GB_S_W * GB_S_H)            // screen pixel count
#define GB_S_P_SZ (GB_S_P * IMAGE_FORMAT_SIZE) // screen pixel buffer size

#define TICKS_PER_FRAME 70224

enum gb_device_t {
    // Gameboy
    device_GB_cpu_dmg0,
    device_GB_cpu_dmg,
    device_GB = device_GB_cpu_dmg,

    //Gameboy Pocket
    device_MGB_cpu_mgb,
    device_MGB = device_MGB_cpu_mgb,

    //Super Gameboy
    device_SGB_cpu_sgb,
    device_SGB = device_SGB_cpu_sgb,

    //Super Gameboy 2
    device_SGB2_cpu_sgb2,
    device_SGB2 = device_SGB2_cpu_sgb2,

    // Gameboy Color
    device_GBC_cpu_cgb0,
    device_GBC_cpu_cgb,
    device_GBC_cpu_cgb_agb,
    device_GBC = device_GBC_cpu_cgb,
};

#define dev_is_GB(x) (          \
  (x) == device_GB_cpu_dmg0 ||  \
  (x) == device_GB_cpu_dmg ||   \
  (x) == device_MGB_cpu_mgb)

#define dev_is_SGB(x) (         \
  (x) == device_SGB_cpu_sgb ||  \
  (x) == device_SGB2_cpu_sgb2)

#define dev_is_GBC(x) (         \
  (x) == device_GBC_cpu_cgb0 || \
  (x) == device_GBC_cpu_cgb ||  \
  (x) == device_GBC_cpu_cgb_agb)
