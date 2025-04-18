import sys
import subprocess
import numpy as np
import json
import matplotlib.pyplot as plt


def run_simulation(num_peer=100, percent_malicious=50, Ttx=10, Tk=100, get_timeout=20, total_time=17500, iterations=3):
    print(f"Running simulation with num_peer={num_peer}, percent_malicious={percent_malicious}, Ttx={Ttx}, Tk={Tk}, get_timeout={get_timeout}, total_time={total_time}")
    outputs = [subprocess.run(["./run", str(num_peer), str(percent_malicious), str(Ttx), str(Tk), str(get_timeout), str(total_time), "--ratio"], 
                              capture_output=True, text=True).stdout for _ in range(iterations)]
    print(outputs)
    
    def extract_value(output, key):
        for line in output.splitlines():
            # print(line)
            if key in line:
                value = line.split(':')[1]
                value = value[1:]
                return np.nan if value == "-nan" else float(value)
        return np.nan  # If the key is not found, return NaN
    
    categories = ["Malicious Blocks in Chain / Total Blocks in Longest Chain", "Malicious Blocks in Chain / Total Malicious Blocks"]
    results = {}
    
    for category in categories:
        values = [extract_value(output, category) for output in outputs]
        filtered_values = [v for v in values if not np.isnan(v)]
        print(f"Values for {category}: {values}")
        results[category] = np.mean(filtered_values) if filtered_values else np.nan
    
    return results

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 script.py <parameter>")
        print("Parameters: num_peers, percent_malicious, Ttx, Tk, get_timeout, total_time")
        sys.exit(1)
    
    parameter = sys.argv[1]
    param_values = {
        "num_peers": range(50, 101, 10),
        "percent_malicious": range(10, 101, 15),
        "Ttx": range(10, 101, 10),
        "Tk": range(100, 1001, 100),
        "get_timeout": [10, 20, 50, 100, 200, 400],
        "total_time": range(10000, 30001, 5000)
    }
    
    if parameter not in param_values:
        print("Invalid parameter.")
        sys.exit(1)
    
    results_dict = {"Malicious Blocks in Chain / Total Blocks in Longest Chain": [],
                    "Malicious Blocks in Chain / Total Malicious Blocks": []}
    
    for val in param_values[parameter]:
        if parameter == "num_peers":
            results = run_simulation(num_peer=val)
        elif parameter == "percent_malicious":
            results = run_simulation(percent_malicious=val)
        elif parameter == "Ttx":
            results = run_simulation(Ttx=val)
        elif parameter == "Tk":
            results = run_simulation(Tk=val)
        elif parameter == "get_timeout":
            results = run_simulation(get_timeout=val)
        elif parameter == "total_time":
            results = run_simulation(total_time=val)
        for key in results_dict:
            results_dict[key].append(results[key])
    
    for key, values in results_dict.items():
        print(f"{key}: {values}")
    
    json_filename = f"simulation_results_{parameter}.json"
    with open(json_filename, "w") as json_file:
        json.dump(results_dict, json_file, indent=4)
    print(f"Results saved to {json_filename}")

    x_axis = param_values[parameter]

    for key, values in results_dict.items():
        plt.plot(x_axis, values, label=key)

    plt.xlabel(f"Parameter: {parameter}")
    plt.ylabel('Block Ratio')
    plt.title('')
    plt.legend()
    plt.show()
    

if __name__ == "__main__":
    main()
