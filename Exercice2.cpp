

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <limits>

using namespace std;

// ---------------------------- Utilitaires ----------------------------

// Convertit une chaîne en vecteur de bits (MSB first par octet)
vector<uint8_t> string_to_bits(const string &s) {
    vector<uint8_t> bits;
    for (unsigned char uc : s) {
        for (int i = 7; i >= 0; --i) {
            bits.push_back((uc >> i) & 1);
        }
    }
    return bits;
}

// Converti uint64_t -> 64 bits big-endian (MSB first)
vector<uint8_t> u64_to_bits_be(uint64_t x) {
    vector<uint8_t> bits(64);
    for (int i = 0; i < 64; ++i)
        bits[63 - i] = ((x >> i) & 1) ? 1 : 0;
    return bits;
}

// Convertit un vecteur de 256 bits (MSB first) en chaine hex (64 hex chars)
string bits256_to_hex(const vector<uint8_t> &state) {
    assert(state.size() == 256);
    stringstream ss;
    ss << hex << nouppercase;
    for (size_t i = 0; i < 256; i += 8) {
        unsigned int byte = 0;
        for (size_t j = 0; j < 8; ++j) {
            byte = (byte << 1) | (state[i + j] & 1);
        }
        ss << setw(2) << setfill('0') << (byte & 0xFF);
    }
    return ss.str();
}

// ---------------------- Automate cellulaire (r = 1) --------------------

// table de la règle : table[pattern] = bit (pattern = left<<2 | center<<1 | right)
vector<uint8_t> rule_to_table(uint32_t rule_number) {
    vector<uint8_t> table(8);
    for (int p = 0; p < 8; ++p)
        table[p] = ((rule_number >> p) & 1) ? 1 : 0;
    return table;
}

// Evolve one step on an arbitrary-length state (periodic boundary)
vector<uint8_t> evolve_once(const vector<uint8_t> &state, const vector<uint8_t> &rule_table) {
    size_t n = state.size();
    vector<uint8_t> next(n, 0);
    for (size_t i = 0; i < n; ++i) {
        uint8_t left   = state[(i + n - 1) % n];
        uint8_t center = state[i];
        uint8_t right  = state[(i + 1) % n];
        int pattern = (left << 2) | (center << 1) | right;
        next[i] = rule_table[pattern];
    }
    return next;
}

// Applique 'steps' évolutions in-place sur state (modifie et renvoie)
void evolve_steps(vector<uint8_t> &state, const vector<uint8_t> &rule_table, size_t steps) {
    for (size_t s = 0; s < steps; ++s)
        state = evolve_once(state, rule_table);
}

// --------------------------- Fonction ac_hash -------------------------
string ac_hash(const string& input, uint32_t rule, size_t steps) {
    // 1) Conversion du message en bits
    vector<uint8_t> msg_bits = string_to_bits(input);
    uint64_t original_len_bits = msg_bits.size();

    // 2) Padding (similaire à SHA)
    vector<uint8_t> padded = msg_bits;
    padded.push_back(1); // bit '1' de marqueur
    size_t rem = ((original_len_bits + 1 + 64) % 256);
    size_t k = (rem == 0) ? 0 : (256 - rem);
    for (size_t i = 0; i < k; ++i) padded.push_back(0);

    // Ajout de la longueur sur 64 bits (big-endian)
    vector<uint8_t> len_bits = u64_to_bits_be(original_len_bits);
    padded.insert(padded.end(), len_bits.begin(), len_bits.end());
    assert(padded.size() % 256 == 0);

    // 3) État interne 256 bits initialisés à 0
    vector<uint8_t> state(256, 0);

    // 4) Table de règle
    vector<uint8_t> rule_table = rule_to_table(rule);

    // 5) Absorption bloc par bloc
    for (size_t block = 0; block < padded.size(); block += 256) {
        for (size_t i = 0; i < 256; ++i)
            state[i] ^= padded[block + i];
        evolve_steps(state, rule_table, steps);
    }

    // 6) Finalisation : diffusion supplémentaire
    const size_t FINAL_STEPS = 10;
    evolve_steps(state, rule_table, FINAL_STEPS);

    // 7) Conversion en hex
    return bits256_to_hex(state);
}

// ------------------------- Tests utilitaires --------------------------

void test_consistency() {
    cout << "=== Test de consistance ===" << endl;
    string input = "HY HASH_AC TEST";
    uint32_t rule = 110;
    size_t steps = 100;
    string h1 = ac_hash(input, rule, steps);
    string h2 = ac_hash(input, rule, steps);
    cout << "Input: \"" << input << "\"\nHash1: " << h1 << "\nHash2: " << h2 << "\n";
    cout << (h1 == h2 ? "Consistent: PASS\n" : "Inconsistent: FAIL\n") << endl;
}

void test_collisions_simple() {
    cout << "=== Test collisions simples ===" << endl;
    vector<pair<string,string>> cases = {
        {"POW","POS"},
        {"a","b"},
        {"test1","test2"},
        {""," "},
        {"abc","abcd"}
    };
    uint32_t rule = 110;
    size_t steps = 100;
    for (auto &p : cases) {
        string h1 = ac_hash(p.first, rule, steps);
        string h2 = ac_hash(p.second, rule, steps);
        cout << "A: \"" << p.first << "\" -> " << h1 << "\nB: \"" << p.second << "\" -> " << h2 << "\n";
        cout << (h1 == h2 ? " COLLISION DETECTEE!\n\n" : " Pas de collision (diff)\n\n");
    }
}

// ------------------------------ main --------------------------------

int main() {
    int choice;
    do {
        cout << "\n=== Hachage par Automate Cellulaire ===\n";
        cout << "1. Hasher une chaine\n";
        cout << "2. Tester collisions simples\n";
        cout << "3. Tester consistance\n";
        cout << "0. Quitter\n";
        cout << "Votre choix: ";
        if (!(cin >> choice)) { cin.clear(); cin.ignore(); continue; }

        if (choice == 0) break;

        switch (choice) {
            case 1: {
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                string input;
                uint32_t rule;
                size_t steps;
                cout << "Entrez la chaine a hacher: ";
                getline(cin, input);
                cout << "Entrez la regle (p.ex. 30, 90, 110): ";
                cin >> rule;
                cout << "Entrez le nombre d'etapes par bloc (p.ex. 10..200): ";
                cin >> steps;
                string digest = ac_hash(input, rule, steps);
                cout << "Digest (256-bit hex): " << digest << "\n";
                break;
            }
            case 2:
                test_collisions_simple();
                break;
            case 3:
                test_consistency();
                break;
            default:
                cout << "Choix invalide\n";
                break;
        }
    } while (true);

    cout << "Au revoir!\n";
    return 0;
}
