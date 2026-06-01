package main

import (
	"encoding/hex"
	"fmt"
	"math/bits"

	"github.com/cespare/xxhash"
	"github.com/dchest/siphash"
	sha256 "github.com/minio/sha256-simd"
	"github.com/segmentio/fasthash/fnv1a"
	"golang.org/x/crypto/salsa20/salsa"

	astrobwt "github.com/deroproject/derohe/astrobwt/astrobwtv3"
)

func main() {
	input := []byte{0x01, 0x02, 0x03, 0x04}

	var step_3 [256]byte
	var counter [16]byte

	sha_key := sha256.Sum256(input)
	salsa.XORKeyStream(step_3[:], step_3[:], &counter, &sha_key)

	rc4s := astrobwt.NewCipher(step_3[:])
	rc4s.XORKeyStream(step_3[:], step_3[:])

	lhash := fnv1a.HashBytes64(step_3[:])
	prev_lhash := lhash

	tries := uint64(0)
	s := step_3[:]

	for {
		tries++
		random_switcher := prev_lhash ^ lhash ^ tries
		op := byte(random_switcher)
		pos1 := byte(random_switcher >> 8)
		pos2 := byte(random_switcher >> 16)

		if pos1 > pos2 {
			pos1, pos2 = pos2, pos1
		}
		if pos2-pos1 > 32 {
			pos2 = pos1 + (pos2-pos1)&0x1f
		}

		if tries <= 5 || (tries >= 258 && tries <= 270) {
			fmt.Printf("Go Iter %d: op=%d pos1=%d pos2=%d lhash=%016x prev=%016x\n", tries, op, pos1, pos2, lhash, prev_lhash)
			fmt.Printf("  s[0:16]=%s s[255]=%02x\n", hex.EncodeToString(s[:16]), s[255])
		}

		_ = s[pos1:pos2]

		// Full 256-case switch from pow.go
		switch op {
		case 0:
			for i := pos1; i < pos2; i++ {
				s[i] = s[i] ^ byte(bits.OnesCount8(s[i]))
				s[i] = bits.RotateLeft8(s[i], 5)
				s[i] *= s[i]
				s[i] = bits.RotateLeft8(s[i], int(s[i]))
				s[pos2], s[pos1] = bits.Reverse8(s[pos1]), bits.Reverse8(s[pos2])
			}
		case 1:
			for i := pos1; i < pos2; i++ {
				s[i] = s[i] << (s[i] & 3)
				s[i] = bits.RotateLeft8(s[i], 1)
				s[i] = s[i] & s[pos2]
				s[i] += s[i]
			}
		case 2:
			for i := pos1; i < pos2; i++ {
				s[i] = s[i] ^ byte(bits.OnesCount8(s[i]))
				s[i] = bits.Reverse8(s[i])
				s[i] = s[i] << (s[i] & 3)
				s[i] = s[i] ^ byte(bits.OnesCount8(s[i]))
			}
		// ... we need all 256 cases. Let me just call the real function instead.
		default:
			// Can't easily replicate all 256 cases here
		}

		if tries <= 5 || (tries >= 258 && tries <= 270) {
			fmt.Printf("  after ops s[0:16]=%s\n", hex.EncodeToString(s[:16]))
		}

		if s[pos1]-s[pos2] < 0x10 {
			prev_lhash = lhash + prev_lhash
			lhash = xxhash.Sum64(s[:pos2])
		}
		if s[pos1]-s[pos2] < 0x20 {
			prev_lhash = lhash + prev_lhash
			lhash = fnv1a.HashBytes64(s[:pos2])
		}
		if s[pos1]-s[pos2] < 0x30 {
			prev_lhash = lhash + prev_lhash
			lhash = siphash.Hash(tries, prev_lhash, s[:pos2])
		}
		if s[pos1]-s[pos2] <= 0x40 {
			rc4s = astrobwt.NewCipher(s[:])
			rc4s.XORKeyStream(s[:], s[:])
		}

		s[255] = s[255] ^ s[pos1] ^ s[pos2]

		if tries > 260+16 || (s[255] >= 0xf0 && tries > 260) {
			fmt.Printf("Loop ended at tries=%d s[255]=%02x\n", tries, s[255])
			fmt.Printf("data_len calc: tries=%d s[253]=%02x s[254]=%02x\n", tries, s[253], s[254])
			break
		}
	}

	hash := astrobwt.AstroBWTv3(input)
	fmt.Printf("Real hash: %s\n", hex.EncodeToString(hash[:]))
}
