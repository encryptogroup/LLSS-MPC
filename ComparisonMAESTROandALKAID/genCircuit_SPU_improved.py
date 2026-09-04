# Code based on the C++ implementation in SPU.
# See LICENSE_SPU for the original license.
# We make this derivative of their implementation available under the same conditions.

import math

BITS = 64
CURRENT_WIRE = 2 * BITS
GATES = []

def make_gate(in1, in2, type):
    global CURRENT_WIRE
    global GATES
    GATES.append([in1, in2, CURRENT_WIRE, type, False])
    CURRENT_WIRE += 1
    return CURRENT_WIRE - 1

def make_adder(nbits, lhs, rhs):
    # Generate p & g.
    P = [-1 if i == 0 else make_gate(lhs[i],rhs[i], "XOR") for i in range(nbits)]
    lhs_xor_rhs = P
    G = [make_gate(lhs[i],rhs[i], "AND") for i in range(nbits - 1)]

    for idx in range(math.ceil(math.log2(nbits))):
        offset = 1 << idx
        G1 = [-1 for _ in range(offset)] + G # auto G1 = lshift_b(ctx, G, {offset});
        G1 = G1[0:nbits]
        P1 = [-1 for _ in range(offset)] + P # auto P1 = lshift_b(ctx, P, {offset});
        P1 = P1[0:nbits]

        # P1 = P & P1
        # G1 = G ^ (P & G1)
        # std::vector<Value> res = spu::vmap(
        #     {P, P}, {P1, G1},
        #     [&](const Value& xx, const Value& yy) { return and_bb(ctx, xx, yy); });
        # P = std::move(res[0]);
        # G = xor_bb(ctx, G, res[1]);
        # offset lowest ones: maintain prior P
        new_P = [-1 if i < 2 * offset else make_gate(P[i],P1[i], "AND") for i in range(nbits - 1)]
        # offset lowest ones: unused, encode as -1
        temp = [-1 if i < offset else make_gate(P[i],G1[i], "AND") for i in range(nbits - 1)]
        P = new_P
        # offset lowest ones: maintain prior G
        G = [G[i] if i < offset else make_gate(G[i],temp[i], "XOR") for i in range(nbits - 1)]

        # After the last iteration, we need G_0, ..., G_62
        # Hence, for the last iteration, we need G_0, ..., G_62, temp_32, ..., temp_62
        #                           ==> G_0, ..., G_62, P_32, ..., P_62
        # Hence, for the iteration before, we need G_0, ..., G_62, temp_16, ..., temp_62, P_16, ..., P_62
        #                           ==> G_0, ..., G_62, P_16, ..., P_62
        # Hence, for the iteration before, we need G_0, ..., G_62, temp_8, ..., temp_62, P_8, ..., P_62
        #                           ==> G_0, ..., G_62, P8, ..., P_62
        # ...
        # Hence, for the second iteration, we need G_0, ..., G_62, temp_2, ..., temp_62, P_2, ..., P_62
        #                           ==> G_0, ..., G_62, P2, ..., P_62
        # Hence, for the first iteration, we need G_0, ..., G_62, temp_1, ..., temp_62, P_1, ..., P_62
        #                           ==> G_0, ..., G_62, P1, ..., P_62

    # out = (G << 1) ^ p0
    # we keep zero here so we (seemingly unnecessarily) xor xor_lhs_rhs[i] with zero as a buffer.
    # This is needed as while we compute the value in the beginning, we need to refresh to one of
    # the last wire IDs so that it counts as part of the output.
    C = [-1] + G # auto C = lshift_b(ctx, G, {1});
    C = C[0:nbits]
    # return xor_bb(ctx, xor_bb(ctx, lhs, rhs), C);
    out = [make_gate(lhs[0],rhs[0], "XOR") if i == 0 else make_gate(lhs_xor_rhs[i],C[i], "XOR") for i in range(nbits)]

make_adder(BITS, [x for x in range(BITS)], [BITS + x for x in range(BITS)])
# 706 AND   631     631
# 514 XOR   506     442

print(str(len(GATES)) + " " + str(CURRENT_WIRE))
print(str(BITS) + " " + str(BITS) + " " + str(BITS))
print()
for g in GATES:
    print("2 1 " + str(g[0]) + " " + str(g[1]) + " " + str(g[2]) + " " + g[3])
