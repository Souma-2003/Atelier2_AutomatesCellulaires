#include <iostream>
#include <vector>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <string>
#include <random>
#include <chrono>
#include <cmath>

using namespace std;
using namespace std::chrono;

string ac_hash(const string& input, uint32_t rule, size_t steps) {
    vector<bool> input_bits;
    for (char c : input) {
        bitset<8> char_bits(c);
        for (int i = 7; i >= 0; --i)
            input_bits.push_back(char_bits[i]);
    }

    size_t original_size = input_bits.size();
    input_bits.push_back(1);
    while ((input_bits.size() + 64) % 256 != 0)
        input_bits.push_back(0);
    
    bitset<64> size_bits(original_size);
    for (int i = 63; i >= 0; --i)
        input_bits.push_back(size_bits[i]);

    vector<bool> state(256, false);

    for (size_t block = 0; block < input_bits.size(); block += 256) {
        for (size_t i = 0; i < 256 && (block + i) < input_bits.size(); ++i)
            state[i] = state[i] != input_bits[block + i];

        for (size_t step = 0; step < steps; ++step) {
            vector<bool> new_state(256, false);
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

            for (size_t i = 0; i < 256; ++i)
                new_state[i] = new_state[i] != state[(i * 7 + step * 13) % 256];

            state = new_state;
        }
    }

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

// Fonctions utilitaires pour les tests
double count_bit_difference(const string& hash1, const string& hash2) {
    if (hash1.length() != hash2.length()) return -1;
    
    int total_bits = hash1.length() * 4;
    int diff_count = 0;
    
    for (size_t i = 0; i < hash1.length(); ++i) {
        int val1, val2;
        stringstream ss1, ss2;
        ss1 << hex << hash1[i];
        ss2 << hex << hash2[i];
        ss1 >> val1;
        ss2 >> val2;
        
        bitset<4> bits1(val1);
        bitset<4> bits2(val2);
        bitset<4> diff = bits1 ^ bits2;
        diff_count += diff.count();
    }
    
    return (diff_count * 100.0) / total_bits;
}

string generate_random_message(size_t length) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(32, 126);
    
    string message;
    for (size_t i = 0; i < length; ++i) {
        message += static_cast<char>(dis(gen));
    }
    return message;
}

string flip_random_bit(const string& message) {
    if (message.empty()) return message;
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> byte_dis(0, message.length() - 1);
    uniform_int_distribution<> bit_dis(0, 7);
    
    string modified = message;
    size_t byte_pos = byte_dis(gen);
    int bit_pos = bit_dis(gen);
    
    modified[byte_pos] ^= (1 << bit_pos);
    return modified;
}

double test_avalanche_effect(uint32_t rule, size_t steps, size_t num_tests = 500) {
    double total_diff = 0.0;
    int valid_tests = 0;
    
    for (size_t i = 0; i < num_tests; ++i) {
        string original = generate_random_message(64);
        string modified = flip_random_bit(original);
        
        string hash_original = ac_hash(original, rule, steps);
        string hash_modified = ac_hash(modified, rule, steps);
        
        double diff_percentage = count_bit_difference(hash_original, hash_modified);
        
        if (diff_percentage >= 0) {
            total_diff += diff_percentage;
            valid_tests++;
        }
    }
    
    return total_diff / valid_tests;
}

double test_bit_distribution(uint32_t rule, size_t steps, size_t target_bits = 50000) {
    size_t total_bits = 0;
    size_t total_ones = 0;
    
    while (total_bits < target_bits) {
        string message = generate_random_message(64);
        string hash = ac_hash(message, rule, steps);
        
        for (char c : hash) {
            int value;
            stringstream ss;
            ss << hex << c;
            ss >> value;
            bitset<4> nibble(value);
            for (int i = 3; i >= 0; --i) {
                if (nibble[i]) total_ones++;
            }
        }
        total_bits += hash.length() * 4;
    }
    
    return (total_ones * 100.0) / total_bits;
}

double test_execution_time(uint32_t rule, size_t steps, size_t num_hashes = 1000) {
    vector<string> test_messages;
    for (size_t i = 0; i < num_hashes; ++i) {
        test_messages.push_back(generate_random_message(64));
    }
    
    auto start = high_resolution_clock::now();
    
    for (const string& message : test_messages) {
        ac_hash(message, rule, steps);
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    
    return duration.count() / 1000.0; // Retourne en millisecondes
}

void print_rule_info(uint32_t rule) {
    cout << "Regle " << rule << " (binaire: ";
    bitset<8> rule_bits(rule);
    for (int i = 7; i >= 0; --i) {
        cout << rule_bits[i];
    }
    cout << ")" << endl;
    
    // Caractéristiques connues des règles
    switch(rule) {
        case 30:
            cout << "  - Automate elementaire chaotique" << endl;
            cout << "  - Utilise dans Mathematica pour la generation aleatoire" << endl;
            cout << "  - Comportement complexe et imprevisible" << endl;
            break;
        case 90:
            cout << "  - Automate lineaire (règle XOR)" << endl;
            cout << "  - Produit des motifs fractals (Triangle de Sierpinski)" << endl;
            cout << "  - Structure mathématique simple" << endl;
            break;
        case 110:
            cout << "  - Automate universel de classe 4" << endl;
            cout << "  - Capable de calcul universel" << endl;
            cout << "  - Equilibre entre ordre et chaos" << endl;
            break;
    }
    cout << endl;
}

int main() {
    const size_t STEPS = 10;
    const size_t NUM_TESTS = 500;
    
    vector<uint32_t> rules = {30, 90, 110};
    
    cout << "=== COMPARAISON DES REGLES D'AUTOMATES CELLULAIRES ===" << endl;
    cout << "Nombre de tests par regle: " << NUM_TESTS << endl;
    cout << "Nombre d'etapes: " << STEPS << endl << endl;
    
    // Informations sur les règles
    for (uint32_t rule : rules) {
        print_rule_info(rule);
    }
    
    // Résultats des tests
    vector<double> avalanche_results;
    vector<double> distribution_results;
    vector<double> time_results;
    
    cout << "Execution des tests..." << endl;
    cout << "=========================================" << endl;
    
    for (uint32_t rule : rules) {
        cout << "\nTest de la regle " << rule << " en cours..." << endl;
        
        // Test d'effet avalanche
        double avalanche = test_avalanche_effect(rule, STEPS, NUM_TESTS);
        avalanche_results.push_back(avalanche);
        
        // Test de distribution
        double distribution = test_bit_distribution(rule, STEPS, 50000);
        distribution_results.push_back(distribution);
        
        // Test de performance
        double execution_time = test_execution_time(rule, STEPS, 1000);
        time_results.push_back(execution_time);
        
        cout << " Tests completes pour la regle " << rule << endl;
    }
    
    // Affichage des résultats
    cout << "\n=========================================" << endl;
    cout << "RESULTATS COMPARATIFS" << endl;
    cout << "=========================================" << endl;
    
    cout << fixed << setprecision(2);
    
    for (size_t i = 0; i < rules.size(); ++i) {
        cout << "\n--- Regle " << rules[i] << " ---" << endl;
        cout << "Effet avalanche: " << avalanche_results[i] << "% (ideal: 50%)" << endl;
        cout << "Distribution des bits: " << distribution_results[i] << "% de 1 (ideal: 50%)" << endl;
        cout << "Temps d'execution: " << time_results[i] << " ms pour 1000 hachages" << endl;
        cout << "Deviation avalanche: " << abs(50.0 - avalanche_results[i]) << "%" << endl;
        cout << "Deviation distribution: " << abs(50.0 - distribution_results[i]) << "%" << endl;
    }
    
    // Analyse comparative
    cout << "\n=========================================" << endl;
    cout << "ANALYSE COMPARATIVE" << endl;
    cout << "=========================================" << endl;
    
    // Trouver la meilleure règle basée sur les critères combinés
    double best_score = 0;
    int best_rule_index = 0;
    
    for (size_t i = 0; i < rules.size(); ++i) {
        // Score basé sur la qualité (avalanche + distribution) et la performance
        double avalanche_quality = 100.0 - abs(50.0 - avalanche_results[i]) * 10;
        double distribution_quality = 100.0 - abs(50.0 - distribution_results[i]) * 10;
        double performance_score = 1000.0 / time_results[i]; // Plus rapide = meilleur score
        
        double total_score = avalanche_quality * 0.4 + distribution_quality * 0.4 + performance_score * 0.2;
        
        cout << "Regle " << rules[i] << " - Score: " << total_score << endl;
        
        if (total_score > best_score) {
            best_score = total_score;
            best_rule_index = i;
        }
    }
    
    cout << "\n REGLE RECOMMANDEE: " << rules[best_rule_index] << "" << endl;
    cout << "Pourquoi:" << endl;
    
    switch(rules[best_rule_index]) {
        case 30:
            cout << "- Excellente propriete d'avalanche (" << avalanche_results[best_rule_index] << "%)" << endl;
            cout << "- Distribution équilibree (" << distribution_results[best_rule_index] << "% de 1)" << endl;
            cout << "- Comportement chaotique ideal pour la cryptographie" << endl;
            cout << "- Performance correcte" << endl;
            break;
        case 90:
            cout << "- Performance la plus rapide (" << time_results[best_rule_index] << " ms)" << endl;
            cout << "- Distribution acceptable (" << distribution_results[best_rule_index] << "% de 1)" << endl;
            cout << "- Structure mathematique simple mais moins securisee" << endl;
            break;
        case 110:
            cout << "- Meilleur equilibre securite/performance" << endl;
            cout << "- Bon effet avalanche (" << avalanche_results[best_rule_index] << "%)" << endl;
            cout << "- Distribution quasi-parfaite (" << distribution_results[best_rule_index] << "% de 1)" << endl;
            cout << "- Automate universel avec proprietes cryptographiques solides" << endl;
            break;
    }
    
    // Test supplémentaire: collision sur un message spécifique
    cout << "\n=========================================" << endl;
    cout << "TEST DE STABILITE SUR MESSAGE FIXE" << endl;
    cout << "=========================================" << endl;
    
    string test_message = "Hy, Hash_ac! This is a test message for cellular automata hash comparison.";
    
    for (uint32_t rule : rules) {
        string hash1 = ac_hash(test_message, rule, STEPS);
        string hash2 = ac_hash(test_message, rule, STEPS); // Même message, même règle
        
        if (hash1 == hash2) {
            cout << " Regle " << rule << ": Stabilite confirmee (hash identique pour meme entree)" << endl;
        } else {
            cout << " Regle " << rule << ": ERREUR - Hash different pour meme entree!" << endl;
        }
    }
    
    return 0;
}