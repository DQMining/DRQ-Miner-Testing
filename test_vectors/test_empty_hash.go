package main

import (
	"fmt"
	"github.com/cespare/xxhash"
	"github.com/segmentio/fasthash/fnv1a"
	"github.com/dchest/siphash"
)

func main() {
	empty := []byte{}
	fmt.Printf("xxhash64(empty) = %016x\n", xxhash.Sum64(empty))
	fmt.Printf("fnv1a(empty) = %016x\n", fnv1a.HashBytes64(empty))
	fmt.Printf("siphash(1, prev, empty) = %016x\n", siphash.Hash(1, 0x293cbca5d2af413e, empty))
}
