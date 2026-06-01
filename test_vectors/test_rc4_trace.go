package main

import (
	"encoding/hex"
	"fmt"

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

	fmt.Printf("After initial RC4: %s\n", hex.EncodeToString(step_3[:32]))

	lhash := fnv1a.HashBytes64(step_3[:])
	prev_lhash := lhash
	fmt.Printf("lhash=%016x prev=%016x\n", lhash, prev_lhash)

	// Iter 1
	tries := uint64(1)
	random_switcher := prev_lhash ^ lhash ^ tries
	op := byte(random_switcher)
	pos1 := byte(random_switcher >> 8)
	pos2 := byte(random_switcher >> 16)
	if pos1 > pos2 { pos1, pos2 = pos2, pos1 }
	if pos2-pos1 > 32 { pos2 = pos1 + (pos2-pos1)&0x1f }
	fmt.Printf("Iter 1: op=%d pos1=%d pos2=%d\n", op, pos1, pos2)

	// Switch case 1 with pos1==pos2 -> no-op
	fmt.Printf("After switch: %s\n", hex.EncodeToString(step_3[:32]))

	// Post-loop conditions
	diff := step_3[pos1] - step_3[pos2]
	fmt.Printf("diff = s[%d]-s[%d] = %d\n", pos1, pos2, diff)

	if diff < 0x10 {
		prev_lhash = lhash + prev_lhash
		lhash = xxhash.Sum64(step_3[:pos2])
		fmt.Printf("After xxhash: lhash=%016x prev=%016x\n", lhash, prev_lhash)
	}
	if diff < 0x20 {
		prev_lhash = lhash + prev_lhash
		lhash = fnv1a.HashBytes64(step_3[:pos2])
		fmt.Printf("After fnv1a: lhash=%016x prev=%016x\n", lhash, prev_lhash)
	}
	if diff < 0x30 {
		prev_lhash = lhash + prev_lhash
		lhash = siphash.Hash(tries, prev_lhash, step_3[:pos2])
		fmt.Printf("After siphash: lhash=%016x prev=%016x\n", lhash, prev_lhash)
	}
	if diff <= 0x40 {
		fmt.Printf("Before RC4 re-xor: %s\n", hex.EncodeToString(step_3[:32]))
		rc4s.XORKeyStream(step_3[:], step_3[:])
		fmt.Printf("After RC4 re-xor: %s\n", hex.EncodeToString(step_3[:32]))
	}

	step_3[255] = step_3[255] ^ step_3[pos1] ^ step_3[pos2]
	fmt.Printf("After s[255] update: s[255]=%02x\n", step_3[255])
	fmt.Printf("Final state: %s\n", hex.EncodeToString(step_3[:32]))
}
