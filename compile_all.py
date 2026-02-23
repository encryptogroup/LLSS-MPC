from pathlib import Path
import subprocess
models = ["replicated","3shamir","10shamir","masked","weak"]
folders = ["RSS","Shamir/3","Shamir/10","Masked","Weak"]

for model, folder in zip(models,folders):
    path = Path('Circuits/')
    print("==="+model+"===")
    files = [f for f in path.iterdir() if f.is_file()]
    for file in files:
        print("Compiling: "+file.name)
        optimized_path = "OptimizedCircuits/"+folder+"/"+file.stem+".txt"
        file_path = Path(optimized_path)
        if file_path.is_file():
            print("Skipping")
        else:
            args = ["./ShareAssigner/build/DelayedResharing","Circuits/"+file.name,"BaselineCircuits/"+folder+"/"+file.stem+".txt",optimized_path,model]
            print(args)
            subprocess.run(args)
        print("\n")
