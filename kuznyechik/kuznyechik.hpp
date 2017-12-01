#pragma once

#include <vector>

void do_x(const std::vector<uint8_t> & iv, const std::vector<uint8_t> & ki, std::vector<uint8_t> & ov);

void do_s(const std::vector<uint8_t> & iv, std::vector<uint8_t> & ov);
void do_l(const std::vector<uint8_t> & iv, std::vector<uint8_t> & ov); 
void do_r(const std::vector<uint8_t> & iv, std::vector<uint8_t> & ov);

void do_inv_s (const std::vector<uint8_t> & iv, std::vector<uint8_t> & ov);
void do_inv_l (const std::vector<uint8_t> & iv, std::vector<uint8_t> & ov);
void do_inv_r (const std::vector<uint8_t> & iv, std::vector<uint8_t> & ov);

void do_f (uint8_t idx, std::vector<uint8_t> & a1, std::vector<uint8_t> & a0);

void split_key (const std::vector<uint8_t> & key, std::vector<uint8_t> & k1, std::vector<uint8_t> & k2);
void kuz_set_key (const uint8_t* key);
void kuz_del_key (void);
std::vector< std::vector<uint8_t> >* kuz_export_keys (void);

int kuz_init (void);
void kuz_fin (void);

void kuz_encrypt_block (uint8_t* data);
void kuz_decrypt_block (uint8_t* data);
