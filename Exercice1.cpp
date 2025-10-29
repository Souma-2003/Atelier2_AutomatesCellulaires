#include <iostream>
#include <vector>
#include <string>
#include <cassert>
using namespace std;

// 1.1 - Fonction pour initialiser l'état à partir d'un vecteur de bits
vector<int> init_state(const vector<int>& initial_bits) {
    return initial_bits;
}

// Fonction pour convertir la règle en tableau binaire
vector<int> rule_to_binary(int rule_number) {
    vector<int> rule_bits(8);
    for (int i = 0; i < 8; ++i) {
        rule_bits[7 - i] = (rule_number >> i) & 1;
    }
    return rule_bits;
}

// 1.2 - Fonction evolve() : applique une règle sur tout le vecteur
vector<int> evolve(const vector<int>& state, int rule_number) {
    int n = state.size();
    vector<int> next_state(n, 0);
    vector<int> rule_bits = rule_to_binary(rule_number);

    for (int i = 0; i < n; ++i) {
        int left   = (i == 0) ? state[n - 1] : state[i - 1];
        int center = state[i];
        int right  = (i == n - 1) ? state[0] : state[i + 1];

        int pattern = (left << 2) | (center << 1) | right;
        next_state[i] = rule_bits[7 - pattern];
    }
    return next_state;
}

// Affichage de l'état avec 0 et 1
void display_state(const vector<int>& state) {
    for (int cell : state)
        cout << (cell ? '1' : '0');
    cout << endl;
}

// MÉTHODES DE VÉRIFICATION

// 1. Vérification par tests unitaires avec motifs connus
bool verify_known_patterns(int rule) {
    cout << "\n=== Verification Rule " << rule << " ===" << endl;
    
    // Test avec un seul 1 au centre
    vector<int> test_state = {0, 0, 0, 1, 0, 0, 0};
    vector<int> current_state = test_state;
    
    cout << "Test 1 - Single 1 pattern:" << endl;
    cout << "Initial: ";
    display_state(current_state);
    
    // Appliquer une évolution et vérifier les résultats connus
    current_state = evolve(current_state, rule);
    cout << "Step 1:  ";
    display_state(current_state);
    
    bool passed = true;
    
    // Vérifications spécifiques selon la règle
    switch(rule) {
        case 30: // Rule 30 - Comportement chaotique
            // Après une étape, on devrait avoir un motif spécifique
            if (current_state[2] == 1 && current_state[3] == 1 && current_state[4] == 1) {
                cout << " Rule 30 pattern verified" << endl;
            } else {
                cout << " Rule 30 pattern mismatch" << endl;
                passed = false;
            }
            break;
            
        case 90: // Rule 90 - Triangle de Sierpinski
            // Pattern caractéristique
            if (current_state[2] == 1 && current_state[4] == 1) {
                cout << " Rule 90 pattern verified" << endl;
            } else {
                cout << " Rule 90 pattern mismatch" << endl;
                passed = false;
            }
            break;
            
        case 110: // Rule 110 - Règle universelle
            // Pattern caractéristique
            if (current_state[2] == 1 && current_state[3] == 1) {
                cout << " Rule 110 pattern verified" << endl;
            } else {
                cout << " Rule 110 pattern mismatch" << endl;
                passed = false;
            }
            break;
    }
    
    return passed;
}

// 2. Vérification par table de vérité complète
bool verify_truth_table(int rule) {
    cout << "\n=== Verification par table de verite ===" << endl;
    
    vector<int> rule_bits = rule_to_binary(rule);
    bool all_correct = true;
    
    // Tester tous les voisinages possibles (000 à 111)
    for (int pattern = 0; pattern < 8; pattern++) {
        // Reconstruire le voisinage à partir du pattern
        int left = (pattern >> 2) & 1;
        int center = (pattern >> 1) & 1;
        int right = pattern & 1;
        
        // Créer un état minimal pour tester ce voisinage
        vector<int> test_state = {left, center, right};
        vector<int> result = evolve(test_state, rule);
        
        // Le résultat devrait être rule_bits[7-pattern]
        int expected = rule_bits[7 - pattern];
        
        if (result[1] != expected) { // On vérifie la cellule centrale
            cout << "Erreur pattern " << left << center << right 
                 << " attendu: " << expected << ", obtenu: " << result[1] << endl;
            all_correct = false;
        } else {
            cout << " Pattern " << left << center << right 
                 << " -- " << expected << "  Correct" << endl;
        }
    }
    
    return all_correct;
}

// 3. Vérification par comparaison avec des résultats prédéfinis
bool verify_against_expected(int rule, const vector<int>& initial, 
                           const vector<vector<int>>& expected_steps, int steps) {
    cout << "\n=== Verification par comparaison ===" << endl;
    
    vector<int> current = initial;
    bool all_correct = true;
    
    for (int i = 0; i < steps && i < expected_steps.size(); i++) {
        current = evolve(current, rule);
        
        if (current != expected_steps[i]) {
            cout << " Etape " << (i+1) << " incorrecte" << endl;
            cout << "  Attendu: ";
            display_state(expected_steps[i]);
            cout << "  Obtenu:  ";
            display_state(current);
            all_correct = false;
        } else {
            cout << " Etape " << (i+1) << " correcte: ";
            display_state(current);
        }
    }
    
    return all_correct;
}

// Fonction pour afficher le menu
void display_menu() {
    cout << "\n=== Automate cellulaire 1D ===" << endl;
    cout << "Choisissez une regle :" << endl;
    cout << "30 - Rule 30 (comportement chaotique)" << endl;
    cout << "90 - Rule 90 (triangle de Sierpinski)" << endl;
    cout << "110 - Rule 110 (universelle)" << endl;
    cout << "0 - Quitter le programme" << endl;
    cout << "Votre choix : ";
}

int main() {
    int rule;
    
    do {
        display_menu();
        cin >> rule;
        
        if (rule == 0) {
            cout << "Au revoir !" << endl;
            break;
        }
        
        if (rule != 30 && rule != 90 && rule != 110) {
            cout << "Regle invalide ! Choisissez 30, 90 ou 110." << endl;
            continue;
        }
        
        // État initial
        vector<int> initial_bits = {0, 0, 0, 1, 0, 0, 0};
        vector<int> state = init_state(initial_bits);
        
        // VÉRIFICATIONS AUTOMATIQUES
        cout << "\n*** DEBUT DES VERIFICATIONS ***" << endl;
        
        bool test1 = verify_known_patterns(rule);
        bool test2 = verify_truth_table(rule);
        
        // Résultats des vérifications
        cout << "\n*** RESULTATS DES VERIFICATIONS ***" << endl;
        cout << "Test motifs connus: " << (test1 ? "PASSE" : "ECHOUE") << endl;
        cout << "Test table verite: " << (test2 ? "PASSE " : "ECHOUE") << endl;
        
        if (test1 && test2) {
            cout << "\n L'automate reproduit correctement la Rule " << rule << " !" << endl;
        } else {
            cout << "\n Problemes detectes avec la Rule " << rule << " !" << endl;
        }
        
        // Simulation normale
        cout << "\n=== Simulation Rule " << rule << " ===" << endl;
        cout << "Initial: ";
        display_state(state);
        
        for (int step = 1; step <= 10; ++step) {
            state = evolve(state, rule);
            cout << "Step " << step << ":  ";
            display_state(state);
        }
        
        cout << "\nAppuyez sur Entree pour continuer...";
        cin.ignore();
        cin.get();
        
    } while (true);
    
    return 0;
}