import csv
import os
import sys
import time
from pathlib import Path
import subprocess

networks = ["WAN", "LAN"]
folders = ["OptimizedCircuits", "BaselineCircuits"]

repetitions = int(sys.argv[1])
print(f"Repetitions: {repetitions}")
Path('runtimes_rss.csv').unlink(missing_ok=True)

with open('runtimes_rss.csv', 'w', newline='') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(['circuit', 'network', 'avg_run_time_seconds'])

    for network in networks:
        subprocess.run(
            ["python3", "network.py", "start", "3", network],
            text=True,
            check=True
        )

        for folder in folders:
            path = Path('../' + str(folder) + '/RSS')
            if not path.exists():
                continue

            files = [f for f in path.iterdir() if f.is_file()]

            for file in files:
                print(f"Benchmarking: {file}")
                circuit_rel_path = f"../{folder}/RSS/{file.name}"

                total_time = 0.0

                for _ in range(repetitions):
                    cmd0 = [
                        "ip", "netns", "exec", "neon_ns0",
                        "./build/DelayedresharingProtocol",
                        circuit_rel_path, "2", "0",
                        "172.16.1.11", "172.16.1.13", "1", "1"
                    ]
                    cmd1 = [
                        "ip", "netns", "exec", "neon_ns1",
                        "./build/DelayedresharingProtocol",
                        circuit_rel_path, "2", "1",
                        "172.16.1.12", "172.16.1.11", "1", "1"
                    ]
                    cmd2 = [
                        "ip", "netns", "exec", "neon_ns2",
                        "./build/DelayedresharingProtocol",
                        circuit_rel_path, "2", "2",
                        "172.16.1.13", "172.16.1.12", "1", "1"
                    ]

                    start_time = time.perf_counter()
                    p0 = subprocess.Popen(cmd0)

                    p1 = subprocess.Popen(cmd1)
                    p2 = subprocess.Popen(cmd2)

                    p0.wait()
                    elapsed = time.perf_counter() - start_time
                    total_time += elapsed

                    p1.wait()
                    p2.wait()
                    time.sleep(0.1)

                avg_time = total_time / repetitions
                writer.writerow([circuit_rel_path, network, f"{avg_time:.6f}"])
                csvfile.flush()

        subprocess.run(
            ["python3", "network.py", "stop", "3", network],
            text=True,
            check=True
        )
        time.sleep(0.1)

print("===Result===")

with open("runtimes_rss.csv") as f:
    reader = csv.reader(f)
    for row in reader:
        print(" ".join(row))