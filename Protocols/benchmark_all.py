import csv
import os
import sys
import time
import re
from pathlib import Path
import subprocess
from collections import defaultdict

networks = ["WAN", "LAN"]
folders = ["OptimizedCircuits", "BaselineCircuits"]

repetitions = int(sys.argv[1])
print(f"Repetitions: {repetitions}")
Path('runtimes_rss.csv').unlink(missing_ok=True)


def cleanup_processes():
    binary_names = ["DelayedresharingProtocol", "DelayedresharingProtocolLowBatch"]
    for binary in binary_names:
        subprocess.run(["pkill", "-9", "-f", binary], stderr=subprocess.DEVNULL)
    time.sleep(0.5)

def cleanup_network(network):
    try:
        subprocess.run(
            ["python3", "network.py", "stop", "3", network],
            text=True,
            check=True,
            capture_output=True
        )
    except subprocess.CalledProcessError as e:
        print(f"Network cleanup failed (exit code {e.returncode}):\n{e.stderr}")
    except FileNotFoundError:
        print("Error: 'python3' executable not found.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

def prepare_network(network):
    try:
        subprocess.run(
            ["python3", "network.py", "start", "3", network],
            text=True,
            check=True
        )
    except subprocess.CalledProcessError as e:
        print(f"Network preparation failed with exit code {e.returncode}.")
    except FileNotFoundError:
        print("Error: 'python3' executable not found.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

with open('runtimes_rss.csv', 'w', newline='') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(['circuit', 'network', 'avg_run_time_seconds'])

    for network in networks:
        cleanup_network()
        time.sleep(1)
        print("Entering Network Setting: "+str(network))
        prepare_network(network)

        for folder in folders:
            path = Path('../' + str(folder) + '/RSS')
            if not path.exists():
                continue

            files = [f for f in path.iterdir() if f.is_file()]

            for file in files:
                print(f"Benchmarking: {file}")
                circuit_rel_path = f"../{folder}/RSS/{file.name}"
                
                program_name = "./build/DelayedresharingProtocol"
                ring_size = "1"
                if file.name in ["NN.txt", "mse.txt"]:
                    program_name = "./build/DelayedresharingProtocolLowBatch"
                    ring_size = "2"

                # Regex pattern to extract the runtime value in ms
                runtime_pattern = re.compile(r"Party 0 Average runtime:\s*([\d.]+)\s*ms")

                repetition_in_command = False
                command_reps = "1"
                if repetition_in_command:
                    command_reps = str(repetitions)
                cmd0 = [
                    "ip",
                    "netns",
                    "exec",
                    "neon_ns0",
                    program_name,
                    circuit_rel_path,
                    "2",
                    "0",
                    "172.16.1.11",
                    "172.16.1.13",
                    ring_size,
                    command_reps,
                ]
                cmd1 = [
                    "ip",
                    "netns",
                    "exec",
                    "neon_ns1",
                    program_name,
                    circuit_rel_path,
                    "2",
                    "1",
                    "172.16.1.12",
                    "172.16.1.11",
                    ring_size,
                    command_reps,
                ]
                cmd2 = [
                    "ip",
                    "netns",
                    "exec",
                    "neon_ns2",
                    program_name,
                    circuit_rel_path,
                    "2",
                    "2",
                    "172.16.1.13",
                    "172.16.1.12",
                    ring_size,
                    command_reps,
                ]
                
                total = 0
                if repetition_in_command:
                    cleanup_processes()
                    p0 = subprocess.Popen(cmd0, stdout=subprocess.PIPE, text=True)
                    p1 = subprocess.Popen(cmd1, stdout=subprocess.DEVNULL, text=True)
                    p2 = subprocess.Popen(cmd2, stdout=subprocess.DEVNULL, text=True)

                    p0_stdout, _ = p0.communicate()
                    p1.wait()
                    p2.wait()
                    match = runtime_pattern.search(p0_stdout)
                    time_seconds = float(match.group(1)) / 1000
                    avg_time_seconds = time_seconds
                else:
                    total_time = 0
                    for i in range(repetitions):
                        cleanup_processes()
                        p0 = subprocess.Popen(cmd0, stdout=subprocess.PIPE, text=True)
                        p1 = subprocess.Popen(cmd1, stdout=subprocess.DEVNULL, text=True)
                        p2 = subprocess.Popen(cmd2, stdout=subprocess.DEVNULL, text=True)

                        p0_stdout, _ = p0.communicate()
                        p1.wait()
                        p2.wait()
                        match = runtime_pattern.search(p0_stdout)
                        time_seconds = float(match.group(1)) / 1000
                        total_time += time_seconds
                        print("Time of run: "+str(time_seconds))
                        time.sleep(1)
                    avg_time_seconds = total_time / repetitions
                    





                print("Average in seconds "+str(avg_time_seconds))

                time.sleep(1)


                writer.writerow([circuit_rel_path, network, f"{avg_time_seconds:.6f}"])
                csvfile.flush()

        cleanup_network()
        time.sleep(1)

print("===Result===")

results = defaultdict(dict)

with open("runtimes_rss.csv") as f:
    reader = csv.reader(f)
    header = next(reader, None)
    if header:
        print(" ".join(header))
        
    for row in reader:
        if not row:
            continue
        print(" ".join(row))
        
        circuit_path, network, avg_time_str = row
        circuit_filename = Path(circuit_path).name
        avg_time = float(avg_time_str)
        
        if "BaselineCircuits" in circuit_path:
            results[(network, circuit_filename)]["baseline"] = avg_time
        elif "OptimizedCircuits" in circuit_path:
            results[(network, circuit_filename)]["optimized"] = avg_time

print("=== Improvement Ratios ===")

for (network, circuit_filename), times in sorted(results.items()):
    base = times.get("baseline")
    opt = times.get("optimized")
    
    speedup = base / opt
    pct_reduction = ((base - opt) / base) * 100
    print(f"{network} {circuit_filename} {pct_reduction}%")
