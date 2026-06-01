module test_vectors

go 1.24.0

toolchain go1.24.2

require github.com/deroproject/derohe v0.0.0-00010101000000-000000000000

require (
	github.com/cespare/xxhash v1.1.0 // indirect
	github.com/dchest/siphash v1.2.3 // indirect
	github.com/klauspost/cpuid/v2 v2.2.3 // indirect
	github.com/minio/sha256-simd v1.0.1 // indirect
	github.com/segmentio/fasthash v1.0.3 // indirect
	golang.org/x/crypto v0.48.0 // indirect
	golang.org/x/sys v0.41.0 // indirect
)

replace github.com/deroproject/derohe => ../derohe
