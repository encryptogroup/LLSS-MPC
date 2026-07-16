import csv
import os
import time
from pathlib import Path
import subprocess
import re

models = ["replicated","3shamir","10shamir","masked","weak"]
folders = ["RSS","Shamir/3","Shamir/10","Masked","Weak"]
repetitions = 5

def get_peak_memory(args):
    cmd = ["/usr/bin/time", "-v"] + args
    result = subprocess.run(cmd, stderr=subprocess.PIPE, stdout=subprocess.PIPE, text=True)

    match = re.search(r"Maximum resident set size \(kbytes\): (\d+)", result.stderr)
    return int(match.group(1)) if match else 0

with open('compiletimes.csv', 'w', newline='') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(['filename', 'model', 'avg_compile_time_seconds', 'max_peak_memory_kb'])
    
    for model, folder in zip(models, folders):
        path = Path('Circuits/')
        print(f"==={model}===")
        files = [f for f in path.iterdir() if f.is_file()]
        
        for file in files:
            durations = []
            memories = []
            
            for i in range(repetitions):
                print(f"Compiling: {file.name} with {model} (Attempt {i+1})")
                optimized_path = f"OptimizedCircuits/{folder}/{file.stem}.txt"
                if os.path.isfile(optimized_path):
                    os.remove(optimized_path)
                
                args = ["./ShareAssigner/build/DelayedResharing", str(path / file.name), 
                        f"BaselineCircuits/{folder}/{file.stem}.txt", optimized_path, model]

                start_time = time.perf_counter()
                peak_mem = get_peak_memory(args)
                end_time = time.perf_counter()
                
                durations.append(end_time - start_time)
                memories.append(peak_mem)
            
            avg_duration = sum(durations) / len(durations)
            avg_memory = max(memories)
            
            writer.writerow([file.name, model, f"{avg_duration:.4f}", f"{avg_memory:.0f}"])
            csvfile.flush()
            print(f"Done: {file.name} | Time: {avg_duration:.2f}s | Mem: {avg_memory:.0f} KB\n")