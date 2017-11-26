#pragma once

#include <vector>

void do_x(const std::vector<uint8_t> & iv, const std::vector<uint8_t> & ki, std::vector<uint8_t> & ov);

void do_s(const std::vector<uint8_t> & iv, std::vector<uint8_t> & ov);
void do_l(const std::vector<uint8_t> & iv, std::vector<uint8_t> & ov); 
void do_r(const std::vector<uint8_t> & iv, std::vector<uint8_t> & ov);

void do_inv_s (const std::vector<uint8_t> & iv, std::vector<uint8_t> & ov);
void do_inv_l (const std::vector<uint8_t> & iv, std::vector<uint8_t> & ov);
void do_inv_r (const std::vector<uint8_t> & iv, std::vector<uint8_t> & ov);
