# Secure Multi-Party Computation With Leveled Linear Secret Sharings

This repository contains the code for the paper "Additions, Multiplications, and the Interaction In-Between: Optimizing MPC Protocols via Leveled Linear Secret Sharing".
First, we provide an implementation of our novel sharing assignment optimization algorithm which, given a circuit and protocol, generates the optimal extended circuit depending on the cost metric of the protocol.
Next, we provide a protocol implementation of our LLSS-MPC protocol based on replicated secret sharing, evaluating an extended circuit in that setting.

## Installation

Our implementations and needed dependencies can be manually installed or set up as a docker container.

### Docker

First, build our provided Dockerfile:

```sh
sudo docker build -t delayed-resharing:1.0 .
```

Next, run the container and follow the remaining instructions from inside of the container:

```sh
sudo docker run --privileged -it delayed-resharing:1.0 /bin/bash
```

### Manual Installation

This code is able to be run on Ubuntu 25.04.

To install prequisites run:

```sh
apt-get update && \
    apt-get install -y --no-install-recommends \
    software-properties-common \
    build-essential \
    cmake \
    git \
    libtool \
    autoconf \
    automake \
    clang \
    iproute2 \
    iputils-ping \
    && \
    apt-get update
apt install -y libboost-filesystem-dev libboost-thread-dev libboost-regex-dev libtool
```

To compile our code, run:

```sh
mkdir thirdparty
sudo bash setup_libote.sh
cd delayedresharing
bash build.sh
cd ..
cd Protocols
bash build.sh
cd ..
cd ShareAssigner
bash build.sh
cd ..
```

## Overview

The implemented Share Assignment Algorithm is contained in ```ShareAssigner/``` and the protocol implementation in ```Protocols/```.

All standard circuits in bristol file format (except for the arithmetic ones that we generated in our custom format) that we use for benchmarking are located inside the ```Circuits/``` folder.
They are taken from [nigelsmart.github.io](https://nigelsmart.github.io/MPC-Circuits/) and [MOTION](https://github.com/encryptogroup/MOTION/tree/dev/circuits).
We retain the original filenames here for keeping circuits and their origin clearly identifiable, whereas the paper uses shorter names for clarity.
The mapping is as follows:

- RCA: ```int_add32_size.bristol```
- PPA: ```int_add32_depth.bristol```
- Multiplier: ```int_mul32_depth.bristol```
- float-add.: ```float_add32_depth.bristol```
- float-mult.: ```float_mul32_depth.bristol```
- float-div.: ```FP-div.bristol```
- AES-128: ```aes_128.bristol```
- SHA-256: ```sha256.bristol```
- LowMC-256: ```lowmc_80-256.bristol```
- MSE: ```mse.txt```
- Neural-Net: ```NN.txt```

For extended circuits, we use a custom new format as the standard bristol file format is insufficient to annotate sharing semantics.
For simplicity of our evaluation, we also generate extended circuits which have inputs and outputs in higher domain, and always immediately upgrade the results of multiplication, which exactly emulates the behavior of "classical" MPC without LLSS.
These circuits can be found inside ```BaselineCircuits/``` for the different protocols.
Circuits with the optimized share assignments produced by our algorithm are stored in ```OptimizedCircuits/``` for the various settings.

The supported protocols are:

- Masked: 2 Party protocol with Masked Sharing.
- Shamir/<3/10>: 3 / 10 Party protocol with Shamir's Secret Sharing.
- RSS: 3 Party protocol Replicated Sharing.
- Weak: Share Assignment without choice of Input or Output operations in lower domain, see Appendix E.3.

## Execution

The different parts of our implementation can be executed as follows.
We provide the generic commands for any protocol and circuit alongside specific examples for the RCA/int_add32_depth example and the RSS-based protocol.

### Sharing Assignment

Sharing Assignment may be performed via

```sh
./ShareAssigner/build/DelayedResharing Circuits/<circuitfile>.<txt/bristol> BaselineCircuits/<circuitfile>.txt OptimizedCircuits/<circuitfile>.txt <replicated/masked/3shamir/10shamir/weak/offline>
```

**Example:**
```sh
./ShareAssigner/build/DelayedResharing Circuits/int_add32_depth.bristol BaselineCircuits/int_add32_depth.txt OptimizedCircuits/int_add32_depth.txt replicated
```

Here the baseline folder contains the circuits with classic protocol assignment and the optimized folder the assignment for delayed resharing.

The ```offline``` setting is for the preprocessing optimization from Appendix D.4, and only numerically outputs the number of "fixing" operations needed within a circuit and does not output actual circuits. The provided output paths are ignored.

The repository already contains readily prepared versions for all optimized circuits in the corresponding subdirectories ```Masked```, ```RSS```, ```Shamir```, ```Weak```.

### 3-Party RSS-based LLSS Protocol Execution

First, navigate into the ```Protocols``` directory.

**Example:**
```sh
cd Protocols # if not already done before
python3 network.py start 3 LAN
# In the next two lines, you may also use int_add32_depth.txt instead of
# RSS/int_add32_depth.txt if you manually ran the Sharing Assignment
# before to freshly generate the circuit.
# Code as provided below uses the already provided circuits.
# Each benchmark will run for 10 iterations total.
bash bench_baseline.sh RSS/int_add32_depth.txt 1
# Outputs from all parties and 10 iterations will be printed
# Last output will be average run times for each of the parties
bash bench_optimized.sh RSS/int_add32_depth.txt 1
# Outputs from all parties and 10 iterations will be printed
# Last output will be average run times for each of the parties
python3 network.py stop 3 LAN
```

Observe from the run times for baseline and optimized extended circuits how the optimized version should be faster.
Yet, note that run times are subject to the specific machine and may not exactly match those reported in the paper due
to differences in benchmarking setup.
In the paper, we always consider the maximum run time among the three parties.

**Full Description:**

To start or stop network emulation in LAN or WAN, run

```sh
python3 network.py <start/stop> 3 <WAN/LAN>
```
This creates the virtual network interfaces needed to execute the protocol below.
Remember to always stop the emulation later, as starting another emulation may lead to unexpected stray virtual interfaces requiring manual cleanup.

To select the extended circuit to execute, you have to specify a circuitfile.
This can be, e.g., ```int_add32_depth.txt``` inside directory ```BaselineCircuits``` if generated in the prior Sharing Assignment step.
In addition, we provide already prepared circuits inside ```BaselineCircuits/Masked```, ```BaselineCircuits/RSS```, ```BaselineCircuits/Shamir```, ```BaselineCircuits/Weak```, optimized for the respective protocols.
In this case, use, e.g., ```RSS/int_add32_depth.txt``` as circuitfile for the circuit optimized for RSS, already provided with the repository.

To execute a protocol (baseline or our optimized extended circuit), run

```sh
bash bench_<baseline/optimized>.sh <circuitfile> <ringsize>
```

with a circuit file that has a share assignment and parameter ring size set to 1 for Boolean circuits and set to 2 for 64-bit Arithmetic circuits.

This launches three parties locally that run the protocol as within our benchmark.
The ringsize parameter can be set to $1$ for binary computation and $2$ for 64 bit ring elements. The latter is intended for the NN.txt and mse.txt example which are included allready.
The benchmark results of the execution are written to the ```Protocols/Outputs``` folder with seperate files for each party.

Alternatively, individual parties can be started with
```sh
bash launch_p<0/1/2>.sh <circuitfile> <ringsize>
```

## Cost calculation for protocols

(Be sure to navigate to the top level directory if you previously navigated into ```Protocols```)

The circuits optimized for the different protocols via our optimizer are provided in ```OptimizedCircuits```.
The cost calculation scripts
```sh
python3 calculate_cost_<mask/replicated/shamir_3/shamir_10>.py
```
report the online communication required to execute the circuits for the protocol settings reported in the paper along with their improvement.
Offline communication is computed similarly with the ```_offline``` suffix.

The scripts use the optimized circuits provided together with this repository.
You may overwrite them with newly optimized circuits, or adapt the scripts.

**Example:**
```sh
python3 calculate_cost_replicated.py
```

## Note

All code provided in this repository is research-level code and should not be deployed in a production setting.

Our custom circuit format always uses the labels "Additive" and "Blinded" to denote the choice of lower and higher sharing assignment. These labels therefore mean different sharings depending on the context, e.g., for replicated "Additive" -> "3ASS" and "Blinded" -> "RSS".
