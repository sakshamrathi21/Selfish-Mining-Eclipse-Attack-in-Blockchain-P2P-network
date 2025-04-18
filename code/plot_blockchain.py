import os
import graphviz
import shutil

def read_blockchain(file_path):
    """Reads the blockchain data and returns the edges as a list of tuples (parent, child)."""
    edges = []
    isMalicious = {}
    depth_block = {}
    with open(file_path, "r") as file:
        for line in file:
                block, parent, type, _, depth = line.strip().split('\t')
                edges.append((parent, block))
                depth_block[block] = int(depth)
                if type == 'Malicious':
                    isMalicious[block] = True
                else:
                    isMalicious[block] = False
    return edges, isMalicious, depth_block

def build_graph(edges):
    """Build a graph dictionary where each key is a parent node and the values are its children."""
    graph = {}
    for parent, child in edges:
        if parent not in graph:
            graph[parent] = []
        graph[parent].append(child)
    for parent in graph:
        graph[parent] = sorted(graph[parent]) # Sort children by block number
    return graph

def plot_blockchain_graphviz(edges, color, depth_map, output_file, max_depth=200):
    """Generates a tree visualization of the blockchain up to the given max depth."""
    # Create a new directed graph
    dot = graphviz.Digraph(format='png', engine='dot')
    dot.attr(rankdir='LR')
    dot.attr(dpi='300')

    # Build the graph dictionary
    graph = build_graph(edges)

    visited = set()

    def add_nodes_and_edges(node):
        """Recursive function to add nodes and edges up to a maximum depth."""
        if node not in depth_map or depth_map[node] > max_depth:
            return  # Stop recursion when max depth is reached
        
        if node in visited:
            return  # Stop recursion if the node has already been visited
        
        # Add the node to the graph
        visited.add(node)

        # Add edges to children if they exist
        if node in graph:
            for child in graph[node]:
                dot.node(child, label=child[:5], fillcolor=color[child], style='filled')
                dot.edge(node, child)
                add_nodes_and_edges(child)  # Recurse for the child

    # Start plotting from the root node
    if edges:
        start_node = "genesis"
        dot.node(start_node, fillcolor='lightblue', style='filled')
        add_nodes_and_edges(start_node)
        all_blocks = color.keys()
        all_blocks = sorted(list(all_blocks), key=lambda x: depth_map[x]) # Sort by depth
        for block in all_blocks:
            if block not in visited:
                if color[block] == 'red':
                    color[block] = 'pink'
                else:
                    color[block] = 'blue'
        for block in all_blocks:
            if block not in visited and depth_map[block] <= max_depth:
                dot.node(block, label=block[:5], fillcolor=color[block], style='filled')
                curr = "genesis"
                extra_block_count = 0
                for i in range(depth_map[block]):
                    if i == depth_map[block] - 1:
                        next = block
                    else:
                        next = "temp" + str(extra_block_count)
                        extra_block_count += 1
                        dot.node(next, label='', fillcolor='white', color = 'white', style='invis')
                    dot.edge(curr, next, style='invis')
                    curr = next
                add_nodes_and_edges(block)

    # Render and save the output to a PNG file
    dot.render(output_file, cleanup=True)

# Remove any previous blockchain graphs
shutil.rmtree("blockchain_graphs", ignore_errors=True)
os.makedirs("blockchain_graphs")

input_directory = "blockchain_data"
output_directory = "blockchain_graphs"

for file_name in os.listdir(input_directory):
    file_path = os.path.join(input_directory, file_name)
    output_file = os.path.join(output_directory, file_name.split(".")[0])

    edges, isMalicious, depth_map = read_blockchain(file_path)
    depth_map["genesis"] = 0
    color = {}
    for block in isMalicious:
        if isMalicious[block]:
            color[block] = 'red'
        else:
            color[block] = 'lightblue'
    plot_blockchain_graphviz(edges, color, depth_map, output_file)