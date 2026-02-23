import os

baselinec = {}
def esimate_circuits(path,baseline):
	print("\n"+path)
	circuits = os.listdir(path)
	for circuit in circuits:
		upgrades = 0
		with open(path+"/"+circuit, 'r') as file:
			print(circuit+": ",end="")
			for line in file:
				if "Additive:Blinded" in line:
					upgrades += 1
			print(upgrades)
			if baseline:
				baselinec[circuit] = upgrades
			else:
				print( f"{((1-upgrades/baselinec[circuit])*100):.3f}%")


esimate_circuits("BaselineCircuits/RSS",True)
esimate_circuits("OptimizedCircuits/Weak",False)
