import os

def costs(ring_element):
	cost_mult = 2*ring_element*(1+ring_element+0.118)
	return cost_mult

cost_mult = costs(64)

baseline_cost = {}

def esimate_circuits(path,baseline):
	print("\n"+path)
	circuits = os.listdir(path)
	for circuit in circuits:
		estimated_cost = 0
		cost_mult = costs(64)
		#estimated_cost += cost_static_setup
		with open(path+"/"+circuit, 'r') as file:
			print(circuit+": ",end="")
			for line in file:
				if ":1" in line:
					cost_mult = costs(1)
				if line.startswith('*') and ("Blinded:Additive" in line):
					estimated_cost = estimated_cost + cost_mult
			print(estimated_cost)
			if baseline:
				baseline_cost[circuit] = estimated_cost
			else:
				print( f"{((1-estimated_cost/baseline_cost[circuit])*100):.3f}%")


esimate_circuits("BaselineCircuits/Masked",True)
esimate_circuits("OptimizedCircuits/Masked",False)

