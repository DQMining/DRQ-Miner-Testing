// Auto-generated from derohe/astrobwt/astrobwtv3/pow.go
// Expects: uint8_t* s (= step_3), uint8_t pos1, uint8_t pos2
// For case 253: prev_lhash, lhash (uint64_t)
// For cases 254/255: rc4 (RC4 cipher with init(s, 256))

switch (op) {
case 0:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = rotl8(s[i], 5);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = rotl8(s[i], s[i]);
        { uint8_t t1 = reverse8(s[pos1]); uint8_t t2 = reverse8(s[pos2]); s[pos2] = t1; s[pos1] = t2; }
    }
    break;
case 1:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = rotl8(s[i], 1);
        s[i] &= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 2:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = reverse8(s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
    }
    break;
case 3:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] = rotl8(s[i], 3);
        s[i] ^= s[pos2];
        s[i] = rotl8(s[i], 1);
    }
    break;
case 4:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] >>= (s[i] & 3);
        s[i] = rotl8(s[i], s[i]);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 5:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] >>= (s[i] & 3);
    }
    break;
case 6:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = rotl8(s[i], 3);
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 7:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = rotl8(s[i], s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = ~s[i];
    }
    break;
case 8:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] = rotl8(s[i], 5);
        s[i] = rotl8(s[i], 5);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 9:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] ^= rotl8(s[i], 4);
        s[i] >>= (s[i] & 3);
        s[i] ^= rotl8(s[i], 2);
    }
    break;
case 10:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = rotl8(s[i], 3);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 11:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] = rotl8(s[i], 5);
        s[i] &= s[pos2];
        s[i] = rotl8(s[i], s[i]);
    }
    break;
case 12:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] ^= rotl8(s[i], 2);
        s[i] = ~s[i];
    }
    break;
case 13:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] ^= s[pos2];
        s[i] >>= (s[i] & 3);
        s[i] = rotl8(s[i], 5);
    }
    break;
case 14:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 15:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] &= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 16:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = rotl8(s[i], 1);
        s[i] = ~s[i];
    }
    break;
case 17:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = rotl8(s[i], 5);
        s[i] = ~s[i];
    }
    break;
case 18:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], 3);
        s[i] = rotl8(s[i], 1);
        s[i] = rotl8(s[i], 5);
    }
    break;
case 19:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = rotl8(s[i], 5);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 20:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] ^= s[pos2];
        s[i] = reverse8(s[i]);
        s[i] ^= rotl8(s[i], 2);
    }
    break;
case 21:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] ^= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] &= s[pos2];
    }
    break;
case 22:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = reverse8(s[i]);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = rotl8(s[i], 1);
    }
    break;
case 23:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 3);
        s[i] = rotl8(s[i], 1);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] &= s[pos2];
    }
    break;
case 24:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] >>= (s[i] & 3);
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], 5);
    }
    break;
case 25:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = rotl8(s[i], 3);
        s[i] = rotl8(s[i], s[i]);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 26:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = reverse8(s[i]);
    }
    break;
case 27:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] &= s[pos2];
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], 5);
    }
    break;
case 28:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = rotl8(s[i], 5);
    }
    break;
case 29:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] ^= s[pos2];
        s[i] >>= (s[i] & 3);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 30:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], 5);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 31:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 32:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] = reverse8(s[i]);
        s[i] = rotl8(s[i], 3);
        s[i] ^= rotl8(s[i], 2);
    }
    break;
case 33:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] ^= rotl8(s[i], 4);
        s[i] = reverse8(s[i]);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 34:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 35:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = ~s[i];
        s[i] = rotl8(s[i], 1);
        s[i] ^= s[pos2];
    }
    break;
case 36:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = rotl8(s[i], 1);
        s[i] ^= rotl8(s[i], 2);
        s[i] = rotl8(s[i], 1);
    }
    break;
case 37:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] >>= (s[i] & 3);
        s[i] >>= (s[i] & 3);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 38:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] = rotl8(s[i], 3);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = rotl8(s[i], s[i]);
    }
    break;
case 39:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= s[pos2];
        s[i] >>= (s[i] & 3);
        s[i] &= s[pos2];
    }
    break;
case 40:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] ^= s[pos2];
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= s[pos2];
    }
    break;
case 41:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = rotl8(s[i], 3);
        s[i] ^= rotl8(s[i], 4);
    }
    break;
case 42:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] = rotl8(s[i], 3);
        s[i] ^= rotl8(s[i], 2);
        s[i] = rotl8(s[i], s[i]);
    }
    break;
case 43:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] &= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 44:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = rotl8(s[i], 3);
        s[i] = rotl8(s[i], s[i]);
    }
    break;
case 45:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] = rotl8(s[i], 5);
        s[i] &= s[pos2];
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
    }
    break;
case 46:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = rotl8(s[i], 5);
        s[i] ^= rotl8(s[i], 4);
    }
    break;
case 47:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] &= s[pos2];
        s[i] = rotl8(s[i], 5);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 48:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] = ~s[i];
        s[i] = ~s[i];
        s[i] = rotl8(s[i], 5);
    }
    break;
case 49:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = reverse8(s[i]);
        s[i] ^= rotl8(s[i], 4);
    }
    break;
case 50:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = reverse8(s[i]);
        s[i] = rotl8(s[i], 3);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = rotl8(s[i], 1);
    }
    break;
case 51:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] ^= rotl8(s[i], 4);
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], 5);
    }
    break;
case 52:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] >>= (s[i] & 3);
        s[i] = ~s[i];
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
    }
    break;
case 53:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= rotl8(s[i], 4);
        s[i] ^= rotl8(s[i], 4);
    }
    break;
case 54:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = reverse8(s[i]);
        s[i] ^= s[pos2];
        s[i] = ~s[i];
        s[i] = ~s[i];
    }
    break;
case 55:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = reverse8(s[i]);
        s[i] ^= rotl8(s[i], 4);
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], 1);
    }
    break;
case 56:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = ~s[i];
        s[i] = rotl8(s[i], 1);
    }
    break;
case 57:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] = rotl8(s[i], 5);
        s[i] = rotl8(s[i], 3);
        s[i] = reverse8(s[i]);
    }
    break;
case 58:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = reverse8(s[i]);
        s[i] ^= rotl8(s[i], 2);
        s[i] &= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 59:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = rotl8(s[i], s[i]);
        s[i] = ~s[i];
    }
    break;
case 60:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = rotl8(s[i], 3);
    }
    break;
case 61:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = rotl8(s[i], 3);
        s[i] = rotl8(s[i], 5);
    }
    break;
case 62:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] = ~s[i];
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 63:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 64:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] = reverse8(s[i]);
        s[i] ^= rotl8(s[i], 4);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 65:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] = rotl8(s[i], 3);
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 66:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] = reverse8(s[i]);
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], 1);
    }
    break;
case 67:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= rotl8(s[i], 2);
        s[i] = rotl8(s[i], 5);
    }
    break;
case 68:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] = ~s[i];
        s[i] ^= rotl8(s[i], 4);
        s[i] ^= s[pos2];
    }
    break;
case 69:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = reverse8(s[i]);
        s[i] >>= (s[i] & 3);
    }
    break;
case 70:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] >>= (s[i] & 3);
        s[i] ^= rotl8(s[i], 4);
    }
    break;
case 71:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 72:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = reverse8(s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 73:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = reverse8(s[i]);
        s[i] = rotl8(s[i], 5);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 74:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = rotl8(s[i], 3);
        s[i] = reverse8(s[i]);
        s[i] &= s[pos2];
    }
    break;
case 75:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] &= s[pos2];
        s[i] ^= rotl8(s[i], 4);
    }
    break;
case 76:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] ^= rotl8(s[i], 2);
        s[i] = rotl8(s[i], 5);
        s[i] >>= (s[i] & 3);
    }
    break;
case 77:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 3);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
    }
    break;
case 78:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] = reverse8(s[i]);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 79:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 80:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] &= s[pos2];
    }
    break;
case 81:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = rotl8(s[i], s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
    }
    break;
case 82:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] = ~s[i];
        s[i] = ~s[i];
        s[i] >>= (s[i] & 3);
    }
    break;
case 83:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = reverse8(s[i]);
        s[i] = rotl8(s[i], 3);
        s[i] = reverse8(s[i]);
    }
    break;
case 84:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = rotl8(s[i], 1);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 85:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] ^= s[pos2];
        s[i] = rotl8(s[i], s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 86:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], s[i]);
        s[i] ^= rotl8(s[i], 4);
        s[i] = ~s[i];
    }
    break;
case 87:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = rotl8(s[i], 3);
        s[i] ^= rotl8(s[i], 4);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 88:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] = rotl8(s[i], 1);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = ~s[i];
    }
    break;
case 89:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = ~s[i];
        s[i] ^= rotl8(s[i], 2);
    }
    break;
case 90:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = reverse8(s[i]);
        s[i] = rotl8(s[i], 5);
        s[i] = rotl8(s[i], 1);
        s[i] >>= (s[i] & 3);
    }
    break;
case 91:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] &= s[pos2];
        s[i] ^= rotl8(s[i], 4);
        s[i] = reverse8(s[i]);
    }
    break;
case 92:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = ~s[i];
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] &= s[pos2];
    }
    break;
case 93:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] &= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 94:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] = rotl8(s[i], s[i]);
        s[i] &= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 95:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] = ~s[i];
        s[i] = rotl8(s[i], 5);
        s[i] = rotl8(s[i], 5);
    }
    break;
case 96:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = rotl8(s[i], 1);
    }
    break;
case 97:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] >>= (s[i] & 3);
    }
    break;
case 98:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] >>= (s[i] & 3);
        s[i] ^= rotl8(s[i], 4);
    }
    break;
case 99:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = reverse8(s[i]);
        s[i] >>= (s[i] & 3);
    }
    break;
case 100:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = reverse8(s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
    }
    break;
case 101:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] >>= (s[i] & 3);
        s[i] = ~s[i];
    }
    break;
case 102:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 3);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = rotl8(s[i], 3);
    }
    break;
case 103:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] = reverse8(s[i]);
        s[i] ^= s[pos2];
        s[i] = rotl8(s[i], s[i]);
    }
    break;
case 104:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = reverse8(s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = rotl8(s[i], 5);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 105:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = rotl8(s[i], 3);
        s[i] = rotl8(s[i], s[i]);
        s[i] ^= rotl8(s[i], 2);
    }
    break;
case 106:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = reverse8(s[i]);
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], 1);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 107:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] ^= rotl8(s[i], 2);
        s[i] = rotl8(s[i], 5);
        s[i] = rotl8(s[i], 1);
    }
    break;
case 108:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] = ~s[i];
        s[i] &= s[pos2];
        s[i] ^= rotl8(s[i], 2);
    }
    break;
case 109:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = rotl8(s[i], s[i]);
        s[i] ^= s[pos2];
        s[i] ^= rotl8(s[i], 2);
    }
    break;
case 110:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= rotl8(s[i], 2);
        s[i] >>= (s[i] & 3);
    }
    break;
case 111:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = reverse8(s[i]);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] >>= (s[i] & 3);
    }
    break;
case 112:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 3);
        s[i] = ~s[i];
        s[i] = rotl8(s[i], 5);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 113:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] = rotl8(s[i], 1);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = ~s[i];
    }
    break;
case 114:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] = reverse8(s[i]);
        s[i] = rotl8(s[i], s[i]);
        s[i] = ~s[i];
    }
    break;
case 115:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] = rotl8(s[i], 5);
        s[i] &= s[pos2];
        s[i] = rotl8(s[i], 3);
    }
    break;
case 116:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] ^= s[pos2];
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 117:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = rotl8(s[i], 3);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] &= s[pos2];
    }
    break;
case 118:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = rotl8(s[i], 5);
    }
    break;
case 119:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = reverse8(s[i]);
        s[i] ^= rotl8(s[i], 2);
        s[i] = ~s[i];
        s[i] ^= s[pos2];
    }
    break;
case 120:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] ^= s[pos2];
        s[i] = reverse8(s[i]);
    }
    break;
case 121:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 122:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], s[i]);
        s[i] = rotl8(s[i], 5);
        s[i] ^= rotl8(s[i], 2);
    }
    break;
case 123:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] = ~s[i];
        s[i] = rotl8(s[i], 3);
        s[i] = rotl8(s[i], 3);
    }
    break;
case 124:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= s[pos2];
        s[i] = ~s[i];
    }
    break;
case 125:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = reverse8(s[i]);
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] >>= (s[i] & 3);
    }
    break;
case 126:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 3);
        s[i] = rotl8(s[i], 1);
        s[i] = rotl8(s[i], 5);
        s[i] = reverse8(s[i]);
    }
    break;
case 127:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] &= s[pos2];
        s[i] ^= s[pos2];
    }
    break;
case 128:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= rotl8(s[i], 2);
        s[i] = rotl8(s[i], 5);
    }
    break;
case 129:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] >>= (s[i] & 3);
    }
    break;
case 130:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] = rotl8(s[i], s[i]);
        s[i] = rotl8(s[i], 1);
        s[i] ^= rotl8(s[i], 4);
    }
    break;
case 131:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = rotl8(s[i], 1);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 132:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] = reverse8(s[i]);
        s[i] = rotl8(s[i], 5);
        s[i] ^= rotl8(s[i], 2);
    }
    break;
case 133:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] = rotl8(s[i], 5);
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 134:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], 1);
        s[i] &= s[pos2];
    }
    break;
case 135:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = reverse8(s[i]);
    }
    break;
case 136:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] ^= s[pos2];
        s[i] = rotl8(s[i], 5);
    }
    break;
case 137:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] >>= (s[i] & 3);
        s[i] = reverse8(s[i]);
        s[i] = rotl8(s[i], s[i]);
    }
    break;
case 138:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] ^= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 139:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] = rotl8(s[i], 3);
        s[i] ^= rotl8(s[i], 2);
        s[i] = rotl8(s[i], 3);
    }
    break;
case 140:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= s[pos2];
        s[i] = rotl8(s[i], 5);
    }
    break;
case 141:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 142:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] = rotl8(s[i], 5);
        s[i] = reverse8(s[i]);
        s[i] ^= rotl8(s[i], 2);
    }
    break;
case 143:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] = rotl8(s[i], 3);
        s[i] >>= (s[i] & 3);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 144:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = ~s[i];
        s[i] = rotl8(s[i], s[i]);
    }
    break;
case 145:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = reverse8(s[i]);
        s[i] ^= rotl8(s[i], 4);
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= rotl8(s[i], 4);
    }
    break;
case 146:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] &= s[pos2];
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
    }
    break;
case 147:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] ^= rotl8(s[i], 4);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 148:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] = rotl8(s[i], 5);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 149:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] = reverse8(s[i]);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 150:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] &= s[pos2];
    }
    break;
case 151:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 152:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] ^= rotl8(s[i], 2);
    }
    break;
case 153:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] = rotl8(s[i], 3);
        s[i] = ~s[i];
        s[i] = ~s[i];
    }
    break;
case 154:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] = ~s[i];
        s[i] ^= s[pos2];
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
    }
    break;
case 155:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] ^= s[pos2];
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= s[pos2];
    }
    break;
case 156:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] >>= (s[i] & 3);
        s[i] = rotl8(s[i], 3);
        s[i] = rotl8(s[i], 1);
    }
    break;
case 157:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = rotl8(s[i], s[i]);
        s[i] = rotl8(s[i], 1);
    }
    break;
case 158:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = rotl8(s[i], 3);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = rotl8(s[i], 1);
    }
    break;
case 159:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] ^= s[pos2];
        s[i] = rotl8(s[i], s[i]);
        s[i] ^= s[pos2];
    }
    break;
case 160:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] = reverse8(s[i]);
        s[i] = rotl8(s[i], 1);
        s[i] = rotl8(s[i], 3);
    }
    break;
case 161:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] ^= s[pos2];
        s[i] = rotl8(s[i], 5);
        s[i] = rotl8(s[i], s[i]);
    }
    break;
case 162:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = reverse8(s[i]);
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 163:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], 1);
    }
    break;
case 164:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = ~s[i];
    }
    break;
case 165:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] ^= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 166:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 3);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] ^= rotl8(s[i], 2);
        s[i] = ~s[i];
    }
    break;
case 167:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] >>= (s[i] & 3);
    }
    break;
case 168:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] &= s[pos2];
        s[i] = rotl8(s[i], s[i]);
        s[i] = rotl8(s[i], 1);
    }
    break;
case 169:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] ^= rotl8(s[i], 4);
        s[i] &= s[pos2];
    }
    break;
case 170:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = reverse8(s[i]);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 171:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 3);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = reverse8(s[i]);
    }
    break;
case 172:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = rotl8(s[i], 1);
    }
    break;
case 173:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 174:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] = rotl8(s[i], s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
    }
    break;
case 175:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 3);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = rotl8(s[i], 5);
    }
    break;
case 176:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] ^= s[pos2];
        s[i] = rotl8(s[i], 5);
    }
    break;
case 177:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= rotl8(s[i], 2);
        s[i] &= s[pos2];
    }
    break;
case 178:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = ~s[i];
        s[i] = rotl8(s[i], 1);
    }
    break;
case 179:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] >>= (s[i] & 3);
        s[i] = reverse8(s[i]);
    }
    break;
case 180:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] ^= rotl8(s[i], 4);
        s[i] ^= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 181:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] ^= rotl8(s[i], 2);
        s[i] = rotl8(s[i], 5);
    }
    break;
case 182:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] = rotl8(s[i], 1);
        s[i] = rotl8(s[i], 5);
        s[i] ^= rotl8(s[i], 4);
    }
    break;
case 183:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 184:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = rotl8(s[i], 5);
        s[i] ^= s[pos2];
    }
    break;
case 185:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], 5);
        s[i] >>= (s[i] & 3);
    }
    break;
case 186:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= rotl8(s[i], 4);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] >>= (s[i] & 3);
    }
    break;
case 187:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = rotl8(s[i], 3);
    }
    break;
case 188:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= rotl8(s[i], 4);
        s[i] ^= rotl8(s[i], 4);
    }
    break;
case 189:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] ^= rotl8(s[i], 4);
        s[i] ^= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 190:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] >>= (s[i] & 3);
        s[i] &= s[pos2];
        s[i] ^= rotl8(s[i], 2);
    }
    break;
case 191:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = rotl8(s[i], 3);
        s[i] = rotl8(s[i], s[i]);
        s[i] >>= (s[i] & 3);
    }
    break;
case 192:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 193:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = rotl8(s[i], s[i]);
        s[i] = rotl8(s[i], 1);
    }
    break;
case 194:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] = rotl8(s[i], s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] &= s[pos2];
    }
    break;
case 195:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= s[pos2];
        s[i] ^= rotl8(s[i], 4);
    }
    break;
case 196:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 3);
        s[i] = reverse8(s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = rotl8(s[i], 1);
    }
    break;
case 197:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], s[i]);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 198:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] >>= (s[i] & 3);
        s[i] = reverse8(s[i]);
        s[i] = rotl8(s[i], 1);
    }
    break;
case 199:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] ^= s[pos2];
    }
    break;
case 200:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = reverse8(s[i]);
        s[i] = reverse8(s[i]);
    }
    break;
case 201:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 3);
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= rotl8(s[i], 4);
        s[i] = ~s[i];
    }
    break;
case 202:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] = ~s[i];
        s[i] = rotl8(s[i], s[i]);
        s[i] = rotl8(s[i], 5);
    }
    break;
case 203:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] &= s[pos2];
        s[i] = rotl8(s[i], 1);
        s[i] = rotl8(s[i], s[i]);
    }
    break;
case 204:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] ^= rotl8(s[i], 2);
        s[i] = rotl8(s[i], s[i]);
        s[i] ^= s[pos2];
    }
    break;
case 205:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= rotl8(s[i], 4);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 206:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] = reverse8(s[i]);
        s[i] = reverse8(s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
    }
    break;
case 207:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] = rotl8(s[i], 3);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
    }
    break;
case 208:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] >>= (s[i] & 3);
        s[i] = rotl8(s[i], 3);
    }
    break;
case 209:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] = reverse8(s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 210:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] = rotl8(s[i], s[i]);
        s[i] = rotl8(s[i], 5);
        s[i] = ~s[i];
    }
    break;
case 211:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = rotl8(s[i], s[i]);
    }
    break;
case 212:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= s[pos2];
        s[i] ^= s[pos2];
    }
    break;
case 213:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = rotl8(s[i], 3);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 214:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] >>= (s[i] & 3);
        s[i] = ~s[i];
    }
    break;
case 215:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] &= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 216:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], s[i]);
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] &= s[pos2];
    }
    break;
case 217:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = rotl8(s[i], 1);
        s[i] ^= rotl8(s[i], 4);
    }
    break;
case 218:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = reverse8(s[i]);
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 219:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], 3);
        s[i] &= s[pos2];
        s[i] = reverse8(s[i]);
    }
    break;
case 220:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = reverse8(s[i]);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 221:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] ^= s[pos2];
        s[i] = ~s[i];
        s[i] = reverse8(s[i]);
    }
    break;
case 222:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] >>= (s[i] & 3);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] ^= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
    }
    break;
case 223:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 3);
        s[i] ^= s[pos2];
        s[i] = rotl8(s[i], s[i]);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 224:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] = rotl8(s[i], 1);
        s[i] = rotl8(s[i], 3);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 225:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] >>= (s[i] & 3);
        s[i] = reverse8(s[i]);
        s[i] = rotl8(s[i], 3);
    }
    break;
case 226:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = reverse8(s[i]);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] ^= s[pos2];
    }
    break;
case 227:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] &= s[pos2];
    }
    break;
case 228:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] >>= (s[i] & 3);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
    }
    break;
case 229:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 3);
        s[i] = rotl8(s[i], s[i]);
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
    }
    break;
case 230:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] &= s[pos2];
        s[i] = rotl8(s[i], s[i]);
        s[i] = rotl8(s[i], s[i]);
    }
    break;
case 231:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 3);
        s[i] >>= (s[i] & 3);
        s[i] ^= s[pos2];
        s[i] = reverse8(s[i]);
    }
    break;
case 232:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], 5);
    }
    break;
case 233:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 1);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = rotl8(s[i], 3);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
    }
    break;
case 234:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] >>= (s[i] & 3);
        s[i] ^= s[pos2];
    }
    break;
case 235:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] = rotl8(s[i], 3);
        s[i] = ~s[i];
    }
    break;
case 236:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] &= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 237:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
        s[i] ^= rotl8(s[i], 2);
        s[i] = rotl8(s[i], 3);
    }
    break;
case 238:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = rotl8(s[i], 3);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
    }
    break;
case 239:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] = rotl8(s[i], 1);
        s[i] = static_cast<uint8_t>(s[i] * s[i]);
        s[i] &= s[pos2];
    }
    break;
case 240:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] &= s[pos2];
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 241:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= rotl8(s[i], 4);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= s[pos2];
        s[i] = rotl8(s[i], 1);
    }
    break;
case 242:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] ^= s[pos2];
    }
    break;
case 243:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = rotl8(s[i], 1);
    }
    break;
case 244:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] ^= rotl8(s[i], 2);
        s[i] = reverse8(s[i]);
        s[i] = rotl8(s[i], 5);
    }
    break;
case 245:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] = rotl8(s[i], 5);
        s[i] ^= rotl8(s[i], 2);
        s[i] >>= (s[i] & 3);
    }
    break;
case 246:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] = rotl8(s[i], 1);
        s[i] >>= (s[i] & 3);
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
    }
    break;
case 247:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 5);
        s[i] ^= rotl8(s[i], 2);
        s[i] = rotl8(s[i], 5);
        s[i] = ~s[i];
    }
    break;
case 248:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = ~s[i];
        s[i] = static_cast<uint8_t>(s[i] - (s[i] ^ 97));
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = rotl8(s[i], 5);
    }
    break;
case 249:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = reverse8(s[i]);
        s[i] ^= rotl8(s[i], 4);
        s[i] ^= rotl8(s[i], 4);
        s[i] = rotl8(s[i], s[i]);
    }
    break;
case 250:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] &= s[pos2];
        s[i] = rotl8(s[i], s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] ^= rotl8(s[i], 4);
    }
    break;
case 251:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = static_cast<uint8_t>(s[i] + s[i]);
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = reverse8(s[i]);
        s[i] ^= rotl8(s[i], 2);
    }
    break;
case 252:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = reverse8(s[i]);
        s[i] ^= rotl8(s[i], 4);
        s[i] ^= rotl8(s[i], 2);
        s[i] = static_cast<uint8_t>(s[i] << (s[i] & 3));
    }
    break;
case 253:
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] = rotl8(s[i], 3);
        s[i] ^= rotl8(s[i], 2);
        s[i] ^= s[pos2];
        s[i] = rotl8(s[i], 3);
        prev_lhash = lhash + prev_lhash;
        lhash = xxhash64(s, pos2);
    }
    break;
case 254:
case 255:
    rc4.init(s, 256);
    for (uint8_t i = pos1; i < pos2; i++) {
        s[i] ^= static_cast<uint8_t>(popcount8(s[i]));
        s[i] = rotl8(s[i], 3);
        s[i] ^= rotl8(s[i], 2);
        s[i] = rotl8(s[i], 3);
    }
    break;
default:
    break;
}
