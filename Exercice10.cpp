#include <iostream>
#include <vector>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <string>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <functional>

using namespace std;
using namespace std::chrono;

// Version améliorée avec règle dynamique adaptative et voisinage variable
string ac_hash_plus(const string& input, uint32_t base_rule, size_t steps) {
    // 1. Conversion du texte en bits avec permutation
    vector<bool> input_bits;
    for (char c : input) {
        bitset<8> char_bits(c);
        // Permutation des bits pour plus de diffusion
        vector<int> permutation = {2, 5, 0, 7, 1, 4, 3, 6};
        for (int idx : permutation) {
            input_bits.push_back(char_bits[idx]);
        }
    }

    // 2. Padding amélioré
    size_t original_size = input_bits.size();
    input_bits.push_back(1);
    
    // Ajout de sel basé sur la longueur du message
    size_t salt = original_size * 37;
    for (int i = 0; i < 32; ++i) {
        input_bits.push_back((salt >> i) & 1);
    }
    
    while ((input_bits.size() + 64) % 512 != 0) {
        input_bits.push_back(0);
    }
    
    bitset<64> size_bits(original_size);
    for (int i = 63; i >= 0; --i) {
        input_bits.push_back(size_bits[i]);
    }

    // 3. État étendu à 512 bits pour plus de sécurité
    vector<bool> state(512, false);

    // 4. Absorption avec voisinage variable
    for (size_t block = 0; block < input_bits.size(); block += 512) {
        // XOR du bloc dans l'état avec rotation
        for (size_t i = 0; i < 512 && (block + i) < input_bits.size(); ++i) {
            size_t rotated_pos = (i * 3) % 512;
            state[rotated_pos] = state[rotated_pos] != input_bits[block + i];
        }

        // Application de l'automate avec règle dynamique adaptative
        for (size_t step = 0; step < steps; ++step) {
            vector<bool> new_state(512, false);
            
            // Règle dynamique basée sur l'état actuel
            uint32_t state_hash = 0;
            for (size_t j = 0; j < 32; ++j) {
                if (state[j * 16]) state_hash |= (1 << j);
            }
            
            uint32_t dynamic_rule = (base_rule + step * 37 + block + state_hash) % 256;
            
            // Voisinage variable (3, 5 ou 7 cellules) basé sur l'étape
            int neighborhood_size = 3 + (step % 3) * 2; // 3, 5 ou 7
            
            for (size_t i = 0; i < 512; ++i) {
                int pattern = 0;
                int bits_to_take = neighborhood_size;
                
                // Construction du motif selon la taille du voisinage
                for (int j = 0; j < bits_to_take; ++j) {
                    int offset = j - bits_to_take / 2;
                    size_t pos = (i + offset + 512) % 512;
                    pattern = (pattern << 1) | state[pos];
                }
                
                // Application de la règle
                new_state[i] = (dynamic_rule >> (pattern % 8)) & 1;
            }

            // Mélange additionnel amélioré
            for (size_t i = 0; i < 512; ++i) {
                size_t mix_pos1 = (i * 7 + step * 13) % 512;
                size_t mix_pos2 = (i * 11 + step * 17) % 512;
                new_state[i] = new_state[i] != state[mix_pos1] != state[mix_pos2];
            }

            state = new_state;
        }
    }

    // 5. Finalisation étendue avec plus d'étapes
    for (int k = 0; k < 20; ++k) {
        vector<bool> new_state(512);
        for (size_t i = 0; i < 512; ++i) {
            // Voisinage de 5 cellules pour la finalisation
            int left2 = state[(i + 510) % 512];
            int left1 = state[(i + 511) % 512];
            int center = state[i];
            int right1 = state[(i + 1) % 512];
            int right2 = state[(i + 2) % 512];
            int pattern = (left2 << 4) | (left1 << 3) | (center << 2) | (right1 << 1) | right2;
            
            new_state[i] = ((base_rule >> (pattern % 32)) & 1) ^ state[(i * 7 + k * 19) % 512];
        }
        state = new_state;
    }

    // 6. Compression de 512 bits à 256 bits pour la sortie
    vector<bool> final_state(256, false);
    for (size_t i = 0; i < 256; ++i) {
        final_state[i] = state[i] != state[i + 256];
    }

    // 7. Conversion en hexadecimal
    stringstream hash_ss;
    hash_ss << hex << setfill('0');
    for (size_t i = 0; i < 256; i += 8) {
        uint8_t byte = 0;
        for (size_t j = 0; j < 8; ++j) {
            byte = (byte << 1) | final_state[i + j];
        }
        hash_ss << setw(2) << (int)byte;
    }

    return hash_ss.str();
}

// Fonctions de test communes
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

// Tests pour une fonction de hachage donnée
struct TestResults {
    double avalanche_effect;
    double bit_distribution;
    double execution_time_ms;
    string test_hash;
};

TestResults test_hash_function(const string& hash_name, 
                              function<string(const string&)> hash_func,
                              size_t num_tests = 1000) {
    
    cout << "Testing " << hash_name << "..." << endl;
    
    TestResults results;
    
    // Test d'effet avalanche
    double total_avalanche = 0.0;
    int avalanche_tests = 0;
    
    for (size_t i = 0; i < num_tests; ++i) {
        string original = generate_random_message(64);
        string modified = flip_random_bit(original);
        
        string hash_orig = hash_func(original);
        string hash_mod = hash_func(modified);
        
        double diff = count_bit_difference(hash_orig, hash_mod);
        if (diff >= 0) {
            total_avalanche += diff;
            avalanche_tests++;
        }
        
        if (i == 0) {
            results.test_hash = hash_orig.substr(0, 16) + "..."; // Premier hash pour exemple
        }
    }
    results.avalanche_effect = total_avalanche / avalanche_tests;
    
    // Test de distribution des bits
    size_t total_bits = 0;
    size_t total_ones = 0;
    
    for (size_t i = 0; i < num_tests; ++i) {
        string message = generate_random_message(64);
        string hash = hash_func(message);
        
        for (char c : hash) {
            int value;
            stringstream ss;
            ss << hex << c;
            ss >> value;
            bitset<4> nibble(value);
            for (int j = 0; j < 4; ++j) {
                if (nibble[j]) total_ones++;
            }
        }
        total_bits += hash.length() * 4;
    }
    results.bit_distribution = (total_ones * 100.0) / total_bits;
    
    // Test de performance
    vector<string> test_messages;
    for (size_t i = 0; i < num_tests; ++i) {
        test_messages.push_back(generate_random_message(64));
    }
    
    auto start = high_resolution_clock::now();
    for (const string& msg : test_messages) {
        hash_func(msg);
    }
    auto end = high_resolution_clock::now();
    
    results.execution_time_ms = duration_cast<microseconds>(end - start).count() / 1000.0;
    
    cout << " " << hash_name << " completed" << endl;
    return results;
}

// Wrappers pour les différentes versions
string ac_hash_30(const string& input) {
    return ac_hash_plus(input, 30, 10);
}

string ac_hash_90(const string& input) {
    return ac_hash_plus(input, 90, 10);
}

string ac_hash_110(const string& input) {
    return ac_hash_plus(input, 110, 10);
}

// Fonction originale pour comparaison (doit être définie)
string ac_hash_original(const string& input, uint32_t rule, size_t steps) {
    // Implémentation originale fournie précédemment
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

string ac_hash_original_30(const string& input) {
    return ac_hash_original(input, 30, 10);
}

string ac_hash_original_90(const string& input) {
    return ac_hash_original(input, 90, 10);
}

string ac_hash_original_110(const string& input) {
    return ac_hash_original(input, 110, 10);
}

int main() {
    const size_t NUM_TESTS = 1000;
    
    cout << "==============================================" << endl;
    cout << "COMPREHENSIVE HASH FUNCTION ANALYSIS" << endl;
    cout << "==============================================" << endl;
    cout << "Number of tests per function: " << NUM_TESTS << endl << endl;
    
    vector<pair<string, function<string(const string&)>>> hash_functions = {
        {"Original Rule 30", ac_hash_original_30},
        {"Original Rule 90", ac_hash_original_90},
        {"Original Rule 110", ac_hash_original_110},
        {"AC-Hash+ Rule 30", ac_hash_30},
        {"AC-Hash+ Rule 90", ac_hash_90},
        {"AC-Hash+ Rule 110", ac_hash_110}
    };
    
    vector<TestResults> all_results;
    
    // Exécution des tests
    for (const auto& hash_func : hash_functions) {
        all_results.push_back(test_hash_function(hash_func.first, hash_func.second, NUM_TESTS));
    }
    
    // Affichage des résultats sous forme de tableau
    cout << "\n" << string(120, '=') << endl;
    cout << "FINAL RESULTS COMPARISON" << endl;
    cout << string(120, '=') << endl;
    
    cout << left << setw(25) << "Hash Function" 
         << setw(15) << "Avalanche (%)" 
         << setw(15) << "Distribution (%)" 
         << setw(15) << "Time (ms)" 
         << setw(30) << "Sample Hash" 
         << setw(15) << "Quality Score" << endl;
    cout << string(120, '-') << endl;
    
    cout << fixed << setprecision(2);
    
    for (size_t i = 0; i < hash_functions.size(); ++i) {
        double avalanche_dev = abs(50.0 - all_results[i].avalanche_effect);
        double dist_dev = abs(50.0 - all_results[i].bit_distribution);
        double quality_score = 100.0 - (avalanche_dev * 10 + dist_dev * 5 + all_results[i].execution_time_ms / 10);
        
        cout << left << setw(25) << hash_functions[i].first
             << setw(15) << all_results[i].avalanche_effect
             << setw(15) << all_results[i].bit_distribution
             << setw(15) << all_results[i].execution_time_ms
             << setw(30) << all_results[i].test_hash
             << setw(15) << quality_score << endl;
    }
    
    // Recommandation
    cout << string(120, '=') << endl;
    cout << "RECOMMENDATION:" << endl;
    
    double best_score = 0;
    string best_function;
    
    for (size_t i = 0; i < hash_functions.size(); ++i) {
        double avalanche_dev = abs(50.0 - all_results[i].avalanche_effect);
        double dist_dev = abs(50.0 - all_results[i].bit_distribution);
        double quality_score = 100.0 - (avalanche_dev * 10 + dist_dev * 5 + all_results[i].execution_time_ms / 10);
        
        if (quality_score > best_score) {
            best_score = quality_score;
            best_function = hash_functions[i].first;
        }
    }
    
    cout << "Best performing function: " << best_function << " " << endl;
    cout << "Quality Score: " << best_score << "/100" << endl;
    
    // Détails des améliorations
    cout << "\nIMPROVEMENTS IN AC-Hash+:" << endl;
    cout << " Etat etendu a 512 bits" << endl;
    cout << " Voisinage variable (3,5,7 cellules)" << endl;
    cout << " Regle dynamique adaptative basee sur l'etat" << endl;
    cout << " Permutation des bits d'entree" << endl;
    cout << " Melange additionnel ameliore" << endl;
    cout << " Finalisation etendue (20 etapes)" << endl;
    cout << " Compression 512→256 bits" << endl;
    
    return 0;
}