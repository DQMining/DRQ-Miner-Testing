package main

import (
	"encoding/hex"
	"fmt"
	"os"
	"path/filepath"

	astrobwt "github.com/deroproject/derohe/astrobwt/astrobwtv3"
)

func main() {
	testCases := [][]byte{
		{},
		{0x00},
		{0x01, 0x02, 0x03, 0x04},
		[]byte("Hello, DERO!"),
		[]byte("test input for astrobwt v3"),
		make([]byte, 76),
	}

	for i := range testCases[5] {
		testCases[5][i] = byte(i)
	}

	for i, input := range testCases {
		hash := astrobwt.AstroBWTv3(input)
		fmt.Printf("Test %d:\n", i)
		fmt.Printf("  Input (%d bytes): %s\n", len(input), hex.EncodeToString(input))
		fmt.Printf("  Hash:  %s\n", hex.EncodeToString(hash[:]))
	}

	exePath, _ := os.Executable()
	fmt.Printf("\nRun from: %s\n", filepath.Dir(exePath))
}
