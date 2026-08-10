import csv
import os
import time
from pathlib import Path
import subprocess
import re
import sys

networks = ["WAN","LAN"]
folders = ["OptimizedCircuits","BaselineCircuits"]
repetitions = int(sys.argv[1])

if os.path.exists('runtimes_rss.csv'):
    os.remove('runtimes_rss.csv')

with open('runtimes_rss.csv', 'w', newline='') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(['circuit', 'network', 'avg_run_time'])

    for network in networks:
        result = subprocess.run(
        ["python3", "network.py", "start", "3", network],
        text=True,
        check=True
        )
        for folder in folders:
            path = Path('../'+str(folder)+'/RSS')
            files = [f for f in path.iterdir() if f.is_file()]

            for file in files:
                print(file)
                writer.writerow([file, network, "lol"])
                csvfile.flush()


        result = subprocess.run(
        ["python3", "network.py", "stop", "3", network],
        text=True,
        check=True
        )
        sleep(0.1)

print("===Result===")

with open("runtimes_rss.csv") as f:
    reader = csv.reader(f)
    for row in reader:
        print(" ".join(row))
