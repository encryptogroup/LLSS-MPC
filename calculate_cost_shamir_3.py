import os

def costs(ring_element):
	n = 3
	t = 2
	cost_upgrade = (2*n-t-1)*ring_element
	cost_input_share_low = 0*ring_element
	cost_input_share_high = (n-t)*ring_element
	cost_rerand = 0
	cost_output_reveal_low = (n-1)*ring_element
	cost_output_reveal_high = (t-1)*ring_element
	return cost_upgrade, cost_input_share_high,cost_input_share_low,cost_rerand,cost_output_reveal_low,cost_output_reveal_high

cost_upgrade, cost_input_share_high,cost_input_share_low,cost_rerand,cost_output_reveal_low,cost_output_reveal_high = costs(64)
# aes key cost
cost_static_setup = 3*128

baseline_cost = {}

def esimate_circuits(path,baseline):
	print("\n"+path)
	circuits = os.listdir(path)
	for circuit in circuits:
		estimated_cost = 0
		cost_upgrade, cost_input_share_high,cost_input_share_low,cost_rerand,cost_output_reveal_low,cost_output_reveal_high = costs(64)
		#estimated_cost += cost_static_setup
		with open(path+"/"+circuit, 'r') as file:
			print(circuit+": ",end="")
			for line in file:
				if ":1" in line:
					cost_upgrade, cost_input_share_high,cost_input_share_low,cost_rerand,cost_output_reveal_low,cost_output_reveal_high = costs(2)
				if line.startswith('OUTPUT'):
					if "Additive:Plain" in line:
						estimated_cost += cost_output_reveal_low + cost_rerand
					if "Blinded:Plain" in line:
						estimated_cost += cost_output_reveal_high
				if line.startswith('INPUT'):
					if "Plain:Additive" in line:
						estimated_cost += cost_input_share_low
					if "Plain:Blinded" in line:
						estimated_cost += cost_input_share_high
				if "Additive:Blinded" in line:
					estimated_cost += cost_upgrade
			print(estimated_cost)
			if baseline:
				baseline_cost[circuit] = estimated_cost
			else:
				print( f"{((1-estimated_cost/baseline_cost[circuit])*100):.3f}%")


esimate_circuits("BaselineCircuits/Shamir/3",True)
esimate_circuits("OptimizedCircuits/Shamir/3",False)