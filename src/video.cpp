#include "video.hpp"

Video_Controller::Video_Controller() {}
Video_Controller::~Video_Controller() {}

u8 Video_Controller::read_reg(u8 loc) {}
void Video_Controller::write_reg(u8 loc, u8 data) {}

u8 Video_Controller::read_ram(u8 loc) {}
void Video_Controller::write_ram(u8 loc, u8 data) {}

u8 Video_Controller::read_oam(u8 loc) {}
void Video_Controller::write_oam(u8 loc, u8 data) {}