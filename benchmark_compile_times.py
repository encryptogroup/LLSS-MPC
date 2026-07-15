import csv
import os
import time
from pathlib import Path
import subprocess
models = ["replicated","3shamir","10shamir","masked","weak"]
folders = ["RSS","Shamir/3","Shamir/10","Masked","Weak"]
repetitions = 5

# Open CSV file to append data
with open('compiletimes.csv', 'w', newline='') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(['filename', 'model', 'compile_time_seconds'])
    csvfile.flush()
    for model, folder in zip(models,folders):
        path = Path('Circuits/')
        print("==="+model+"===")
        files = [f for f in path.iterdir() if f.is_file()]
        for file in files:
            times = []
            for i in range(repetitions):
                    print("Compiling: "+file.name)
                    optimized_path = "OptimizedCircuits/"+folder+"/"+file.stem+".txt"
                    if os.path.isfile(optimized_path):
                        os.remove(optimized_path)
                    file_path = Path(optimized_path)
                    args = ["./ShareAssigner/build/DelayedResharing","Circuits/"+file.name,"BaselineCircuits/"+folder+"/"+file.stem+".txt",optimized_path,model]
                    print(args)
                    start_time = time.perf_counter()
                    subprocess.run(args)
                    end_time = time.perf_counter()
                    duration = end_time - start_time
                    times.append(duration)
            duration = sum(times) / len(times)
            writer.writerow([file, model, f"{duration:.4f}"])
            csvfile.flush()
            print("\n")
