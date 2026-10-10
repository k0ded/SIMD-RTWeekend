#pragma once

// Encodes 2D coordinates into a 32-bit Morton code using BMI2
inline uint32_t morton_encode_2d(uint16_t x, uint16_t y) {
    // 0x55555555 is 01010101... in binary (even bits)
    // 0xAAAAAAAA is 10101010... in binary (odd bits)
    uint32_t x_spread = _pdep_u32(x, 0x55555555);
    uint32_t y_spread = _pdep_u32(y, 0xAAAAAAAA);
    return x_spread | y_spread;
}

// Decodes a 32-bit Morton code back into 2D coordinates using BMI2
inline void morton_decode_2d(uint32_t code, uint16_t& x, uint16_t& y) {
    x = static_cast<uint16_t>(_pext_u32(code, 0x55555555));
    y = static_cast<uint16_t>(_pext_u32(code, 0xAAAAAAAA));
}