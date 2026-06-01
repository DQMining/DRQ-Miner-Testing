package main

import (
	"encoding/hex"
	"fmt"

	sha256 "github.com/minio/sha256-simd"
	"github.com/segmentio/fasthash/fnv1a"
	"golang.org/x/crypto/salsa20/salsa"

	astrobwt "github.com/deroproject/derohe/astrobwt/astrobwtv3"
)

func main() {
	input := []byte{0x01, 0x02, 0x03, 0x04}

	fmt.Printf("Input: %s\n", hex.EncodeToString(input))

	// Step 1: SHA-256 of input
	sha_key := sha256.Sum256(input)
	fmt.Printf("SHA256 key: %s\n", hex.EncodeToString(sha_key[:]))

	// Step 2: Salsa20
	var step_3 [256]byte
	var counter [16]byte
	salsa.XORKeyStream(step_3[:], step_3[:], &counter, &sha_key)
	fmt.Printf("After Salsa20 (first 64): %s\n", hex.EncodeToString(step_3[:64]))
	fmt.Printf("After Salsa20 (last 64):  %s\n", hex.EncodeToString(step_3[192:]))

	// Step 3: RC4
	rc4s := astrobwt.NewCipher(step_3[:])
	rc4s.XORKeyStream(step_3[:], step_3[:])
	fmt.Printf("After RC4 (first 64): %s\n", hex.EncodeToString(step_3[:64]))
	fmt.Printf("After RC4 (last 64):  %s\n", hex.EncodeToString(step_3[192:]))

	// Step 4: FNV1a
	lhash := fnv1a.HashBytes64(step_3[:])
	fmt.Printf("FNV1a lhash: %016x\n", lhash)

	// Step 5: First few iterations of the loop
	prev_lhash := lhash
	tries := uint64(0)
	for iter := 0; iter < 5; iter++ {
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
		fmt.Printf("Iter %d: tries=%d rs=%016x op=%d pos1=%d pos2=%d\n", iter, tries, random_switcher, op, pos1, pos2)
		fmt.Printf("  step_3[0:8] before: %s\n", hex.EncodeToString(step_3[:8]))
	}

	// Full hash
	hash := astrobwt.AstroBWTv3(input)
	fmt.Printf("\nFinal hash: %s\n", hex.EncodeToString(hash[:]))
}
