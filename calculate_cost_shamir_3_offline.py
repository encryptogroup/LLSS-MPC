import os

def costs(ring_element):
	n = 3
	t = 2
	cost_upgrade = (((t-1)*(n-t-1))/(n-t+1))*ring_element
	return cost_upgrade

cost_upgrade = costs(64)
# aes key cost

baseline_cost = {}

def esimate_circuits(path,baseline):
	print("\n"+path)
	circuits = os.listdir(path)
	for circuit in circuits:
		estimated_cost = 0
		cost_upgrade = costs(64)
		#estimated_cost += cost_static_setup
		with open(path+"/"+circuit, 'r') as file:
			print(circuit+": ",end="")
			for line in file:
				if ":1" in line:
					cost_upgrade = costs(2)
				if "Additive:Blinded" in line:
					estimated_cost += cost_upgrade
			print(estimated_cost)
			if baseline:
				baseline_cost[circuit] = estimated_cost
			else:
				if 0 == baseline_cost[circuit]:
					print("Cannot compute")
				else:
					print( f"{((1-estimated_cost/baseline_cost[circuit])*100):.3f}%")


esimate_circuits("BaselineCircuits/Shamir/3",True)
esimate_circuits("OptimizedCircuits/Shamir/3",False)