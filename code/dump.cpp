// void longChainHeight() {
//     for (int i = 0 ; i < num_nodes ; i ++ ) {
//         cout << peers[i]->blockchain->getLongestChainHeight() << endl;
//     }
// }

// int ratio_cal () {
//     double* num_blocks_per_peer_in_blockchain = new double[num_nodes]();
//     double* total_blocks_produced_per_peer = new double[num_nodes]();
//     double* ratio_of_blocks_per_peer = new double[num_nodes](); 
//     vector<Block> chain = peers[0]->blockchain->currentChain();
//     for (Block b : chain) {
//         num_blocks_per_peer_in_blockchain[b.minerID] += 1;
//     }
//     for (auto b : peers[0]->blockchain->blocks) {
//         total_blocks_produced_per_peer[b.second.minerID] += 1;
//     }
//     for (int i = 0 ; i < num_nodes ; i ++ ) {
//         ratio_of_blocks_per_peer[i] = num_blocks_per_peer_in_blockchain[i] / total_blocks_produced_per_peer[i];
//     }
//     vector<vector<double>> average_ratio(2, vector<double>(2, 0));
//     int count[2][2] = {{0, 0}, {0, 0}};
//     for (int i = 0 ; i < num_nodes ; i ++ ) {
//         if (isnan(ratio_of_blocks_per_peer[i])) {
//             continue;
//         }
//         average_ratio[peers[i]->isSlow][peers[i]->isLowCPU] += ratio_of_blocks_per_peer[i];
//         count[peers[i]->isSlow][peers[i]->isLowCPU] += 1;
//     }
//     for (int i = 0 ; i < 2 ; i ++ ) {
//         for (int j = 0 ; j < 2 ; j ++ ) {
//             average_ratio[i][j] /= count[i][j];
//         }
//     }
//     ofstream file(RatioFile, ios::trunc); // Save all to ratio.txt
//     for (int i = 0 ; i < 2 ; i ++ ) {
//         for (int j = 0 ; j < 2 ; j ++ ) {
//             file << (i == 0 ? "Fast" : "Slow") << " " << (j == 0 ? "High CPU" : "Low CPU") << " :-\n";
//             for(int k = 0; k < num_nodes; k++) {
//                 if(peers[k]->isSlow == i && peers[k]->isLowCPU == j) {
//                     file << "Peer " << k << " : " << ratio_of_blocks_per_peer[k] << "\n";
//                 }
//             }
//             file << "------------------------\n";
//         }
//     }
//     file << "Average Ratios :-\n";
//     file << "Fast High CPU : " << average_ratio[0][0] << "\n";
//     file << "Fast Low CPU : " << average_ratio[0][1] << "\n";
//     file << "Slow High CPU : " << average_ratio[1][0] << "\n";
//     file << "Slow Low CPU : " << average_ratio[1][1] << "\n";


//     // for plotting
//     cout << "Fast High CPU : " << average_ratio[0][0] << "\n";
//     cout << "Fast Low CPU : " << average_ratio[0][1] << "\n";
//     cout << "Slow High CPU : " << average_ratio[1][0] << "\n";
//     cout << "Slow Low CPU : " << average_ratio[1][1] << "\n";
//     file.close();
//     return 1;
// }

// void blockchain_print() {
//     system(("rm -rf " + BlockChainSaveDirectory).c_str()); // Remove all previous data
//     system(("mkdir " + BlockChainSaveDirectory).c_str()); // Making a new directory
//     int fast_high_cpu, fast_low_cpu, slow_high_cpu, slow_low_cpu; // Storing node of each type
//     fast_high_cpu = fast_low_cpu = slow_high_cpu = slow_low_cpu = -1;
//     for (int i = 0 ; i < num_nodes ; i ++ ) {
//         if (peers[i]->isSlow && peers[i]->isLowCPU) {
//             slow_low_cpu = i;
//         } else if (peers[i]->isSlow && !peers[i]->isLowCPU) {
//             slow_high_cpu = i;
//         } else if (!peers[i]->isSlow && peers[i]->isLowCPU) {
//             fast_low_cpu = i;
//         } else {
//             fast_high_cpu = i;
//         }
//     }
//     if(slow_low_cpu != -1) {
//         peers[slow_low_cpu]->blockchain->saveBlockChain(BlockChainSaveDirectory + "slow_low_cpu.txt");
//     }
//     if(slow_high_cpu != -1) {
//         peers[slow_high_cpu]->blockchain->saveBlockChain(BlockChainSaveDirectory + "slow_high_cpu.txt");
//     }
//     if(fast_low_cpu != -1) {
//         peers[fast_low_cpu]->blockchain->saveBlockChain(BlockChainSaveDirectory + "fast_low_cpu.txt");
//     }
//     if(fast_high_cpu != -1) {
//         peers[fast_high_cpu]->blockchain->saveBlockChain(BlockChainSaveDirectory + "fast_high_cpu.txt");
//     }
// }


// void handleTimeout(int signum) {
//     // Whenever the time is up, we come to this function
//     if (whether_longest_chain_height) {
//         longChainHeight();
//     }

//     if (whether_ratio) {
//         ratio_cal();
//     }

//     if (whether_blockchain) {
//         blockchain_print();
//     }

//     if (whether_branches) {
//         vector<int> branch_heights;
//         for (const auto& b : peers[0]->blockchain->leafBlocks) {
//             branch_heights.push_back(peers[0]->blockchain->blocks[b].height);
//         }
//         sort(branch_heights.begin(), branch_heights.end());
//         cout << "Branch Heights:- ";
//         for (int i = 0; i < branch_heights.size(); i++) {
//             cout << branch_heights[i] << " ";
//         }
//         cout << endl;
//     }

//     for(int i = 0; i < num_nodes; i++) {
//         delete peers[i]; // Delete the peer objects
//     }
    
//     cout << "Execution Finished. " << endl; // Printing the message when we are done
//     exit(0); // Exiting the program
// }
