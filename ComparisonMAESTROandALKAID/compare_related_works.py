from math import ceil, log

def alkaid(l):
    """
    https://eprint.iacr.org/2025/2298.pdf Lemma 2
    """
    N2 = (l - 1) / 3
    N3 = N2
    N4 = N2

    N2p = l * log(l, 4) / 4 - (l - 1) / 3
    N3p = N2p
    N4p = N2p

    rounds = 1 + ceil(log(l, 4))
    offline = 3 * N3 + 3 * N3p + 6 * N4 + 9 * N4p + 3 * l
    online = 6 * (N2 + N3 + N4) + 12 * (N2p + N3p + N4p) + 3 * l

    return (rounds, offline, online)

def number_Sboxes_main_AES(): # AES128
    return 10 * 16 # 10 rounds, 128 bit block consists of 16 bytes

def number_Sboxes_key_expansion(): # AES128
    return 40 # https://eprint.iacr.org/2024/1317.pdf §C.1: for i=1,...,10, r=0,...,3 one Sbox each

def number_Sboxes_complete_AES():
    return number_Sboxes_main_AES() + number_Sboxes_key_expansion()

def AES_Chida(include_key_expansion = True):
    """
    https://dl.acm.org/doi/pdf/10.1145/3267973.3267977

    S-Box consists of 4 GF(2^8) multiplications in 3 rounds
    Each multiplication has 8*3 bits communication
    """
    off_rounds = 0
    rounds = 10 * 3
    offline = 0
    online = 0
    if include_key_expansion:
        online += 4 * 8 * 3 * number_Sboxes_complete_AES()
        # no additional rounds:
        # subkeys k^0 are available immediately, k^{i+1} is available one Sbox after k^{i}
        # (also see https://eprint.iacr.org/2024/1317.pdf §C). AES adds round keys after the
        # SubBytes step which is an Sbox, so each round r=1,...,10 can generate its round key
        # in parallel to computing SubBytes to then add it. In round r=0, k^0 is used which does
        # not need communication rounds to be computed.
    else:
        online += 4 * 8 * 3 * number_Sboxes_main_AES()
    return (off_rounds, rounds, offline, online)

def maestro_LUT_16():
    """
    https://eprint.iacr.org/2024/1317.pdf
    Protocol 3 with Protocol 8 to generate OHV
    """
    off_rounds = 0
    rounds = 0
    offline = 0
    online = 0
    # Prot 4/8 (LUT <.>->[.]) on 4 bit value ==> 1 round, 8 bit per party
    # Prot 8 OHV:
    # 6 + 4 + 1 RSS mults on bits in 2 rounds
    off_rounds += 2
    offline += 11 * 3
    # Prot 4: OHV with Prot 8, reconstruct a additively shared k=4 bit value ==> 1 round, 2 * 4 * 3 comm
    rounds += 1
    online += 2 * 4 * 3
    # Two parallel RSS multiplications on 4bit vals ==> 1 round, 2 * 4 * 3(parties) bits communication total
    rounds += 1
    online += 2 * 4 * 3
    return (off_rounds, rounds, offline, online)

def AES_maestro_LUT_16(include_key_expansion = True):
    off_rounds, rounds, offline, online = maestro_LUT_16()

    if include_key_expansion:
        offline *= number_Sboxes_complete_AES()
        online *= number_Sboxes_complete_AES()
        rounds *= 10
        # no additional rounds for key expansion:
        # subkeys k^0 are available immediately, k^{i+1} is available one Sbox after k^{i}
        # (also see https://eprint.iacr.org/2024/1317.pdf §C). AES adds round keys after the
        # SubBytes step which is an Sbox, so each round r=1,...,10 can generate its round key
        # in parallel to computing SubBytes to then add it. In round r=0, k^0 is used which does
        # not need communication rounds to be computed.
    else:
        offline *= number_Sboxes_main_AES()
        online *= number_Sboxes_main_AES()
        rounds *= 10

    return (off_rounds, rounds, offline, online)

def maestro_GF24():
    """
    https://eprint.iacr.org/2024/1317.pdf
    Protocol 3 but with the GF(2^4) inverse [v^{-1}] = [v^2] * [v^4] * [v^8]
    """
    off_rounds = 0
    rounds = 0
    offline = 0
    online = 0
    # invert 4 bit value by squaring locally, then two multiplications
    # ==> we need to convert additive to replicated first!
    rounds += 2
    online += 4 * 3 * 3 # 4 bit values, 3 parties, 3 resharings (one to convert, 2 for multiplicationss)
    # Two parallel RSS multiplications on 4bit vals ==> 1 round, 2 * 4 * 3(parties) bits communication total
    rounds += 1
    online += 2 * 4 * 3
    return (off_rounds, rounds, offline, online)

def AES_maestro_GF24(include_key_expansion = True):
    off_rounds, rounds, offline, online = maestro_GF24()

    if include_key_expansion:
        offline *= number_Sboxes_complete_AES()
        online *= number_Sboxes_complete_AES()
        rounds *= 10
        # no additional rounds for key expansion:
        # subkeys k^0 are available immediately, k^{i+1} is available one Sbox after k^{i}
        # (also see https://eprint.iacr.org/2024/1317.pdf §C). AES adds round keys after the
        # SubBytes step which is an Sbox, so each round r=1,...,10 can generate its round key
        # in parallel to computing SubBytes to then add it. In round r=0, k^0 is used which does
        # not need communication rounds to be computed.
    else:
        offline *= number_Sboxes_main_AES()
        online *= number_Sboxes_main_AES()
        rounds *= 10

    return (off_rounds, rounds, offline, online)

def maestro_23_LUT_256():
    """
    https://eprint.iacr.org/2024/1317.pdf
    Protocol 4 using Protocol 5 to generate len 256 OHV
    But we have inputs in [.], so according to Table 4, it gets a bit cheaper
    """
    off_rounds = 0
    rounds = 0
    offline = 0
    online = 0
    # len 256 OHV with Protocol 5:
    def ohv(k):
        inner_rounds = 0
        inner_comm = 0
        if k > 1:
            iir, iic = ohv(k - 1)
            inner_rounds += iir
            inner_comm += iic
            # 2^{k-1} - 1 ANDs
            inner_rounds += 1
            inner_comm += (2**(k-1) - 1) * 3
        return (inner_rounds, inner_comm)
    ir, ic = ohv(8)
    off_rounds += ir
    offline += ic
    # reconstruct replicated:
    rounds += 1
    online += 1 * 8 * 3
    # Table 4 check:
    assert offline == (256 - 8 - 1) * 3
    assert online == (8) * 3
    assert rounds == 1
    return (off_rounds, rounds, offline, online)

def AES_maestro_23_LUT_256(include_key_expansion = True):
    off_rounds, rounds, offline, online = maestro_23_LUT_256()

    if include_key_expansion:
        offline *= number_Sboxes_complete_AES()
        online *= number_Sboxes_complete_AES()
        rounds *= 10
        # no additional rounds for key expansion:
        # subkeys k^0 are available immediately, k^{i+1} is available one Sbox after k^{i}
        # (also see https://eprint.iacr.org/2024/1317.pdf §C). AES adds round keys after the
        # SubBytes step which is an Sbox, so each round r=1,...,10 can generate its round key
        # in parallel to computing SubBytes to then add it. In round r=0, k^0 is used which does
        # not need communication rounds to be computed.
    else:
        offline *= number_Sboxes_main_AES()
        online *= number_Sboxes_main_AES()
        rounds *= 10

    return (off_rounds, rounds, offline, online)

def maestro_33_LUT_256_AES_wo_key_expansion():
    """
    https://eprint.iacr.org/2024/1317.pdf
    Protocol 7 with Protocols 6 and 8 offline to generate 2 len 16 OHVs

    !!! This is more like a complete AES evaluation, not really decomposable into the same interactive blocks
    """
    off_rounds = 0
    rounds = 0
    offline = 0
    online = 0

    def ohv(k):
        assert k == 4
        # 11 RSS mults in Protocol 8
        return (2, 11 * 3)

    # 16 times LUT [.] -> <.> on bytes (Protocol 6 variant)
    # - 2 * OHV on 4 bit values (parallel)
    # - recover 8 bits additive
    ir, ic = ohv(4)
    off_rounds += ir
    offline += 16 * 2 * ic
    rounds += 1
    online += 16 * 2 * 8 * 3

    # 9 * 16 times LUT <.>-><.> (and upgr input) on bytes (Protocol 6)
    # - 2 * OHV on 4 bit values (parallel)
    # - recover 8 bits additive
    ir, ic = ohv(4)
    off_rounds += 0 # parallel
    offline += 9 * 16 * 2 * ic
    rounds += 9 * 1
    online += 9 * 16 * 2 * 8 * 3

    return (off_rounds, rounds, offline, online)

def AES_maestro_33_LUT_256(include_key_expansion = True, optimize_total_comm = False):
    off_rounds, rounds, offline, online = maestro_33_LUT_256_AES_wo_key_expansion()

    if include_key_expansion:
        if optimize_total_comm:
            ke_off_rounds, ke_rounds, ke_offline, ke_online = maestro_GF24()
        else:
            ke_off_rounds, ke_rounds, ke_offline, ke_online = maestro_23_LUT_256()
        offline += number_Sboxes_key_expansion() * ke_offline
        online += number_Sboxes_key_expansion() * ke_online
        rounds = max(rounds, ke_rounds * 10) # key expansion and main AES can run in parallel.

    return (off_rounds, rounds, offline, online)

"""
Check against https://eprint.iacr.org/2024/1317.pdf Table 1.
Note that this table excludes the key expansion and has communication per party
"""
off_r, on_r, off, on = AES_Chida(False)
assert off_r == 0 and off // 3 == 0 and on_r == 30 and on // 3 == 5120
off_r, on_r, off, on = AES_maestro_GF24(False)
assert off_r == 0 and off // 3 == 0 and on_r == 30 and on // 3 == 3200
off_r, on_r, off, on = AES_maestro_LUT_16(False)
assert off_r == 2 and off // 3 == 1760 and on_r == 20 and on // 3 == 2560
off_r, on_r, off, on = AES_maestro_33_LUT_256(False)
assert off_r == 2 and off // 3 == 3520 and on_r == 10 and on // 3 == 2560
off_r, on_r, off, on = AES_maestro_23_LUT_256(False)
#!!! table states off_r == 6, but §4 says we use Prot. 5 for len 256 OHV, §3.4 says this has log N - 1
#!!! rounds, log 256 - 1 = 7.
assert off_r == 7 and off // 3 == 39520 and on_r == 10 and on // 3 == 1280

# AES128 Circuit: 6400 ANDs, depth 60
# optimized LLSS version (§D-C without I/O, in and out wires all in higher domain): 4368 upgrades, depth 60
AES_CIRC_ANDS = 6400
AES_CIRC_OPT_UPGRADES = 4368
AES_CIRC_DEPTH = 60
print(f"Protocol                                 | offline comm. | online comm. | total comm. | online rounds")
print(f"AES128 RSS                               |           --- | {AES_CIRC_ANDS * 3:>12} | {AES_CIRC_ANDS * 3:>11} | {AES_CIRC_DEPTH:>13}")
print(f"AES128 Leveled RSS                       |           --- | {AES_CIRC_OPT_UPGRADES * 3:>12} | {AES_CIRC_OPT_UPGRADES * 3:>11} | {AES_CIRC_DEPTH:>13}")
_, r, off, on = AES_Chida()
print(f"AES128 Chida et al.                      | {off:>13} | {on:>12} | {off+on:>11} | {r:>13}")
_, r, off, on = AES_maestro_LUT_16()
print(f"AES128 MAESTRO LUT-16                    | {off:>13} | {on:>12} | {off+on:>11} | {r:>13}")
_, r, off, on = AES_maestro_GF24()
print(f"AES128 MAESTRO GF(2^4)                   | {off:>13} | {on:>12} | {off+on:>11} | {r:>13}")
_, r, off, on = AES_maestro_23_LUT_256()
print(f"AES128 MAESTRO (2,3) LUT-256             | {off:>13} | {on:>12} | {off+on:>11} | {r:>13}")
_, r, off, on = AES_maestro_33_LUT_256()
print(f"AES128 MAESTRO (3,3) LUT-256 (rounds)    | {off:>13} | {on:>12} | {off+on:>11} | {r:>13}")
_, r, off, on = AES_maestro_33_LUT_256(True, True)
print(f"AES128 MAESTRO (3,3) LUT-256 (comm)      | {off:>13} | {on:>12} | {off+on:>11} | {r:>13}")

print()

# Different 64bit PPAs (64 bit output, no carry-out), and versions improved by our optimizer
# MOTION's Sklansky Adder (https://github.com/encryptogroup/MOTION/blob/dev/circuits/int/int_add64_depth.stats):
MOTION_PPA64_CIRC_ANDS = 383
MOTION_PPA64_CIRC_DEPTH = 6
OPT_MOTION_PPA64_CIRC_ANDS = 249
OPT_MOTION_PPA64_CIRC_DEPTH = 6
# cryptoTools Sklansky Adder (https://github.com/ladnir/cryptoTools/blob/1a344ee4b7f4afaf39193eb413300ba17962b19e/cryptoTools/Circuit/BetaLibrary.cpp#L865)
#   as used by ABY3 (e.g., https://github.com/ladnir/aby3/blob/e52b1d441b9456ffe1315862f22edb8280d42efd/aby3/Circuit/CircuitLibrary.cpp#L260):
CRYPTOTOOLS_PPA64_CIRC_ANDS = 373
CRYPTOTOOLS_PPA64_CIRC_DEPTH = 7
OPT_CRYPTOTOOLS_PPA64_CIRC_ANDS = 243
OPT_CRYPTOTOOLS_PPA64_CIRC_DEPTH = 7
# "packed" Kogge-Stone adder (radix 2) from SecretFlow SPU (https://github.com/secretflow/spu/blob/4d470a2340846d84902c08c176f83c309fc579fb/src/libspu/mpc/ab_api.cc#L314)
#   used by default (https://github.com/secretflow/spu/blob/4d470a2340846d84902c08c176f83c309fc579fb/src/libspu/mpc/ab_api.cc#L438):
# ("packed" means that each PPA layer contains 64 operations, including operations on zeros for what is not needed, apparently used for good 64bit alignment)
SPU_PPA64_CIRC_ANDS = 832
SPU_PPA64_CIRC_DEPTH = 7
OPT_SPU_PPA64_CIRC_ANDS = 704
OPT_SPU_PPA64_CIRC_DEPTH = 7
# "Improved" Kogge-Stone adder (radix 2), as above, but removing all useless gates (0 AND 0, x XOR 0, and the "MSB
# column" as the overflow is discarded anyway):
IMPR_SPU_PPA64_CIRC_ANDS = 631
IMPR_SPU_PPA64_CIRC_DEPTH = 6
OPT_IMPR_SPU_PPA64_CIRC_ANDS = 569
OPT_IMPR_SPU_PPA64_CIRC_DEPTH = 6

print(f"Protocol                                 | offline comm. | online comm. | total comm. | online rounds")
print(f"RSS (PPA64 MOTION Sklansky)              |           --- | {MOTION_PPA64_CIRC_ANDS * 3:>12} | {MOTION_PPA64_CIRC_ANDS * 3:>11} | {MOTION_PPA64_CIRC_DEPTH:>13}")
print(f"Leveled RSS (PPA64 MOTION Sklansky)      |           --- | {OPT_MOTION_PPA64_CIRC_ANDS * 3:>12} | {OPT_MOTION_PPA64_CIRC_ANDS * 3:>11} | {OPT_MOTION_PPA64_CIRC_DEPTH:>13}")
r, off, on = alkaid(64)
print(f"ALKAID (custom PPA64)                    | {off:13.0f} | {on:12.0f} | {off+on:11.0f} | {r:>13}")
print(f"RSS (PPA64 cryptoTools Sklansky)         |           --- | {CRYPTOTOOLS_PPA64_CIRC_ANDS * 3:>12} | {CRYPTOTOOLS_PPA64_CIRC_ANDS * 3:>11} | {CRYPTOTOOLS_PPA64_CIRC_DEPTH:>13}")
print(f"Leveled RSS (PPA64 cryptoTools Sklansky) |           --- | {OPT_CRYPTOTOOLS_PPA64_CIRC_ANDS * 3:>12} | {OPT_CRYPTOTOOLS_PPA64_CIRC_ANDS * 3:>11} | {OPT_CRYPTOTOOLS_PPA64_CIRC_DEPTH:>13}")
print(f"RSS (PPA64 SPU Kogge-Stone)              |           --- | {SPU_PPA64_CIRC_ANDS * 3:>12} | {SPU_PPA64_CIRC_ANDS * 3:>11} | {SPU_PPA64_CIRC_DEPTH:>13}")
print(f"Leveled RSS (PPA64 SPU Kogge-Stone)      |           --- | {OPT_SPU_PPA64_CIRC_ANDS * 3:>12} | {OPT_SPU_PPA64_CIRC_ANDS * 3:>11} | {OPT_SPU_PPA64_CIRC_DEPTH:>13}")
print(f"RSS (PPA64 Improved Kogge-Stone)         |           --- | {IMPR_SPU_PPA64_CIRC_ANDS * 3:>12} | {IMPR_SPU_PPA64_CIRC_ANDS * 3:>11} | {IMPR_SPU_PPA64_CIRC_DEPTH:>13}")
print(f"Leveled RSS (PPA64 Improved Kogge-Stone) |           --- | {OPT_IMPR_SPU_PPA64_CIRC_ANDS * 3:>12} | {OPT_IMPR_SPU_PPA64_CIRC_ANDS * 3:>11} | {OPT_IMPR_SPU_PPA64_CIRC_DEPTH:>13}")

# Benchmark numbers from https://eprint.iacr.org/2025/2298.pdf Table IV
# Using values for batchsize 1000 for better packing to be closer to theoretical communication.
# We then divide by 1000.
# The authors informed us that the table reports average incoming + outgoing traffic per party.
# We measure total traffic, i.e., the sum over sent traffic per party. Hence, we multiply by 3/2.
# Also, the table reports communication in KB, so we multiply by 8000
ALKAID_BENCH_ABY3_COMM = 203.125 / 1000 * 3 / 2 * 8000
ALKAID_BENCH_ALKAID_ON_COMM = 140.625 / 1000 * 3 / 2 * 8000
ALKAID_BENCH_ALKAID_TOT_COMM = 197.265 / 1000 * 3 / 2 * 8000

print(f"ALKAID Table IV: ABY3                    |           --- | {ALKAID_BENCH_ABY3_COMM:12.0f} | {ALKAID_BENCH_ABY3_COMM:11.0f} |           ---")
print(f"ALKAID Table IV: ALKAID                  | {ALKAID_BENCH_ALKAID_TOT_COMM-ALKAID_BENCH_ALKAID_ON_COMM:13.0f} | {ALKAID_BENCH_ALKAID_ON_COMM:12.0f} | {ALKAID_BENCH_ALKAID_TOT_COMM:11.0f} |           ---")
