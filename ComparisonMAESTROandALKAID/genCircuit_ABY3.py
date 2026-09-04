# Code based on the C++ implementation in cryptoTools.
# See LICENSE_cryptoTools for the original license.
# We make this derivative of their implementation available under the same conditions.

from math import log2, ceil

BITS = 64
CURRENT_WIRE = 2 * BITS
GATES = []

def make_gate(in1, in2, type):
    global CURRENT_WIRE
    global GATES
    GATES.append([in1, in2, CURRENT_WIRE, type, False])
    CURRENT_WIRE += 1
    return CURRENT_WIRE - 1

class Idx:
    def __init__(self):
        self.lvl = -1
        self.pos = -1

def ConstrIdx(lvl, pos):
    out = Idx()
    out.lvl = lvl
    out.pos = pos
    return out

class Node:
    def __init__(self):
        self.first = False
        self.used = False
        self.enqued = False
        self.curWire = Idx()
        self.lowWire = Idx()

def parallelPrefix_build(sSize):
    # initGate0 = GateType::Xor
    # initGate1 = GateType::And
    a1wires = [x for x in range(BITS)]
    a2wires = [BITS + x for x in range(BITS)]
    sumwires = [BITS + x for x in range(BITS)]

    P = [-1 for i in range(sSize)]
    G = [-1 for i in range(sSize)]

    for i in range(sSize):
        P[i] = make_gate(a1wires[i], a2wires[i], "XOR")
        if i < sSize - 1:
            G[i] = make_gate(a1wires[i], a2wires[i], "AND")

    # Sklansky algorithm
    d = ceil(log2(sSize))

    graph = [[Node() for _ in range(sSize)] for _ in range(d)]
    lvls = [-1 for _ in range(sSize)]

    for level in range(d):
        startPos = 1 << level
        step = 1 << (level + 1)

        first = True
        for i in range(startPos, sSize, step):
            lowWire = i - 1

            endPos = min(i + startPos, sSize)
            for curWire in range(i, endPos):
                graph[level][curWire].curWire.lvl = lvls[curWire]
                graph[level][curWire].curWire.pos = curWire
                graph[level][curWire].lowWire.lvl = lvls[lowWire]
                graph[level][curWire].lowWire.pos = lowWire
                graph[level][curWire].first = first
                lvls[curWire] = level
            first = False

    stack = []

    def add(idx: Idx):
        assert idx.pos != -1
        if idx.lvl != -1:
            c0 = graph[idx.lvl][idx.pos]
            if c0.enqued == False:
                assert c0.used == False
                c0.enqued = True
                stack.append(idx)

    for i in range(1, sSize):
        add(ConstrIdx(lvls[i - 1], i - 1))

    i = 0
    while i < len(stack):
        lvl = stack[i].lvl
        pos = stack[i].pos
        g = graph[lvl][pos]
        g.used = True

        if lvl:
            add(g.curWire)
            add(g.lowWire)
        i += 1

    for level in range(d):
        for i in range(sSize):
            g = graph[level][i]
            if g.used:
                P0 = P[g.lowWire.pos]
                G0 = G[g.lowWire.pos]
                P1 = P[g.curWire.pos]

                if g.curWire.pos < sSize - 1:
                    G1 = G[g.curWire.pos]

                    tempWire = make_gate(P1, G0, "AND")
                    G[g.curWire.pos] = make_gate(tempWire, G1, "XOR")
                if not g.first:
                    P[g.curWire.pos] = make_gate(P0, P1, "AND")

    # following is reordered
    for i in range(1, sSize):
        P[i] = make_gate(a1wires[i], a2wires[i], "XOR")
    sumwires[0] = make_gate(a1wires[0], a2wires[0], "XOR")
    for i in range(1, sSize):
        sumwires[i] = make_gate(P[i], G[i - 1], "XOR")

parallelPrefix_build(BITS)

print(str(len(GATES)) + " " + str(CURRENT_WIRE))
print(str(BITS) + " " + str(BITS) + " " + str(BITS))
print()
for g in GATES:
    print("2 1 " + str(g[0]) + " " + str(g[1]) + " " + str(g[2]) + " " + g[3])
