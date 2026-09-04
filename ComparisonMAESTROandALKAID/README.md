# Comparison to MAESTRO and ALKAID

In Appendix §D-D of our [full version](https://eprint.iacr.org/2026/368.pdf), we provide additional
comparison to MAESTRO and ALKAID. This directory contains scripts etc. used to obtain the numbers
in the comparison to provide full transparency and details on how these comparisons were made.

## Generating Table Data

Simply run
```sh
python3 compare_related_works.py
```
to obtain all numbers used in the appendix. The script analytically computs communication cost for
ALKAID and MAESTRO. For other communication costs, it hardscripts the depth and number of gates
in the respective circuits. These are obtained as explained in the following:

## Hardscripted Circuit Sizes

The gate counts for AES-128 are directly taken from Table VIII of our paper, the concrete circuits
can be found [here](../Circuits/aes_128.bristol) (unoptimized) and
[here](../OptimizedCircuits/Weak/aes_128.txt) (optimized).

For comparing PPA performance to ALKAID, we need 64 bit adders. We consider different PPAs and provide
the circuits here for reproducability.

For all circuits, simply counting the numbers of "AND" in bristol files or "additive:blinded" in our
format provides the number of AND gates respectively upgrade gates.

### MOTION Sklansky Adder

Taken from [here](https://github.com/encryptogroup/MOTION/blob/dev/circuits/int/int_add64_depth.bristol),
also provided as [int_add64_depth.bristol](./int_add64_depth.bristol) in this directory.

### ABY3/cryptoTools Sklansky Adder

The [original code](https://github.com/ladnir/cryptoTools/blob/1a344ee4b7f4afaf39193eb413300ba17962b19e/cryptoTools/Circuit/BetaLibrary.cpp#L865)
generates a circuit without reading from or writing to a file. We translated their code to Python
code for generating a bristol file in [genCircuit_ABY3.py](./genCircuit_ABY3.py).
The circuit can be compiled as follows:
```sh
python3 genCircuit_ABY3.py > circuit_ABY3.bristol
```
and is already provided [here](./circuit_ABY3.bristol).

### SecretFlow-SPU Kogge-Stone Adder (A)

The [original code](https://github.com/secretflow/spu/blob/4d470a2340846d84902c08c176f83c309fc579fb/src/libspu/mpc/ab_api.cc#L314)
generates a circuit without reading from or writing to a file. We translated their code to Python
code for generating a bristol file in [genCircuit_SPU.py](./genCircuit_SPU.py).
The circuit can be compiled as follows:
```sh
python3 genCircuit_SPU.py > circuit_SPU.bristol
```
and is already provided [here](./circuit_SPU.bristol).

### Improved SecretFlow-SPU Kogge-Stone Adder (B)

We find that the Kogge-Stone Adder A introduces plenty padding gates which serve no purpose. We
also consider "Kogge-Stone Adder B" which removes those to get an improved version.
Python code to generate the bristol file in [genCircuit_SPU_improved.py](./genCircuit_SPU_improved.py).
The circuit can be compiled as follows:
```sh
python3 genCircuit_SPU_improved.py > circuit_SPU_improved.bristol
```
and is already provided [here](./circuit_SPU_improved.bristol).

### Optimized Circuit Variants

To optimize the circuits with our optimizer, use the following commands, which only generates the
optimized version for inputs and outputs in higher sharings.
```sh
../ShareAssigner/build/DelayedResharing int_add64_depth.bristol /dev/null int_add64_depth_optimized.txt weak
../ShareAssigner/build/DelayedResharing circuit_ABY3.bristol /dev/null circuit_ABY3_optimized.txt weak
../ShareAssigner/build/DelayedResharing circuit_SPU.bristol /dev/null circuit_SPU_optimized.txt weak
../ShareAssigner/build/DelayedResharing circuit_SPU_improved.bristol /dev/null circuit_SPU_improved_optimized.txt weak
```
We already provide the outputs in this directory, using the suffix "_optimized.txt".

### Licenses

Our code to generate the circuit files for the implementations in cryptoTools and SecretFlow-SPU are
based on the respective original code bases. We provide the licenses for both in this directory,
making our derived code to generate their adders available under the same conditions.
