import re
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns

def parse_simulation_data(filename):
    data = {}
    
    with open(filename, 'r') as file:
        content = file.readlines()
    
    percent_malicious, get_timeout = None, None
    
    for line in content:
        match = re.search(r'percent_malicious=(\d+), .* get_timeout=(\d+)', line)
        if match:
            percent_malicious = int(match.group(1))
            get_timeout = int(match.group(2))
        
        if line.startswith("{"):
            try:
                ratios = eval(line.strip())
                data[(percent_malicious, get_timeout)] = (
                    ratios['Malicious Blocks in Chain / Total Blocks in Longest Chain'],
                    ratios['Malicious Blocks in Chain / Total Malicious Blocks']
                )
            except:
                continue
    
    return data

def plot_heatmaps(data):
    percent_malicious_vals = sorted(set(k[0] for k in data.keys()))
    get_timeout_vals = sorted(set(k[1] for k in data.keys()))
    
    ratio1_matrix = np.zeros((len(percent_malicious_vals), len(get_timeout_vals)))
    ratio2_matrix = np.zeros((len(percent_malicious_vals), len(get_timeout_vals)))
    
    for i, pm in enumerate(percent_malicious_vals):
        for j, gt in enumerate(get_timeout_vals):
            ratio1_matrix[i, j], ratio2_matrix[i, j] = data.get((pm, gt), (0, 0))
    
    fig, axes = plt.subplots(1, 2, figsize=(12, 6))
    sns.heatmap(ratio1_matrix, annot=True, xticklabels=get_timeout_vals, yticklabels=percent_malicious_vals, ax=axes[0], cmap="coolwarm")
    axes[0].set_title("Malicious Blocks / Total Blocks in Longest Chain")
    axes[0].set_xlabel("get_timeout")
    axes[0].set_ylabel("percent_malicious")
    
    sns.heatmap(ratio2_matrix, annot=True, xticklabels=get_timeout_vals, yticklabels=percent_malicious_vals, ax=axes[1], cmap="coolwarm")
    axes[1].set_title("Malicious Blocks / Total Malicious Blocks")
    axes[1].set_xlabel("get_timeout")
    axes[1].set_ylabel("percent_malicious")
    
    plt.tight_layout()
    plt.savefig("heatmaps_eclipse.png")
    plt.close()

def plot_across_percent_malicious(data):
    get_timeout_vals = sorted(set(k[1] for k in data.keys()))
    percent_malicious_vals = sorted(set(k[0] for k in data.keys()))
    
    for gt in get_timeout_vals:
        ratio1 = [data.get((pm, gt), (0, 0))[0] for pm in percent_malicious_vals]
        ratio2 = [data.get((pm, gt), (0, 0))[1] for pm in percent_malicious_vals]
        
        plt.figure()
        plt.plot(percent_malicious_vals, ratio1, marker='o', label='Malicious Blocks / Total Blocks in Longest Chain')
        plt.plot(percent_malicious_vals, ratio2, marker='s', label='Malicious Blocks / Total Malicious Blocks')
        plt.xlabel("percent_malicious")
        plt.ylabel("Ratio")
        plt.title(f"Ratios across percent_malicious for get_timeout={gt}")
        plt.legend()
        plt.grid()
        plt.savefig(f"percent_malicious_vs_ratios_get_timeout_{gt}_eclipse.png")
        plt.close()

def plot_across_get_timeout(data):
    percent_malicious_vals = sorted(set(k[0] for k in data.keys()))
    get_timeout_vals = sorted(set(k[1] for k in data.keys()))
    
    for pm in percent_malicious_vals:
        ratio1 = [data.get((pm, gt), (0, 0))[0] for gt in get_timeout_vals]
        ratio2 = [data.get((pm, gt), (0, 0))[1] for gt in get_timeout_vals]
        
        plt.figure()
        plt.plot(get_timeout_vals, ratio1, marker='o', label='Malicious Blocks / Total Blocks in Longest Chain')
        plt.plot(get_timeout_vals, ratio2, marker='s', label='Malicious Blocks / Total Malicious Blocks')
        plt.xlabel("get_timeout")
        plt.ylabel("Ratio")
        plt.title(f"Ratios across get_timeout for percent_malicious={pm}")
        plt.legend()
        plt.grid()
        plt.savefig(f"get_timeout_vs_ratios_percent_malicious_{pm}_eclipse.png")
        plt.close()

filename = "trym"
result = parse_simulation_data(filename)
plot_heatmaps(result)
plot_across_percent_malicious(result)
plot_across_get_timeout(result)