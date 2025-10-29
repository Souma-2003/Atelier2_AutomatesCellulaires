#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <cmath>
using namespace std;

// Version COMPLÈTEMENT REVISÉE de AC_HASH
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


// Test d'effet avalanche COMPLET
void comprehensive_avalanche_test() {
    cout << "=== TEST EFFET AVALANCHE COMPLET ===" << endl;
    cout << "Comparaison entre l'ancienne et nouvelle version AC_HASH" << endl;
    
    vector<string> test_messages = {
        "Hello", "World", "Test", "Message", "Avalanche",
        "Blockchain", "Security", "Cryptography", "Hash", "Function"
    };
    
    // Test avec l'ancienne version (si disponible)
    cout << "\n1. TESTS AVEC MESSAGES SIMILAIRES:" << endl;
    for (size_t i = 0; i < test_messages.size() - 1; i++) {
        string msg1 = test_messages[i];
        string msg2 = test_messages[i + 1];
        
        string hash1 = ac_hash(msg1, 30, 100);  // Using rule 30 and 100 steps
        string hash2 = ac_hash(msg2, 30, 100);  // Using rule 30 and 100 steps
        
        cout << "Message 1: \"" << msg1 << "\" -> " << hash1.substr(0, 16) << "..." << endl;
        cout << "Message 2: \"" << msg2 << "\" -> " << hash2.substr(0, 16) << "..." << endl;
        cout << "Similarite: " << (hash1 == hash2 ? "COLLISION!" : "OK") << endl << endl;
    }
    
    // Test d'effet avalanche détaillé
    cout << "\n2. TEST EFFET AVALANCHE DETAILLE:" << endl;
    int num_tests = 50;
    int collisions = 0;
    vector<double> percentages;
    
    for (int i = 0; i < num_tests; i++) {
        string original = "TestMessage" + to_string(i);
        
        // Modifier un bit à différentes positions
        for (int bit_pos = 0; bit_pos < 8; bit_pos++) {
            string modified = original;
            if (!modified.empty()) {
                char& c = modified[modified.length() / 2];
                c ^= (1 << bit_pos);
                
                string hash1 = ac_hash(original, 30, 100);  // Using rule 30 and 100 steps
                string hash2 = ac_hash(modified, 30, 100);  // Using rule 30 and 100 steps
                
                if (hash1 == hash2) {
                    collisions++;
                    cout << "COLLISION: \"" << original << "\" vs \"" << modified << "\"" << endl;
                }
                
                // Calculer le pourcentage de bits différents
                int diff_bits = 0;
                for (size_t j = 0; j < hash1.length() && j < hash2.length(); j++) {
                    bitset<8> bits1(hash1[j]);
                    bitset<8> bits2(hash2[j]);
                    diff_bits += (bits1 ^ bits2).count();
                }
                double percentage = (diff_bits * 100.0) / (hash1.length() * 8);
                percentages.push_back(percentage);
            }
        }
        
        if ((i + 1) % 10 == 0) {
            cout << "Progression: " << (i + 1) << "/" << num_tests << endl;
        }
    }
    
    // Analyse statistique
    if (!percentages.empty()) {
        double total = 0;
        double min_pct = 100, max_pct = 0;
        for (double p : percentages) {
            total += p;
            if (p < min_pct) min_pct = p;
            if (p > max_pct) max_pct = p;
        }
        double average = total / percentages.size();
        
        cout << "\n3. RESULTATS STATISTIQUES:" << endl;
        cout << "Tests effectues: " << percentages.size() << endl;
        cout << "Collisions detectees: " << collisions << endl;
        cout << "Pourcentage moyen de bits differents: " << fixed << setprecision(2) << average << "%" << endl;
        cout << "Minimum: " << fixed << setprecision(2) << min_pct << "%" << endl;
        cout << "Maximum: " << fixed << setprecision(2) << max_pct << "%" << endl;
        
        // Distribution
        vector<int> distribution(10, 0);
        for (double p : percentages) {
            int bucket = min(static_cast<int>(p / 10), 9);
            distribution[bucket]++;
        }
        
        cout << "\nDistribution:" << endl;
        for (int i = 0; i < 10; i++) {
            double pct = (distribution[i] * 100.0) / percentages.size();
            cout << setw(2) << (i * 10) << "-" << setw(2) << ((i + 1) * 10) << "%: " 
                 << distribution[i] << " tests (" << fixed << setprecision(1) << pct << "%)" << endl;
        }
        
        // Évaluation
        cout << "\n4. EVALUATION FINALE:" << endl;
        if (collisions > 0) {
            cout << " ECHEC: " << collisions << " collisions detectees" << endl;
        } else if (average >= 48 && average <= 52) {
            cout << " EXCELLENT: Effet avalanche optimal (~50%)" << endl;
        } else if (average >= 45 && average <= 55) {
            cout << " BON: Effet avalanche acceptable" << endl;
        } else if (average >= 40 && average <= 60) {
            cout << "  MOYEN: Effet avalanche modere" << endl;
        } else {
            cout << " INSUFFISANT: Effet avalanche trop faible" << endl;
        }
    }
}

// Test de performance basique
void basic_hash_tests() {
    cout << "=== TESTS DE BASE ===" << endl;
    
    vector<pair<string, string>> test_cases = {
        {"", "Empty string"},
        {"a", "Single char"},
        {"abc", "Short string"},
        {"Hello World!", "With punctuation"},
        {"Test message for AC_HASH function", "Longer message"}
    };
    
    for (const auto& test : test_cases) {
        string hash = ac_hash(test.first, 30, 100);  // Using rule 30 and 100 steps
        cout << setw(25) << test.second << ": " << hash.substr(0, 32) << "..." << endl;
    }
}

int main() {
    cout << "ANALYSE AC_HASH - EFFET AVALANCHE" << endl;
    cout << "==================================" << endl;
    
    basic_hash_tests();
    comprehensive_avalanche_test();
    
    return 0;
}