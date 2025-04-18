import sys
import subprocess
import numpy as np
import json
import matplotlib.pyplot as plt
from itertools import product
import os

def run_simulation(num_peer=100, percent_malicious=50, Ttx=10, Tk=100, get_timeout=20, total_time=17500, iterations=3):
    print(f"Running simulation with num_peer={num_peer}, percent_malicious={percent_malicious}, Ttx={Ttx}, Tk={Tk}, get_timeout={get_timeout}, total_time={total_time}", flush=True)

    output_filename = f"ringmaster_{num_peer}_{percent_malicious}_{Ttx}_{Tk}_{get_timeout}_{total_time}.png"

    if os.path.exists(output_filename):
        print("Already calculated. Skipping...")
        return
    outputs = [subprocess.run(["./run", str(num_peer), str(percent_malicious), str(Ttx), str(Tk), str(get_timeout), str(total_time), "--ratio", "--blockchain"], 
                              capture_output=True, text=True).stdout for _ in range(iterations)]
    
    
    
    os.system(f"mv blockchain_graphs/ringmaster.png {output_filename}")

    
    def extract_value(output, key):
        for line in output.splitlines():
            if key in line:
                value = line.split(':')[1].strip()
                return np.nan if value == "-nan" else float(value)
        return np.nan  
    
    categories = ["Malicious Blocks in Chain / Total Blocks in Longest Chain", "Malicious Blocks in Chain / Total Malicious Blocks"]
    results = {}
    
    for category in categories:
        values = [extract_value(output, category) for output in outputs]
        filtered_values = [v for v in values if not np.isnan(v)]
        results[category] = np.mean(filtered_values) if filtered_values else np.nan
    
    return results

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 script.py <parameter1> [parameter2]")
        print("Parameters: num_peers, percent_malicious, Ttx, Tk, get_timeout, total_time")
        sys.exit(1)
    
    param1 = sys.argv[1]
    param2 = sys.argv[2] if len(sys.argv) > 2 else None
    
    param_values = {
        "num_peers": range(50, 101, 10),
        "percent_malicious": range(5, 101, 5),
        "Ttx": range(10, 101, 10),
        "Tk": range(100, 1001, 100),
        "get_timeout": [10, 20],
        "total_time": range(10000, 30001, 5000)
    }
    
    if param1 not in param_values or (param2 and param2 not in param_values):
        print("Invalid parameter(s).")
        sys.exit(1)
    
    param1_values = param_values[param1]
    param2_values = param_values[param2] if param2 else [None]
    
    results_dict = {}
    
    for val1, val2 in product(param1_values, param2_values):
        sim_params = {param1: val1}
        if param2:
            sim_params[param2] = val2
        
        results = run_simulation(**sim_params)
        results_dict[(val1, val2)] = results
        print(results, flush=True)
    
    json_filename = f"simulation_results_{param1}_{param2 if param2 else ''}.json"
    with open(json_filename, "w") as json_file:
        json.dump(results_dict, json_file, indent=4)
    print(f"Results saved to {json_filename}")
    
    if not param2:
        x_axis = param1_values
        for key in ["Malicious Blocks in Chain / Total Blocks in Longest Chain", "Malicious Blocks in Chain / Total Malicious Blocks"]:
            values = [results_dict[(val, None)][key] for val in param1_values]
            plt.plot(x_axis, values, label=key)
        
        plt.xlabel(f"Parameter: {param1}")
        plt.ylabel('Block Ratio')
        plt.legend()
        plt.show()

if __name__ == "__main__":
    main()
