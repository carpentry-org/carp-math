# Examples for carp-math

This file contains usage examples for all sub-modules in the library.

## Color

## Basic Conversions

```clojure
(use Color)

(defn main []
  (let [c (Color.red)
        h (Color.rgb-to-hsl &c)
        light (Color.lighten &c 0.2)
        hex (Color.to-hex &light)]
    (println* "Hex: " hex)))
```

## Parsing Hex

```clojure
(match (Color.from-hex "#3498db")
  (Result.Success c) (println* "HSL: " (str &(Color.rgb-to-hsl &c)))
  (Result.Error msg) (println* "Error: " msg))
```

## Mixing Colors (Lerp)

```clojure
(let [c1 (Color.red)
      c2 (Color.blue)
      mid (Color.lerp &c1 &c2 0.5)]
  (println* "Mid: " (str &mid)))
```


---

## Matrix

Here are some common ways to use `carp-matrix`.

## Basic Creation and Arithmetic

```clojure
(use Mat)

(defn main []
  (let [m1 (Mat.zeros 2 2)
        m2 (Mat.unsafe-from-array 2 2 [1.0 2.0 3.0 4.0])
        m3 (Mat.unsafe-from-array 2 2 [5.0 6.0 7.0 8.0])
        res (Mat.unsafe-add &m2 &m3)]
    (println* "Result: " (str &res))))
```

## Matrix Multiplication

```clojure
(let [a (Mat.unsafe-from-array 2 3 [1.0 2.0 3.0 4.0 5.0 6.0])
      b (Mat.unsafe-from-array 3 2 [7.0 8.0 9.0 10.0 11.0 12.0])
      c (Mat.unsafe-mul &a &b)]
  (println* "Product: " (str &c)))
```

## In-Place Operations

Use `!` functions to avoid allocations in hot loops.

```clojure
(let [m (Mat.random 100 100)]
  (do
    (ignore (Mat.scalar-mul! &m 2.0))
    (ignore (Mat.add! m &m)) ; Double it again in-place
    (println* "Sum: " (Mat.sum &m))))
```

## Safe API (Result Handling)

```clojure
(let [m (Mat.zeros 2 2)]
  (match (Mat.row &m 5) ; Index out of bounds
    (Result.Success r) (println* "Row: " (str &r))
    (Result.Error msg) (println* "Failed: " msg)))
```


---

## Noise

## Basic 2D Noise

Generating a simple 2D heightmap value:

```carp
(use Noise)

(let [state (make-noise 42)
      x 10.5
      y 20.1
      h (noise2d &state x y)]
  (IO.println &(format "Height at %f, %f is %f" x y h)))
```

## 3D Noise

Useful for volumetric noise or time-varying 2D noise:

```carp
(let [state (make-noise 99)
      time 1.23
      val (noise3d &state 0.5 0.5 time)]
  (IO.println &(str val)))
```

## Fractal Brownian Motion (FBM)

FBM combines multiple "octaves" of noise to create more natural-looking textures like clouds or terrain.

```carp
(let [state (make-noise 1)
      ;; 6 octaves, 0.5 persistence (diminishing returns), 2.0 lacunarity (increasing frequency)
      config (FBMConfig.init 6 0.5 2.0)
      val (fbm2d &state 0.01 0.01 &config)]
  (IO.println &(str val)))
```

## Seeding for Procedural Generation

You can create multiple independent noise states to ensure consistency across different biomes or layers:

```carp
(let [ground-noise (make-noise 100)
      cave-noise (make-noise 200)
      vegetation-noise (make-noise 300)]
  (do
    ;; Each use of these states is deterministic and independent
    ...))
```


---

## Fft

Here are some common ways to use the modular `carp-fft` library.

## Setup

First, load the required modules in your Carp entry point:

```clojure
(load "git@github.com:carpentry-org/carp-math@master" "complex.carp")
(load "git@github.com:carpentry-org/carp-math@master" "fft.carp")
(load "git@github.com:carpentry-org/carp-math@master" "window.carp")
(load "git@github.com:carpentry-org/carp-math@master" "spectrum.carp")
(load "git@github.com:carpentry-org/carp-math@master" "convolution.carp")

(use Complex)
(use FFT)
(use Window)
(use Spectrum)
(use Convolution)
```

## Basic Complex Arithmetic

Create and manipulate complex numbers using the `Complex` module.

```clojure
(defn main []
  (let [c1 (Complex.init 3.0 4.0)          ; 3 + 4i
        c2 (Complex.init 1.0 -2.0)         ; 1 - 2i
        c-sum (Complex.+ &c1 &c2)          ; 4 + 2i
        c-prod (Complex.* &c1 &c2)         ; 11 - 2i
        mag (Complex.abs &c1)]             ; 5.0
    (do
      (println* "Sum: " (str &c-sum))
      (println* "Product: " (str &c-prod))
      (println* "Magnitude: " mag))))
```

## 1D FFT and IFFT

Perform a Fast Fourier Transform and reconstruct the original signal with the inverse FFT.

```clojure
(defn main []
  ;; Create a simple signal (e.g., 8-point sine wave + DC component)
  (let [signal [(Complex.init 1.0 0.0) (Complex.init 1.707 0.0)
                (Complex.init 2.0 0.0) (Complex.init 1.707 0.0)
                (Complex.init 1.0 0.0) (Complex.init 0.293 0.0)
                (Complex.init 0.0 0.0) (Complex.init 0.293 0.0)]
        ;; The non-try API panics on size mismatches, making it cleaner to use
        spectrum (FFT.fft &signal)
        reconstructed (FFT.ifft &spectrum)]
    (do
      (println* "Spectrum:")
      (for [i 0 8]
        (println* "  Freq " i ": " (str (Array.unsafe-nth &spectrum i))))
      (println* "Reconstructed matches original: "
                (Complex.approx (Array.unsafe-nth &reconstructed 1)
                                (Array.unsafe-nth &signal 1)
                                0.0001)))))
```

## Real FFT with Windowing

Apply a window function (like Hann or Flat Top) to a real-valued signal, perform a real-to-complex transform, and reconstruct.

```clojure
(defn main []
  (let [size 1024
        ;; 1. Generate window coefficients
        window (Window.hann size)
        ;; 2. Create sample real-valued signal (e.g., 1024-point sine)
        signal (Array.replicate size &0.5)]
    (do
      ;; 3. Apply window in-place
      (Window.apply-real! &signal &window)
      ;; 4. Run real FFT to get one-sided spectrum
      (let [spectrum (FFT.rfft-one-sided &signal)]
        (do
          (println* "One-sided spectrum length: " (Array.length &spectrum)) ; 513
          ;; 5. Reconstruct back to time domain
          (let [reconstructed (FFT.irfft &spectrum)]
            (println* "Reconstructed real signal length: " (Array.length &reconstructed))))))))
```

## Power and dB Spectra Analysis

Perform spectral analysis using the `Spectrum` module to inspect magnitudes or power in decibels.

```clojure
(defn main []
  (let [signal [(Complex.init 3.0 4.0) (Complex.init 0.0 -5.0)]
        mags (Spectrum.magnitude &signal)    ; [5.0, 5.0]
        power (Spectrum.power &signal)       ; [25.0, 25.0]
        db-vals (Spectrum.db &signal)]        ; dB scaled
    (do
      (println* "Magnitudes: " @(Array.unsafe-nth &mags 0))
      (println* "Power: " @(Array.unsafe-nth &power 0))
      (println* "dB (0): " @(Array.unsafe-nth &db-vals 0)))))
```

## Linear Convolution and Autocorrelation

Filter signals in the time-domain using high-performance frequency-domain convolution.

```clojure
(defn main []
  (let [a [1.0 2.0 3.0]
        b [0.0 1.0 0.5]
        ;; Performs fast linear convolution using RFFT/IRFFT internally
        conv (Convolution.convolve &a &b)              ; [0.0, 1.0, 2.5, 4.0, 1.5]
        ;; Auto-correlation (cross-correlation with itself)
        acorr (Convolution.autocorrelate &a)]           ; [2.0, 5.0, 2.0]
    (do
      (println* "Convolution length: " (Array.length &conv))
      (println* "Autocorrelation at lag 0 (energy): " @(Array.unsafe-nth &acorr 1)))))
```


---

## Simd

## Basic Usage

Initializing SIMD registers, performing parallel addition, accessing lanes, and checking hardware-supported lane width:

```clojure
(load "git@github.com:carpentry-org/carp-math@master" "simd.carp")
(use Simd)

(defn main []
  (let [;; 1. Initialize SIMD vectors (4 float lanes initialized explicitly)
        v1 (Simd.init 1.0f 2.0f 3.0f 4.0f)
        v2 (Simd.init 10.0f 20.0f 30.0f 40.0f)
        
        ;; 2. Perform parallel addition (4, 8, or 16 calculations in a single instruction!)
        result (Simd.add v1 v2)]
    (do
      (println* "Lane 0: " (Simd.get result 0))   ;; Outputs: 11.0
      (println* "Lane 3: " (Simd.get result 3))   ;; Outputs: 44.0
      
      ;; 3. Check target hardware lane width
      (println* "Compile-time lane width: " (Simd.width)))))
```


---

## Transform

## 1. Creating and Moving a Transform
```clojure
(load "transform.carp")
(use Transform)
(use Vector3)

(defn main []
  (let [t (Transform.identity)]
    (do
      ;; Move the object
      (Transform.set-position! &t (Vector3.init 10.0 0.0 5.0))
      
      ;; Scale it up
      (Transform.set-scale! &t (Vector3.init 2.0 2.0 2.0))
      
      (let [m (Transform.to-matrix &t)]
        (println* "Model Matrix data: " (str (TransformMat4.data &m)))))))
```

## 2. Rotation and Basis Vectors
```clojure
(load "transform.carp")
(use Transform)
(use Quaternion)
(use Vector3)

(defn main []
  (let [t (Transform.identity)
        ;; Rotate 90 degrees around Y axis
        rot (Quaternion.from-euler 0.0 1.5707 0.0)]
    (do
      (Transform.set-rotation! &t rot)
      
      ;; Extract the new world-space forward vector
      (let [fwd (Transform.forward &t)]
        (println* "New forward direction: " (Vector3.str &fwd))))))
```

