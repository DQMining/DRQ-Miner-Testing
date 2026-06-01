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

	var step_3 [256]byte
	var counter [16]byte

	sha_key := sha256.Sum256(input)
	fmt.Printf("SHA256: %s\n", hex.EncodeToString(sha_key[:]))

	salsa.XORKeyStream(step_3[:], step_3[:], &counter, &sha_key)
	fmt.Printf("After Salsa20: %s\n", hex.EncodeToString(step_3[:32]))

	rc4s := astrobwt.NewCipher(step_3[:])
	rc4s.XORKeyStream(step_3[:], step_3[:])
	fmt.Printf("After RC4 init+xor: %s\n", hex.EncodeToString(step_3[:32]))
	fmt.Printf("Full step_3: %s\n", hex.EncodeToString(step_3[:]))

	lhash := fnv1a.HashBytes64(step_3[:])
	fmt.Printf("FNV1a: %016x\n", lhash)

	// Now do another RC4 XOR (simulating the <= 0x40 condition)
	rc4s.XORKeyStream(step_3[:], step_3[:])
	fmt.Printf("After 2nd RC4 xor: %s\n", hex.EncodeToString(step_3[:32]))
	fmt.Printf("s[255] after 2nd RC4: %02x\n", step_3[255])

	// And the real hash
	hash := astrobwt.AstroBWTv3(input)
	fmt.Printf("Real hash: %s\n", hex.EncodeToString(hash[:]))
}
