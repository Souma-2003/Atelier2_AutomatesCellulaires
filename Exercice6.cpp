#include <iostream>
#include <vector>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <string>
#include <random>
#include <cmath>

using namespace std;

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

// Fonction pour extraire les bits d'un hash hexadécimal
vector<bool> hash_to_bits(const string& hash) {
    vector<bool> bits;
    for (char c : hash) {
        int value;
        stringstream ss;
        ss << hex << c;
        ss >> value;
        bitset<4> nibble(value);
        for (int i = 3; i >= 0; --i) {
            bits.push_back(nibble[i]);
        }
    }
    return bits;
}

// Générer un message aléatoire
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

// Test statistique du chi-carré pour l'équilibre des bits
double chi_squared_test(int ones_count, int total_bits) {
    double expected = total_bits / 2.0;
    double chi_squared = pow(ones_count - expected, 2) / expected + 
                         pow((total_bits - ones_count) - expected, 2) / expected;
    return chi_squared;
}

int main() {
    const size_t TARGET_BITS = 100000;  // Au moins 10⁵ bits
    const size_t MESSAGE_LENGTH = 64;   // Longueur des messages en octets
    const uint32_t RULE = 110;
    const size_t STEPS = 10;
    
    size_t total_bits = 0;
    size_t total_ones = 0;
    size_t hash_count = 0;
    
    cout << "Analyse de la distribution des bits de ac_hash..." << endl;
    cout << "Objectif: au moins " << TARGET_BITS << " bits" << endl;
    cout << "Regle: " << RULE << ", Etapes: " << STEPS << endl << endl;
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<uint32_t> rule_dis(0, 255);
    
    // Utiliser différentes règles pour plus de variété
    vector<uint32_t> rules = {30, 45, 73, 89, 101, 110, 124, 135, 149, 150, 
                              153, 165, 182, 184, 188, 190, 193, 195, 210, 222};
    
    while (total_bits < TARGET_BITS) {
        // Générer un message aléatoire
        string message = generate_random_message(MESSAGE_LENGTH);
        
        // Choisir une règle aléatoire parmi la liste
        uint32_t current_rule = rules[hash_count % rules.size()];
        
        // Calculer le hash
        string hash = ac_hash(message, current_rule, STEPS);
        
        // Extraire les bits
        vector<bool> bits = hash_to_bits(hash);
        
        // Compter les bits à 1
        for (bool bit : bits) {
            if (bit) total_ones++;
        }
        
        total_bits += bits.size();
        hash_count++;
        
        if (hash_count % 100 == 0) {
            double percentage = (total_ones * 100.0) / total_bits;
            cout << "Hashs calcules: " << hash_count 
                 << ", Bits analyses: " << total_bits 
                 << ", Pourcentage de 1: " << fixed << setprecision(4) << percentage << "%" << endl;
        }
    }
    
    // Calcul des résultats finaux
    double percentage_ones = (total_ones * 100.0) / total_bits;
    double chi_squared = chi_squared_test(total_ones, total_bits);
    
    // Seuil du chi-carré pour 1 degré de liberté à 95% (3.841)
    const double CHI_SQUARED_THRESHOLD = 3.841;
    bool is_balanced = chi_squared < CHI_SQUARED_THRESHOLD;
    
    cout << "\n=== RESULTATS DE LA DISTRIBUTION ===" << endl;
    cout << "Nombre total de hashs analyses: " << hash_count << endl;
    cout << "Nombre total de bits analyses: " << total_bits << endl;
    cout << "Nombre de bits a 1: " << total_ones << endl;
    cout << "Nombre de bits a 0: " << total_bits - total_ones << endl;
    cout << "Pourcentage de bits a 1: " << fixed << setprecision(4) << percentage_ones << "%" << endl;
    cout << "Pourcentage de bits a 0: " << fixed << setprecision(4) << (100.0 - percentage_ones) << "%" << endl;
    cout << "Ecart a l'equilibre parfait: " << fixed << setprecision(4) << abs(50.0 - percentage_ones) << "%" << endl;
    cout << "Valeur du test chi-carre: " << fixed << setprecision(4) << chi_squared << endl;
    cout << "Seuil chi-carre (95%): " << CHI_SQUARED_THRESHOLD << endl;
    cout << "Distribution equilibree: " << (is_balanced ? "OUI" : "NON") << endl;
    
    // Analyse supplémentaire par octet
    cout << "\n=== ANALYSE PAR OCTET ===" << endl;
    cout << "Bits analyses par position de bit (0-255 dans le hash):" << endl;
    
    // Réinitialiser pour analyser par position
    total_bits = 0;
    vector<size_t> position_ones(256, 0);
    vector<size_t> position_count(256, 0);
    
    for (size_t i = 0; i < min(hash_count, static_cast<size_t>(1000)); ++i) {
        string message = generate_random_message(MESSAGE_LENGTH);
        uint32_t current_rule = rules[i % rules.size()];
        string hash = ac_hash(message, current_rule, STEPS);
        vector<bool> bits = hash_to_bits(hash);
        
        for (size_t j = 0; j < bits.size(); ++j) {
            if (bits[j]) position_ones[j]++;
            position_count[j]++;
        }
        total_bits += bits.size();
    }
    
    // Afficher les statistiques par position
    double max_deviation = 0;
    double min_percentage = 100, max_percentage = 0;
    
    for (size_t i = 0; i < 256; ++i) {
        if (position_count[i] > 0) {
            double pos_percentage = (position_ones[i] * 100.0) / position_count[i];
            double deviation = abs(50.0 - pos_percentage);
            
            if (pos_percentage < min_percentage) min_percentage = pos_percentage;
            if (pos_percentage > max_percentage) max_percentage = pos_percentage;
            if (deviation > max_deviation) max_deviation = deviation;
        }
    }
    
    cout << "Pourcentage min de 1 sur une position: " << fixed << setprecision(2) << min_percentage << "%" << endl;
    cout << "Pourcentage max de 1 sur une position: " << fixed << setprecision(2) << max_percentage << "%" << endl;
    cout << "Deviation maximale: " << fixed << setprecision(2) << max_deviation << "%" << endl;
    
    return 0;
}