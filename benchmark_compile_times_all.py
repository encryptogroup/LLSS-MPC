import csv
import os
import time
from pathlib import Path
import subprocess
import re
import sys

models = ["replicated", "3shamir", "10shamir", "masked", "weak"]
folders = ["RSS", "Shamir/3", "Shamir/10", "Masked", "Weak"]
repetitions = int(sys.argv[1])

def get_peak_memory(args):
    cmd = ["/usr/bin/time", "-v"] + args
    result = subprocess.run(cmd, stderr=subprocess.PIPE, stdout=subprocess.PIPE, text=True)

    match = re.search(r"Maximum resident set size \(kbytes\): (\d+)", result.stderr)
    return int(match.group(1)) if match else 0

def get_circuit_size(baseline_path, original_file):
    if not os.path.exists(baseline_path):
        return 0
    with open(baseline_path, 'r', encoding='utf-8', errors='ignore') as f:
        line_count = sum(1 for _ in f)
    return line_count

with open('compiletimes.csv', 'w', newline='') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(['filename', 'model', 'circuit_size', 'avg_compile_time_seconds', 'avg_peak_memory_kb'])
    
    for model, folder in zip(models, folders):
        path = Path('Circuits/')
        print(f"==={model}===")
        files = [f for f in path.iterdir() if f.is_file()]
        
        for file in files:
            durations = []
            memories = []
            baseline_path = Path(f"BaselineCircuits/{folder}/{file.stem}.txt")
            
            for i in range(repetitions):
                print(f"Compiling: {file.name} with {model} (Repetition {i+1})")
                optimized_path = f"OptimizedCircuits/{folder}/{file.stem}.txt"
                if os.path.isfile(optimized_path):
                    os.remove(optimized_path)
                
                args = ["./ShareAssigner/build/DelayedResharing", str(path / file.name), 
                        str(baseline_path), optimized_path, model]

                start_time = time.perf_counter()
                peak_mem = get_peak_memory(args)
                end_time = time.perf_counter()
                
                durations.append(end_time - start_time)
                memories.append(peak_mem)
            
            circuit_size = get_circuit_size(baseline_path, file)
            avg_duration = sum(durations) / len(durations)
            avg_memory = sum(memories) / len(memories)
            
            writer.writerow([file.name, model, circuit_size, f"{avg_duration:.4f}", f"{avg_memory:.0f}"])
            csvfile.flush()
            print(f"Optimized Circuit: {file.name} | Size: {circuit_size} | Time: {avg_duration:.2f}s | Mem: {avg_memory:.0f} KB\n")

print("===Result===")
with open("compiletimes.csv") as f:
    reader = csv.reader(f)
    for row in reader:
        print(" ".join(row))
