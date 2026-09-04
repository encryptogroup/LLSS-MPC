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
    # generate some zero
    zero = make_gate(lhs[0], lhs[0], "XOR")

    # Generate p & g.
    P = [make_gate(lhs[i],rhs[i], "XOR") for i in range(nbits)]
    G = [make_gate(lhs[i],rhs[i], "AND") for i in range(nbits)]

    for idx in range(math.ceil(math.log2(nbits))):
        offset = 1 << idx
        G1 = [zero for _ in range(offset)] + G # auto G1 = lshift_b(ctx, G, {offset});
        G1 = G1[0:nbits]
        P1 = [zero for _ in range(offset)] + P # auto P1 = lshift_b(ctx, P, {offset});
        P1 = P1[0:nbits]

        # P1 = P & P1
        # G1 = G ^ (P & G1)
        # std::vector<Value> res = spu::vmap(
        #     {P, P}, {P1, G1},
        #     [&](const Value& xx, const Value& yy) { return and_bb(ctx, xx, yy); });
        # P = std::move(res[0]);
        # G = xor_bb(ctx, G, res[1]);
        new_P = [make_gate(P[i],P1[i], "AND") for i in range(nbits)]
        temp = [make_gate(P[i],G1[i], "AND") for i in range(nbits)]
        P = new_P
        G = [make_gate(G[i],temp[i], "XOR") for i in range(nbits)]

    # out = (G << 1) ^ p0
    C = [zero] + G # auto C = lshift_b(ctx, G, {1});
    C = C[0:nbits]
    # return xor_bb(ctx, xor_bb(ctx, lhs, rhs), C);
    xor_lhs_rhs = [make_gate(lhs[i],rhs[i], "XOR") for i in range(nbits)]
    out = [make_gate(xor_lhs_rhs[i],C[i], "XOR") for i in range(nbits)]

make_adder(BITS, [x for x in range(BITS)], [BITS + x for x in range(BITS)])
print(str(len(GATES)) + " " + str(CURRENT_WIRE))
print(str(BITS) + " " + str(BITS) + " " + str(BITS))
print()
for g in GATES:
    print("2 1 " + str(g[0]) + " " + str(g[1]) + " " + str(g[2]) + " " + g[3])
