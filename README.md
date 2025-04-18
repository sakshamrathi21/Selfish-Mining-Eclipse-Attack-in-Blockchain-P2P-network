# Analyzing Selfish Mining + Eclipse Attack in Blockchain P2P network
This project has been done as part of the course CS765 - Introduction to Blockchain, Cryptocurrency and Smart Contracts. Here are the team members:
- Saksham Rathi (22B1003)
- Kavya Gupta (22B1053)
- Mayank Kumar (22B0933)

Here are list of files/directories present in this repository:

```
22B1003_22B1053_22B0933
|-- README.md
|-- code
    |-- Makefile
    |-- cpp files
    |-- header files
    |-- blockchain_data
    |-- blockchain_graphs
    |-- python scripts
|-- documentation
    |-- images
    |-- report.pdf
```
The images folder can be downloaded from this link (https://drive.google.com/drive/folders/10gC7ju0IK-e4fHM7VMIjRoYMziE8xiP_?usp=sharing).
## Instructions on compiling and running the code
```
cd code
make clean
make
```
This will produce an executable named "run".

Here is the list of command line arguments which the executable expects:
```
./run <number-of-nodes> <percentage-of-malicious-nodes> <mean-Time-for-transactions> <average-Block-Arrival-Time> <get-Request-Timeout> <time-of-execution> <flags>
```

Here is an example usage:
```
./run 100 50 10 100 20 17500
```

It is worth noting that the time of execution is measured in terms of the events of the discrete simulator, and is not connected to the real time.

## Flags which can be passed:
- --ratio: to calculate the two ratios mentioned in the problem statement
- --blockchain: to plot the blockchain at the ringmaster node
- --show-network: to plot the normal and overlay network
- --countermeasure: run the simulation along with the countermeasure
- --dump-all: to plot the blockchains at all the nodes
- --no-eclipse: perform only selfish mining attack