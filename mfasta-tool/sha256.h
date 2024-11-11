#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <bit>
#include <stdexcept>
#include <string>
#include <vector>

namespace refresh
{
    class SHA256 {
    public:
        using sha256_t = std::array<uint32_t, 8>;

        SHA256() 
        {
            reset();
        }

        void reset() 
        {
            data_len = 0;
            bit_len = 0;
            state = {
                0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
            };
        }

        template<typename T>
        void update(const T* ptr, size_t len) 
        {
            const uint8_t* data = reinterpret_cast<const uint8_t*>(ptr);
            update_data(data, len * sizeof(T));
        }

        template<typename Iter>
        void update(Iter first, Iter last) 
        {
            for (; first != last; ++first) 
            {
                const uint8_t* data = reinterpret_cast<const uint8_t*>(&(*first));
                update_data(data, sizeof(*first));
            }
        }

        void update(const std::string& s) 
        {
            update_data(reinterpret_cast<const uint8_t*>(s.data()), s.size());
        }

        template<typename T>
        void update(const std::vector<T>& v) 
        {
            update(v.begin(), v.end());
        }

        template<typename T, size_t SIZE>
        void update(const std::array<T, SIZE>& a) 
        {
            update(a.begin(), a.end());
        }

        void finalize() 
        {
            uint64_t total_bit_len = bit_len + data_len * 8;
            data_buffer[data_len++] = 0x80;
            
            if (data_len > 56) 
            {
                while (data_len < 64) 
                    data_buffer[data_len++] = 0;
                
                process_block();
                data_len = 0;
            }
        
            while (data_len < 56) 
                data_buffer[data_len++] = 0;
            
            for (int i = 7; i >= 0; --i) 
                data_buffer[data_len++] = (total_bit_len >> (i * 8)) & 0xff;

            process_block();
        }

        sha256_t& get_hash() 
        {
            return state;
        }

    private:
        uint8_t data_buffer[64] = { 0 };
        uint32_t data_len = 0;
        uint64_t bit_len = 0;
        sha256_t state;

        static constexpr uint32_t k[] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
            0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
            0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
            0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
            0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
            0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
            0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
            0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
            0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

        void process_block() 
        {
            uint32_t m[64];
            for (size_t i = 0; i < 16; ++i) 
            {
                m[i] = (data_buffer[i * 4] << 24) |
                    (data_buffer[i * 4 + 1] << 16) |
                    (data_buffer[i * 4 + 2] << 8) |
                    (data_buffer[i * 4 + 3]);
            }
            
            for (size_t i = 16; i < 64; ++i) 
            {
                uint32_t s0 = std::rotr(m[i - 15], 7) ^
                    std::rotr(m[i - 15], 18) ^
                    (m[i - 15] >> 3);
                uint32_t s1 = std::rotr(m[i - 2], 17) ^
                    std::rotr(m[i - 2], 19) ^
                    (m[i - 2] >> 10);
                m[i] = m[i - 16] + s0 + m[i - 7] + s1;
            }

            uint32_t a = state[0];
            uint32_t b = state[1];
            uint32_t c = state[2];
            uint32_t d = state[3];
            uint32_t e = state[4];
            uint32_t f = state[5];
            uint32_t g = state[6];
            uint32_t h = state[7];

            for (size_t i = 0; i < 64; ++i) 
            {
                uint32_t s1 = std::rotr(e, 6) ^ std::rotr(e, 11) ^ std::rotr(e, 25);
                uint32_t ch = (e & f) ^ (~e & g);
                uint32_t temp1 = h + s1 + ch + k[i] + m[i];
                uint32_t s0 = std::rotr(a, 2) ^ std::rotr(a, 13) ^ std::rotr(a, 22);
                uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                uint32_t temp2 = s0 + maj;

                h = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = b;
                b = a;
                a = temp1 + temp2;
            }

            state[0] += a;
            state[1] += b;
            state[2] += c;
            state[3] += d;
            state[4] += e;
            state[5] += f;
            state[6] += g;
            state[7] += h;
        }

        void update_data(const uint8_t* data, size_t len) 
        {
            for (size_t i = 0; i < len; ++i) 
            {
                data_buffer[data_len] = data[i];
                data_len++;

                if (data_len == 64) 
                {
                    process_block();
                    bit_len += 512;
                    data_len = 0;
                }
            }
        }
    };
}
