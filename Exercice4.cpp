#include "sha256.cpp"
#include <iostream>
#include <sstream>
#include <ctime>
#include <vector>
#include <iomanip>
#include <chrono>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <bitset>
#include <functional>

using namespace std;
using namespace std::chrono;

// ==================== CONFIGURATION ====================
enum HashMode { SHA256_MODE, AC_HASH_MODE };
HashMode currentHashMode = SHA256_MODE;
uint32_t ac_hash_rule = 110;
size_t ac_hash_steps = 3;

// ==================== FONCTIONS DE HACHAGE ====================

string sha256(const string &str);

/**
 * Implémentation AC_HASH avec automate cellulaire
 */
string ac_hash(const string& input, uint32_t rule, size_t steps) {
    // 1. Conversion du texte en bits
    vector<bool> input_bits;
    for (char c : input) {
        bitset<8> char_bits(c);
        for (int i = 7; i >= 0; --i)
            input_bits.push_back(char_bits[i]);
    }

    // 2. Padding façon SHA
    size_t original_size = input_bits.size();
    input_bits.push_back(1); // bit de fin
    while ((input_bits.size() + 64) % 256 != 0)
        input_bits.push_back(0);
    
    bitset<64> size_bits(original_size);
    for (int i = 63; i >= 0; --i)
        input_bits.push_back(size_bits[i]);

    // 3. Initialisation de l'état
    vector<bool> state(256, false);

    // 4. Absorption des blocs
    for (size_t block = 0; block < input_bits.size(); block += 256) {
        // XOR bloc dans état
        for (size_t i = 0; i < 256 && (block + i) < input_bits.size(); ++i)
            state[i] = state[i] != input_bits[block + i];

        // Application dynamique de l'automate
        for (size_t step = 0; step < steps; ++step) {
            vector<bool> new_state(256, false);

            // Règle dynamique (change à chaque étape)
            uint32_t dynamic_rule = (rule + step * 37 + block) % 256;
            vector<bool> rule_bits(8);
            for (int i = 0; i < 8; ++i)
                rule_bits[7 - i] = (dynamic_rule >> i) & 1;

            for (size_t i = 0; i < 256; ++i) {
                int left = (i == 0) ? state[255] : state[i - 1];
                int center = state[i];
                int right = (i == 255) ? state[0] : state[i + 1];
                int pattern = (left << 2) | (center << 1) | right;
                new_state[i] = rule_bits[7 - pattern];
            }

            // Mélange additionnel
            for (size_t i = 0; i < 256; ++i)
                new_state[i] = new_state[i] != state[(i * 7 + step * 13) % 256];

            state = new_state;
        }
    }

    // 5. Finalisation (10 étapes supplémentaires)
    for (int k = 0; k < 10; ++k) {
        vector<bool> new_state(256);
        for (size_t i = 0; i < 256; ++i) {
            int left = state[(i + 255) % 256];
            int center = state[i];
            int right = state[(i + 1) % 256];
            int pattern = (left << 2) | (center << 1) | right;
            new_state[i] = ((rule >> pattern) & 1) ^ state[(i * 5 + k * 11) % 256];
        }
        state = new_state;
    }

    // 6. Conversion du résultat en hex
    stringstream hash_ss;
    hash_ss << hex << setfill('0');
    for (size_t i = 0; i < 256; i += 8) {
        uint8_t byte = 0;
        for (size_t j = 0; j < 8; ++j)
            byte = (byte << 1) | state[i + j];
        hash_ss << setw(2) << (int)byte;
    }

    return hash_ss.str();
}

/**
 * Fonction de hachage principale qui sélectionne l'algorithme
 */
string compute_hash(const string &str) {
    if (currentHashMode == AC_HASH_MODE) {
        return ac_hash(str, ac_hash_rule, ac_hash_steps);
    } else {
        return sha256(str);
    }
}

// ==================== CLASSES BLOCKCHAIN ====================

/**
 * Classe Transaction
 */
class Transaction {
public:
    string id;
    string sender;
    string receiver;
    double amount;
    long timestamp;

    Transaction(string s, string r, double a) : sender(s), receiver(r), amount(a) {
        timestamp = time(nullptr);
        id = compute_hash(s + r + to_string(a) + to_string(timestamp));
    }

    string toString() const {
        return "TX_" + id.substr(0, 8) + ": " + sender + " -> " + receiver + 
               " [" + to_string(amount) + " tokens]";
    }

    void display() const {
        cout << " From  " << sender << " to " << receiver << " : " << amount << " tokens";
        cout << "   | ID: " << id.substr(0, 16) << "...\n";
    }
};

/**
 * Classe Arbre de Merkle
 */
class MerkleTree {
private:
    vector<vector<string>> tree;

public:
    MerkleTree(const vector<Transaction> &transactions) {
        buildTree(transactions);
    }

    void buildTree(const vector<Transaction> &transactions) {
        tree.clear();
        if (transactions.empty()) {
            tree.push_back({compute_hash("")});
            return;
        }

        vector<string> currentLevel;
        for (const auto &tx : transactions)
            currentLevel.push_back(compute_hash(tx.toString()));
        tree.push_back(currentLevel);

        while (currentLevel.size() > 1) {
            currentLevel = buildMerkleLevel(currentLevel);
            tree.push_back(currentLevel);
        }
    }

    vector<string> buildMerkleLevel(const vector<string> &level) {
        vector<string> newLevel;
        for (size_t i = 0; i < level.size(); i += 2) {
            if (i + 1 < level.size())
                newLevel.push_back(compute_hash(level[i] + level[i + 1]));
            else
                newLevel.push_back(compute_hash(level[i] + level[i]));
        }
        return newLevel;
    }

    string getRoot() const {
        return tree.empty() ? "" : tree.back()[0];
    }

    void displayTree() const {
        cout << "\n MERKLE TREE STRUCTURE:\n";
        for (int level = tree.size() - 1; level >= 0; level--) {
            cout << "  Level " << (tree.size() - level - 1) << " : ";
            for (const auto &hash : tree[level])
                cout << hash.substr(0, 8) << "... ";
            cout << endl;
        }
    }
};

/**
 * Classe Block
 */
class Block {
public:
    int id;
    long timestamp;
    string previousHash;
    string merkleRoot;
    vector<Transaction> transactions;
    int nonce;
    string hash;
    string validator;
    double blockReward;

    Block(int idx, string prevHash, vector<Transaction> txs, double reward = 10.0)
        : id(idx), previousHash(prevHash), transactions(txs), blockReward(reward) {
        timestamp = time(nullptr);
        nonce = 0;
        validator = "";
        MerkleTree merkleTree(transactions);
        merkleRoot = merkleTree.getRoot();
        hash = calculateHash();
    }

    string calculateHash() const {
        stringstream ss;
        ss << id << timestamp << previousHash << merkleRoot << nonce << validator;
        return compute_hash(ss.str());
    }

    void mineBlock(int difficulty) {
        cout << "  Mining Block " << id << " with difficulty " << difficulty << " using ";
        cout << (currentHashMode == AC_HASH_MODE ? "AC_HASH" : "SHA256") << "...\n";
        
        string target(difficulty, '0');
        nonce = 0;
        auto start = high_resolution_clock::now();
        int iterations = 0;
        const int MAX_ITERATIONS = 50000;
        
        hash = calculateHash();
        
        bool hash_found = false;
        
        while (!hash_found) {
            if (hash.substr(0, difficulty) == target) {
                hash_found = true;
            } else if (iterations >= MAX_ITERATIONS) {
                cout << "     Maximum iterations reached (" << MAX_ITERATIONS << ")\n";
                cout << "     Accepting current hash for demonstration: " << hash.substr(0, 64) << "\n";
                hash_found = true;
            } else {
                nonce++;
                iterations++;
                hash = calculateHash();
                
                if (iterations % 2000 == 0) {
                    auto current_time = high_resolution_clock::now();
                    auto elapsed = duration_cast<seconds>(current_time - start).count();
                    cout << "    " << iterations << " iterations... " << elapsed << "s elapsed... Current hash: " << hash.substr(0, 16) << "...\n";
                }
            }
        }
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        
        if (hash.substr(0, difficulty) == target) {
            cout << "  Block mined successfully! Nonce: " << nonce << " | Hash: " << hash.substr(0, 64) << "\n";
        } else {
            cout << "  Mining incomplete - Using current hash for demonstration\n";
            cout << "  Nonce: " << nonce << " | Hash: " << hash.substr(0, 64) << "\n";
        }
        cout << "  Mining time: " << duration.count() / 1000.0 << " seconds | Iterations: " << iterations << "\n\n";
    }

    void validateBlock(const string &validateur) {
        auto start = high_resolution_clock::now();
        validator = validateur;
        hash = calculateHash();
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        cout << "  Block " << id << " validated by: " << validator << " (Time: " << duration.count() << " us)\n\n";
    }

    void display() const {
        cout << "------------------------------------------------------------\n";
        cout << "  BLOCK " << setw(3) << id << "\n";
        cout << "  Hash: " << hash.substr(0, 64) << "\n";
        cout << "  Previous: " << previousHash.substr(0, 64) << "\n";
        cout << "  Merkle Root: " << merkleRoot.substr(0, 64) << "\n";
        cout << "  Transactions: " << transactions.size() << " | Nonce: " << nonce << "\n";
        cout << "  Hash Mode: " << (currentHashMode == AC_HASH_MODE ? "AC_HASH" : "SHA256") << "\n";
        if (currentHashMode == AC_HASH_MODE) {
            cout << "  AC_HASH Rule: " << ac_hash_rule << " | Steps: " << ac_hash_steps << "\n";
        }
        if (!validator.empty()) cout << "  Validator: " << validator << "\n";
        cout << "  Reward: " << blockReward << " tokens\n";
        cout << "------------------------------------------------------------\n";
        if (!transactions.empty()) {
            cout << "  Transactions details:\n";
            for (const auto &tx : transactions) {
                tx.display();
            }
        }
        cout << endl;
    }
};

/**
 * Classe Blockchain
 */
class Blockchain {
private:
    int difficulty;
    map<string, double> stakes;
    double totalStake;
    string consensusType;

public:
    vector<Block> chain;
    string chainName;

    Blockchain(string name = "GenericChain", int diff = 1, string consensus = "PoW") 
        : chainName(name), difficulty(diff), totalStake(0), consensusType(consensus) {
        vector<Transaction> genesisTx = {Transaction("system", "founder", 1000)};
        chain.emplace_back(0, "0", genesisTx, 0);
        chain[0].hash = chain[0].calculateHash();
        cout << chainName << " initialized with Genesis Block!\n";
    }

    void initializeValidators(const vector<string> &validatorNames, const vector<double> &initialStakes) {
        if (consensusType != "PoS") {
            cout << "Note: Validators not used in PoW consensus\n";
            return;
        }
        
        stakes.clear();
        totalStake = 0;
        for (size_t i = 0; i < validatorNames.size() && i < initialStakes.size(); i++) {
            stakes[validatorNames[i]] = initialStakes[i];
            totalStake += initialStakes[i];
        }
        cout << "Validators initialized with total stake: " << totalStake << " tokens\n";
    }

    void addBlockPoW(vector<Transaction> transactions, double reward = 10.0) {
        Block newBlock(chain.size(), chain.back().hash, transactions, reward);
        newBlock.mineBlock(difficulty);
        chain.push_back(newBlock);
        cout << "  Block " << (chain.size()-1) << " added via PoW\n";
    }

    void addBlockPoS(vector<Transaction> transactions, double reward = 10.0) {
        if (stakes.empty() || totalStake == 0) {
            cerr << " No validators available for PoS!\n";
            return;
        }
        
        double random = (double)rand() / RAND_MAX * totalStake;
        double cumulative = 0;
        string selectedValidator;
        
        for (const auto &pair : stakes) {
            cumulative += pair.second;
            if (random <= cumulative) {
                selectedValidator = pair.first;
                break;
            }
        }
        
        cout << "  Selected validator for Block " << chain.size() << ": " << selectedValidator << "\n";
        
        Block newBlock(chain.size(), chain.back().hash, transactions, reward);
        newBlock.validateBlock(selectedValidator);
        chain.push_back(newBlock);
        stakes[selectedValidator] += reward;
        totalStake += reward;
        cout << "  Block " << (chain.size()-1) << " added via PoS\n";
    }

    bool isChainValid() const {
        cout << "\nValidating " << chainName << "...\n";
        bool isValid = true;
        
        for (size_t i = 1; i < chain.size(); i++) {
            const Block &current = chain[i];
            const Block &previous = chain[i-1];
            
            if (current.previousHash != previous.hash) {
                cout << "   Invalid previous hash at block " << current.id << "\n";
                isValid = false;
            }
            
            if (current.hash != current.calculateHash()) {
                cout << "   Invalid hash at block " << current.id << "\n";
                isValid = false;
            }
            
            MerkleTree currentTree(current.transactions);
            if (current.merkleRoot != currentTree.getRoot()) {
                cout << "   Invalid Merkle Root at block " << current.id << "\n";
                isValid = false;
            }
        }
        
        if (isValid) {
            cout << "   " << chainName << " is valid! (" << chain.size() << " blocks)\n";
        } else {
            cout << "   " << chainName << " is invalid!\n";
        }
        return isValid;
    }

    void displayChain() const {
        cout << "\n" << chainName << " - BLOCKCHAIN (" << chain.size() << " blocks)\n";
        cout << "============================================================\n";
        for (const auto &block : chain) {
            block.display();
        }
    }

    void displayStakes() const {
        if (consensusType != "PoS") return;
        if (stakes.empty()) { 
            cout << "No validators configured.\n"; 
            return; 
        }
        
        cout << "\nCURRENT STAKES DISTRIBUTION:\n";
        cout << "----------------------------------------------------------\n";
        cout << setw(15) << "Validator" << setw(12) << "Stake" << setw(12) << "Percentage\n";
        cout << "----------------------------------------------------------\n";
        for (const auto &pair : stakes) {
            double percentage = (pair.second / totalStake) * 100;
            cout << setw(15) << pair.first << setw(12) << pair.second 
                 << setw(11) << fixed << setprecision(2) << percentage << "%\n";
        }
        cout << "----------------------------------------------------------\n";
        cout << "Total Stake: " << totalStake << " tokens\n";
    }

    void displayStatistics() const {
        cout << "\n" << chainName << " STATISTICS:\n";
        cout << "  Total Blocks: " << chain.size() << "\n";
        
        int totalTx = 0;
        for (const auto &block : chain) totalTx += block.transactions.size();
        cout << "  Total Transactions: " << totalTx << "\n";
        cout << "  Chain Difficulty: " << difficulty << "\n";
        cout << "  Consensus: " << consensusType << "\n";
        cout << "  Hash Mode: " << (currentHashMode == AC_HASH_MODE ? "AC_HASH" : "SHA256") << "\n";
        
        if (consensusType == "PoS") {
            cout << "  Total Validators: " << stakes.size() << "\n";
        }
        
        if (currentHashMode == AC_HASH_MODE) {
            cout << "  AC_HASH Rule: " << ac_hash_rule << "\n";
            cout << "  AC_HASH Steps: " << ac_hash_steps << "\n";
        }
    }
};

// ==================== FONCTIONS UTILITAIRES ====================

/**
 * Fonction utilitaire pour dessiner des barres de progression
 */
string drawBar(double value, double maxVal, int length = 20) {
    int filled = static_cast<int>((value / maxVal) * length);
    return string(filled, '=') + string(length - filled, ' ');
}

// ==================== FONCTIONS DE TEST PERFORMANCE ====================

/**
 * Fonction pour tester les performances entre ac_hash et SHA256
 */
/**
 * Fonction pour tester les performances entre ac_hash et SHA256
 */
void testPerformance() {
    cout << "\n\n=== TEST DE PERFORMANCE AC_HASH vs SHA256 ===\n";
    cout << "============================================\n";
    
    // Sauvegarde de la configuration actuelle
    HashMode originalMode = currentHashMode;
    uint32_t originalRule = ac_hash_rule;
    size_t originalSteps = ac_hash_steps;
    
    // Données de test pour le minage
    vector<Transaction> testTransactions = {
        Transaction("test1", "test2", 10.0),
        Transaction("test3", "test4", 20.0)
    };
    
    // Résultats
    struct MiningResult {
        double avgTime;
        double avgIterations;
        vector<double> times;
        vector<int> iterations;
        vector<string> hashes;
    };
    
    map<string, MiningResult> results;
    vector<string> methods = {"SHA256", "AC_HASH"};
    
    const int NUM_BLOCKS = 10;
    const int DIFFICULTY = 1;  // Difficulté fixée à 1 comme demandé
    
    // Test pour chaque méthode
    for (const string& method : methods) {
        cout << "\n--- Test avec " << method << " ---\n";
        
        // Configuration de la méthode
        if (method == "SHA256") {
            currentHashMode = SHA256_MODE;
        } else {
            currentHashMode = AC_HASH_MODE;
            ac_hash_rule = 110;
            ac_hash_steps = 3;
        }
        
        double totalTime = 0;
        double totalIterations = 0;
        vector<double> times;
        vector<int> iterations;
        vector<string> hashes;
        
        for (int i = 0; i < NUM_BLOCKS; i++) {
            cout << "  Test bloc " << (i+1) << "/" << NUM_BLOCKS << "... \n";
            
            // Simulation de minage pour mesurer les itérations
            string target(DIFFICULTY, '0');
            int iteration_count = 0;
            string testData = "block_data_" + to_string(i) + "_" + to_string(time(nullptr));
            string hash;
            int nonce = 0;
            
            auto start = high_resolution_clock::now();
            
            do {
                string dataWithNonce = testData + to_string(nonce);
                hash = compute_hash(dataWithNonce);
                iteration_count++;
                nonce++;
                
                // Sécurité pour éviter les boucles infinies
                if (iteration_count > 1000000) {
                    cout << "    Timeout après 1,000,000 itérations\n";
                    break;
                }
                
            } while (hash.substr(0, DIFFICULTY) != target);
            
            auto end = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(end - start); // Changé en microseconds pour plus de précision
            
            double timeMs = duration.count() / 1000.0; // Conversion en millisecondes
            totalTime += timeMs;
            totalIterations += iteration_count;
            times.push_back(timeMs);
            iterations.push_back(iteration_count);
            hashes.push_back(hash);
            
            // Affichage détaillé du minage pour ce bloc
            cout << "    Hash trouve: " << hash << "\n";
            cout << "    Nonce: " << (nonce - 1) << " | Iterations: " << iteration_count << " | Temps: " << timeMs << " ms\n\n";
        }
        
        results[method] = {
            totalTime / NUM_BLOCKS,
            totalIterations / NUM_BLOCKS,
            times,
            iterations,
            hashes
        };
    }
    
    // Restauration de la configuration originale
    currentHashMode = originalMode;
    ac_hash_rule = originalRule;
    ac_hash_steps = originalSteps;
    
    // Affichage des résultats dans un tableau
    cout << "\n\n=== RAPPORT DE COMPARAISON ===\n";
    cout << "==============================\n";
    cout << fixed << setprecision(4); // Plus de précision pour les temps
    
    // En-tête du tableau
    cout << "+----------+------------------+------------------+------------------+------------------+\n";
    cout << "| Methode  | Temps moyen (ms) | Iterations moy.  | Temps min (ms)   | Temps max (ms)   |\n";
    cout << "+----------+------------------+------------------+------------------+------------------+\n";
    
    // Lignes de données
    for (const string& method : methods) {
        MiningResult result = results[method];
        double minTime = *min_element(result.times.begin(), result.times.end());
        double maxTime = *max_element(result.times.begin(), result.times.end());
        
        cout << "| " << setw(8) << left << method 
             << " | " << setw(16) << result.avgTime 
             << " | " << setw(16) << result.avgIterations
             << " | " << setw(16) << minTime
             << " | " << setw(16) << maxTime << " |\n";
    }
    cout << "+----------+------------------+------------------+------------------+------------------+\n";
    
    // Analyse comparative détaillée - TOUJOURS AFFICHER MÊME SI TEMPS = 0
    cout << "\n=== ANALYSE COMPARATIVE DETAILLEE ===\n";
    double sha256Time = results["SHA256"].avgTime;
    double acHashTime = results["AC_HASH"].avgTime;
    double sha256Iter = results["SHA256"].avgIterations;
    double acHashIter = results["AC_HASH"].avgIterations;
    
    // SUPPRIMER LA CONDITION - TOUJOURS AFFICHER L'ANALYSE
    cout << "4.1. Temps moyen de minage de " << NUM_BLOCKS << " blocs (difficulté " << DIFFICULTY << ") :\n";
    cout << "   - SHA256: " << sha256Time << " ms par bloc\n";
    cout << "   - AC_HASH: " << acHashTime << " ms par bloc\n";
    
    if (sha256Time > 0) {
        double timeRatio = acHashTime / sha256Time;
        cout << "   - Rapport: " << timeRatio << "x (" << (timeRatio > 1 ? "AC_HASH est plus lent" : "AC_HASH est plus rapide") << ")\n";
    } else {
        cout << "   - Rapport: SHA256 trop rapide pour calculer un rapport significatif\n";
    }
    cout << "\n";
    
    cout << "4.2. Nombre moyen d'iterations pour trouver un hash valide :\n";
    cout << "   - SHA256: " << sha256Iter << " iterations par bloc\n";
    cout << "   - AC_HASH: " << acHashIter << " iterations par bloc\n";
    
    if (sha256Iter > 0) {
        double iterRatio = acHashIter / sha256Iter;
        cout << "   - Rapport: " << iterRatio << "x (" << (iterRatio > 1 ? "AC_HASH necessite plus d'iterations" : "AC_HASH necessite moins d'iterations") << ")\n";
    } else {
        cout << "   - Rapport: SHA256 trop efficace pour calculer un rapport significatif\n";
    }
    cout << "\n";
    
    // Affichage des hashs trouvés pour chaque bloc
    cout << "4.3. Hashs trouves pour chaque bloc :\n";
    cout << "+-------+----------------------------------------------------------------+----------------------------------------------------------------+\n";
    cout << "| Bloc  | SHA256 Hash                                                    | AC_HASH Hash                                                    |\n";
    cout << "+-------+----------------------------------------------------------------+----------------------------------------------------------------+\n";
    
    for (int i = 0; i < NUM_BLOCKS; i++) {
        cout << "| " << setw(5) << (i+1) 
             << " | " << setw(62) << results["SHA256"].hashes[i]
             << " | " << setw(62) << results["AC_HASH"].hashes[i] << " |\n";
    }
    cout << "+-------+----------------------------------------------------------------+----------------------------------------------------------------+\n";
    
    // Tableau de résultats détaillés
    cout << "\n4.4. Tableau de resultats detailles (temps et iterations) :\n";
    cout << "+-------+------------------+------------------+------------------+------------------+\n";
    cout << "| Bloc  | SHA256 (ms)      | SHA256 (iter)    | AC_HASH (ms)     | AC_HASH (iter)   |\n";
    cout << "+-------+------------------+------------------+------------------+------------------+\n";
    
    for (int i = 0; i < NUM_BLOCKS; i++) {
        cout << "| " << setw(5) << (i+1) 
             << " | " << setw(16) << results["SHA256"].times[i]
             << " | " << setw(16) << results["SHA256"].iterations[i]
             << " | " << setw(16) << results["AC_HASH"].times[i]
             << " | " << setw(16) << results["AC_HASH"].iterations[i] << " |\n";
    }
    cout << "+-------+------------------+------------------+------------------+------------------+\n";
    
    // Recommandation finale
    cout << "\n=== RECOMMANDATION ===\n";
    if (sha256Time > 0 && acHashTime > 0) {
        double timeRatio = acHashTime / sha256Time;
        double iterRatio = acHashIter / sha256Iter;
        
        if (timeRatio < 1.0 && iterRatio < 1.0) {
            cout << "AC_HASH est plus performant que SHA256 dans les deux metriques.\n";
        } else if (timeRatio < 1.0) {
            cout << "AC_HASH est plus rapide mais necessite plus d'iterations.\n";
        } else if (iterRatio < 1.0) {
            cout << "AC_HASH necessite moins d'iterations mais est plus lent.\n";
        } else {
            cout << "SHA256 est plus performant que AC_HASH dans les deux metriques.\n";
        }
    } else {
        if (sha256Time == 0 && acHashTime > 0) {
            cout << "SHA256 est significativement plus rapide que AC_HASH.\n";
        } else {
            cout << "Comparaison difficile en raison des temps de calcul trop faibles.\n";
        }
    }
    
    // Ajouter une pause pour permettre la lecture
    cout << "\nAppuyez sur Entree pour continuer...";
    cin.ignore();
    cin.get();
}

// ==================== DEMONSTRATION NORMALE ====================

/**
 * Fonction pour exécuter la démonstration normale
 */
void runNormalDemo() {
    // Données de test
    vector<Transaction> txs1 = {
        Transaction("sen1", "rec1", 50.5),
        Transaction("sen2", "rec2", 25.0),
        Transaction("sen3", "rec3", 15.75)
    };
    vector<Transaction> txs2 = {
        Transaction("sen4", "rec4", 10.0),
        Transaction("sen1", "rec2", 5.25),
        Transaction("sen5", "rec5", 8.5)
    };
    vector<Transaction> txs3 = {
        Transaction("sen6", "rec1", 3.0),
        Transaction("sen2", "rec5", 12.5),
        Transaction("sen5", "rec6", 7.8)
    };

    // ------------------- Test Proof of Work -------------------
    cout << "\n=== DEMARRAGE DE LA BLOCKCHAIN PoW ===\n";
    int powDifficulty = (currentHashMode == AC_HASH_MODE) ? 1 : 2;
    Blockchain powChain("PoW_Chain", powDifficulty, "PoW");
    
    cout << " Configuration - Difficulte: " << powDifficulty << " | ";
    cout << "Hash: " << (currentHashMode == AC_HASH_MODE ? "AC_HASH (Demo mode)" : "SHA256") << "\n\n";
    
    auto powStart = high_resolution_clock::now();
    
    try {
        powChain.addBlockPoW(txs1, 12.5);
        powChain.addBlockPoW(txs2, 12.5);
        powChain.addBlockPoW(txs3, 12.5);
    } catch (const exception& e) {
        cout << " Erreur pendant le mining: " << e.what() << endl;
        return;
    }
    
    auto powEnd = high_resolution_clock::now();
    chrono::duration<double, milli> powDuration = powEnd - powStart;

    powChain.displayStatistics();
    
    // Validation de la chaîne PoW
    cout << "\n=== VALIDATION DE LA CHAINE PoW ===\n";
    bool powValid = powChain.isChainValid();

    // ------------------- Test Proof of Stake -------------------
    cout << "\n=== DEMARRAGE DE LA BLOCKCHAIN PoS ===\n";
    Blockchain posChain("PoS_Chain", 2, "PoS");
    posChain.initializeValidators({"Validator_A", "Validator_B", "Validator_C", "Validator_D"}, 
                                  {1000, 2000, 1500, 1200});

    auto posStart = high_resolution_clock::now();
    posChain.addBlockPoS(txs1, 15.0);
    posChain.addBlockPoS(txs2, 15.0);
    posChain.addBlockPoS(txs3, 15.0);
    auto posEnd = high_resolution_clock::now();
    chrono::duration<double, milli> posDuration = posEnd - posStart;

    posChain.displayStatistics();
    posChain.displayStakes();
    
    cout << "\n=== VALIDATION DE LA CHAINE PoS ===\n";
    bool posValid = posChain.isChainValid();

    // ------------------- Analyse comparative -------------------
    cout << "\n\n=== ANALYSE COMPARATIVE ===\n";
    cout << "===============================\n";

    double powSpeed = powDuration.count() / 3.0;
    double posSpeed = posDuration.count() / 3.0;
    double maxTime = max(powDuration.count(), posDuration.count());

    cout << left;
    cout << "Metric                 | Proof of Work        | Proof of Stake\n";
    cout << "--------------------------------------------------------------\n";
    cout << setw(22) << "Total Time (ms)" 
         << " | " << setw(20) << (to_string((int)powDuration.count()) + " [" + drawBar(powDuration.count(), maxTime) + "]")
         << " | " << setw(20) << (to_string((int)posDuration.count()) + " [" + drawBar(posDuration.count(), maxTime) + "]") << "\n";
    cout << setw(22) << "Blocks Added" 
         << " | " << setw(20) << "3" 
         << " | " << setw(20) << "3" << "\n";
    cout << setw(22) << "Speed per Block (ms)" 
         << " | " << setw(20) << (int)powSpeed 
         << " | " << setw(20) << (int)posSpeed << "\n";
    cout << setw(22) << "Hash Mode" 
         << " | " << setw(20) << (currentHashMode == AC_HASH_MODE ? "AC_HASH" : "SHA256")
         << " | " << setw(20) << (currentHashMode == AC_HASH_MODE ? "AC_HASH" : "SHA256") << "\n";
    cout << setw(22) << "Consensus" 
         << " | " << setw(20) << "PoW"
         << " | " << setw(20) << "PoS" << "\n";
    cout << setw(22) << "Chain Valid" 
         << " | " << setw(20) << (powValid ? "VALID" : "INVALID")
         << " | " << setw(20) << (posValid ? "VALID" : "INVALID") << "\n";
    cout << "--------------------------------------------------------------\n";

    // ------------------- Affichage détaillé -------------------
    cout << "\n\n=== AFFICHAGE DETAILLE DES CHAINES ===\n";
    cout << "=================================\n";

    cout << "\nAppuyez sur Entree pour voir les details de la chaine PoW...";
    cin.ignore();
    cin.get();
    powChain.displayChain();

    cout << "\nAppuyez sur Entree pour voir les details de la chaine PoS...";
    cin.get();
    posChain.displayChain();
}

// ==================== PROGRAMME PRINCIPAL ====================

int main() {
    srand(time(nullptr));

    cout << "=================================================\n";
    cout << "         MINI BLOCKCHAIN FROM SCRATCH\n";
    cout << "=================================================\n\n";

    int mainChoice;
    do {
        cout << "\n=== Configuration du mode de hachage ===\n";
        cout << "1. SHA256 (Standard)\n";
        cout << "2. AC_HASH (Automate Cellulaire)\n"; 
        cout << "3. Test Performance (AC_HASH vs SHA256)\n";
        cout << "0. Quitter\n";
        cout << "Choix: ";
        cin >> mainChoice;

        switch (mainChoice) {
            case 1:
                currentHashMode = SHA256_MODE;
                cout << "Mode SHA256 configure\n";
                runNormalDemo();
                break;
            case 2:
                currentHashMode = AC_HASH_MODE;
                cout << "Regle AC_HASH (30, 90, 110): ";
                cin >> ac_hash_rule;
                cout << "Nombre d'etapes: ";
                cin >> ac_hash_steps;
                cout << "Mode AC_HASH configure (Regle: " << ac_hash_rule << ", Etapes: " << ac_hash_steps << ")\n";
                runNormalDemo();
                break;
            case 3:
                testPerformance();
                break;
            case 0:
                cout << "Au revoir!\n";
                break;
            default:
                cout << "Choix invalide!\n";
                break;
        }
    } while (mainChoice != 0);
    
    return 0;
}