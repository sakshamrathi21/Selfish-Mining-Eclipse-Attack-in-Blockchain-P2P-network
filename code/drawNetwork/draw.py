import networkx as nx
import matplotlib.pyplot as plt
import sys

def read_graph_from_file(file_name):
    G = nx.Graph()
    with open(file_name, 'r') as f:
        for line in f:
            nodes = line.strip().split()
            if len(nodes) == 2:
                u, v = int(nodes[0]), int(nodes[1])
                G.add_edge(u, v)
    return G

def read_malicious_peers(file_name):
    malicious_peers = set()
    with open(file_name, 'r') as f:
        for line in f:
            malicious_peers.add(int(line.strip()))
    return malicious_peers

def draw_graph(file_name):
    G = read_graph_from_file(file_name)
    malicious_peers = read_malicious_peers('drawNetwork/malicious_peers.txt') 

    plt.figure(figsize=(15, 10))  
    pos = nx.spring_layout(G)  
    node_colors = ['orange' if node in malicious_peers else 'lightblue' for node in G.nodes()]
    nx.draw(G, pos, with_labels=True, node_color=node_colors, edge_color='gray', node_size=200, font_size=8)
    plt.savefig(f"{file_name[:-4]}.png")
    plt.clf()

draw_graph('drawNetwork/normal_network.txt')
draw_graph('drawNetwork/overlay_network.txt')
