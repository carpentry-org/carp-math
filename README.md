# carp-math

A consolidated Math Library suite for the Carp programming language.

## Modules Included
- **[Color](#color)**: See module documentation below.
- **[Fft](#fft)**: See module documentation below.
- **[Matrix](#matrix)**: See module documentation below.
- **[Noise](#noise)**: See module documentation below.
- **[Simd](#simd)**: See module documentation below.
- **[Transform](#transform)**: See module documentation below.

## Installation

```
(load "git@github.com:carpentry-org/carp-math@master")
```

That pulls in every module. A single one can be loaded on its own:

```
(load "git@github.com:carpentry-org/carp-math@master" "matrix.carp")
```

## Examples

See [examples.md](examples.md) for module usage examples, and the
[API documentation](https://carpentry.dev/carp-math) for the full reference.

---

## Color

A graphics-oriented color library for the [Carp language](https://github.com/carp-lang/Carp).

## Features

- **RGB & RGBA Support:** Precise `Double` based color representation.
- **HSL Color Space:** Full support for HSL with robust RGB <-> HSL conversions.
- **Manipulation:** Helpers for `lighten`, `darken`, `saturate`, `desaturate`, and `lerp`.
- **Hex Strings:** Parse from and format to standard hex codes (e.g., `#ff0000`).
- **Byte Helpers:** Easy conversion to/from 0-255 byte ranges for interoperability with SDL, OpenGL, etc.

## Installation

```clojure
(load "git@github.com:carpentry-org/carp-math@master" "colors.carp")
(use Color)
```

The file is `colors.carp`, not `color.carp`. Carp searches the working
directory before the directory of the file doing the loading, and core's
`Test.carp` opens with `(load "Color.carp")`. A `color.carp` in the project
root answers that load on a case-insensitive filesystem, and every test in
the project then fails to find `IO.colorize`.

## Running Tests

```bash
carp -x test/color_test.carp
```

## Examples

See [examples.md](examples.md) for usage examples.

## License

MIT


---

## Matrix

A robust, low-level numerical matrix library for the [Carp language](https://github.com/carp-lang/Carp).

## Features

- **Safe & Unsafe API:** Choose between performance and safety with `Result`-returning functions or `unsafe-` variants.
- **True In-Place Mutation:** Memory-efficient operations like `add!`, `sub!`, and `hadamard!` that modify data without new allocations.
- **Cache-Aware Performance:** Optimized matrix multiplication using transposed layouts for better cache locality.
- **Zero-Copy Structural Ops:** Metadata-only `reshape` and `flatten` (currently behaves as copy due to Carp deftype semantics, but architected for minimal overhead).
- **Comprehensive API:** Includes `zeros`, `identity`, `random`, `transpose`, `dot`, `mat-vec`, `outer-product`, `approx=`, and more.
- **Rigorous Testing:** Deep verification of mathematical identities and edge cases.

## Installation

```clojure
(load "git@github.com:carpentry-org/carp-math@master" "matrix.carp")
(use Mat)
```

## Running Tests

```bash
carp -x test/matrix_test.carp
```

## Examples

See [examples.md](examples.md) for usage examples.

## License

MIT


---

## Noise

A fast, seeded Simplex Noise implementation for the [Carp](https://github.com/carp-lang/Carp) programming language.

## Features

- **Seeded Noise State**: Independent noise generators that do not rely on global RNG state.
- **2D & 3D Simplex Noise**: Implementation based on the canonical Simplex algorithms.
- **Fractal Brownian Motion (FBM)**: Composable fractal noise with configurable octaves, persistence, and lacunarity.
- **Performance**: Uses unsafe array access for hot paths and avoids allocations during sampling.

## Installation

Clone this repository into your project or add it as a submodule:

```bash
git submodule add https://github.com/sqrew/carp-noise.git
```


## Examples

See [examples.md](examples.md) for usage examples.
### Fractal Noise (FBM)

```carp
(let [state (make-noise 123)
      config (FBMConfig.init 6 0.5 2.0)
      val (fbm2d &state 0.1 0.2 &config)]
  (IO.println &(Double.to-string val)))
```

## Testing

Run the test suite with:

```bash
carp -x test/noise_test.carp
```

## License

This project is licensed under the MIT License.


---

## Fft

A robust, double-precision Fast Fourier Transform (FFT) and Digital Signal Processing (DSP) library for the [Carp language](https://github.com/carp-lang/Carp).

## Architecture

The library is separated into focused modules for easier extension and maintainability:

1. **Complex Numbers (`complex.carp`):** Loaded via dependency [carp-complex-numbers](https://github.com/sqrew/carp-complex-numbers), providing double-precision complex arithmetic (`+`, `-`, `*`, `/`, `abs`/`norm`, conjugate, exponentiation, trigonometric functions, etc.).
2. **Main Transforms (`fft.carp`):**
   - 1D FFT & IFFT (`fft`, `ifft`, `try-fft`, `try-ifft`) using a Cooley-Tukey Radix-2 decimation-in-time algorithm.
   - Real FFT (`rfft`, `rfft-one-sided`) and Complex-to-Real IFFT (`irfft`) which takes a one-sided spectrum (size $N/2 + 1$) and automatically reconstructs Hermitian symmetry before transforming.
   - 2D FFT & IFFT (`fft2d`, `ifft2d`, `try-fft2d`, `try-ifft2d`) via row-column decomposition.
   - Signal utilities like zero-padding (`pad-zero`, `pad-zero-complex`) and zero-frequency shifting (`fftshift`, `ifftshift`).
3. **Signal Windowing (`window.carp`):** Common window generators (Hann, Hamming, Blackman, Rectangular, Bartlett/Triangular, Gaussian, and Flat Top) with in-place multipliers (`apply!`, `apply-real!`).
4. **Spectral Analysis (`spectrum.carp`):** Functions to calculate magnitude, power, natural log, and decibel (dB) log-magnitude spectra (`magnitude`, `power`, `log-spectrum`, `db`).
5. **Signal Convolution (`convolution.carp`):** High-performance spectral domain operations: circular convolution (`circular-convolve`), linear convolution (`convolve`), cross-correlation (`cross-correlate`), and autocorrelation (`autocorrelate`).

## Installation

Add the library files to your project:

```clojure
(load "git@github.com:carpentry-org/carp-math@master" "convolution.carp")

(use Complex)
(use FFT)
(use Window)
(use Spectrum)
(use Convolution)
```

## Running Tests

Run the comprehensive test suite (verifying DSP behaviors like Impulse, Alternating/Nyquist spikes, Parseval's theorem, convolutions, and roundtrips):

```bash
carp -x test/fft_test.carp
```

## Examples

See [examples.md](examples.md) for usage examples.

## License

MIT


---

## Simd

A high-performance, architecture-scaling SIMD (Single Instruction Multiple Data) library for the [Carp](https://github.com/carp-lang/Carp) programming language.

This library leverages GCC/Clang Vector Extensions to compile dynamic vector math operations directly to native CPU SIMD instructions (SSE/AVX on x86_64, NEON on ARM/Apple Silicon), with zero heap allocation or runtime call overhead.

## Features
- **Auto-Scaling Lane Width**: Detects target hardware architecture at compile time, adapting lane size dynamically:
  - **AVX-512** (`__AVX512F__`): 16 Float lanes (512-bit registers)
  - **AVX / AVX2** (`__AVX2__`): 8 Float lanes (256-bit registers)
  - **SSE / NEON** (Default): 4 Float lanes (128-bit registers)
- **Zero-Overhead Primitives**: Emits inline operators (`+`, `-`, `*`, `/`) that standard C compilers vectorize directly into CPU instruction pipelines.
- **Pointer Loads/Stores**: Low-level functions to load and store batches of floats directly to and from contiguous arrays.

## Examples

See [examples.md](examples.md) for usage examples.

## Compilation Flags
To enable wider registers (like AVX2), pass the architecture flags to your C compiler when running `carp`:
```bash
carp -x main.carp --cflag="-mavx2"
```

## License
MIT


---

## Transform
A high-performance 3D transformation library for the [Carp](https://github.com/carp-lang/Carp) programming language.

This library provides a standardized way to manage the spatial state (Position, Rotation, Scale) of game objects. It uses Quaternions for stable rotations and generates 4x4 matrices compatible with WGPU.

## Design Conventions
- **Matrix Layout**: Column-major (OpenGL/WGPU style).
- **Coordinate System**: Right-handed.
- **Rotation**: Hamilton product quaternions.
- **Euler Convention**: YXZ intrinsic (Yaw-Pitch-Roll).

## Features
- **Quaternion Support**: Stable 3D rotations with automatic normalization to prevent drift.
- **Basis Extraction**: Easily get `forward`, `right`, and `up` vectors from any transform.
- **TRS to Matrix**: Efficiently generates `TransformMat4` (M = T * R * S).
- **Standardized**: Built on Carp's internal `Vector3` library.


## Examples

See [examples.md](examples.md) for usage examples.
## License
MIT


## License

MIT
