#  Atelier 2 — Automate Cellulaire et Fonction de Hachage dans la Blockchain (C++)

## Contexte

Atelier réalisé dans le cadre du module **Blockchain** du Master **Intelligence Artificielle & Science des Données** — FST Tanger.
Auteure : **Soumaya LAAKEL GAUZI** — Encadrement : **Pr. Ikram BENABDELOUAHAB**.

---

## Sommaire 

1. Implémentation de l’automate cellulaire 1D (r = 1)
2. Implémentation de `ac_hash()` basée sur l’automate
3. Intégration de `ac_hash()` dans la blockchain existante
4. Comparaison AC_HASH vs SHA256 (minage)
5. Analyse de l’effet avalanche pour AC_HASH
6. Analyse de la distribution des bits de AC_HASH
7. Tests multi-règles (30, 90, 110)
8. Avantages potentiels d’un hachage AC dans une blockchain
9. Faiblesses / vulnérabilités possibles
10. Proposition d’amélioration / variante
11. Résultats des tests (tableaux)
12. Mode d’exécution automatique (`run_tests.*`)

Les sections suivantes détaillent chacun de ces points (objectif, principe, résultats si fournis).

---

## 1) Automate cellulaire 1D (voisinage r = 1)

### Objectif

Implémenter en C++ un automate cellulaire 1D à états binaires et fournir des fonctions pour initialiser, faire évoluer et vérifier le comportement selon une règle élémentaire (Rule 30, 90, 110).

### Fonctions demandées

* `init_state(std::vector<int> bits)` — initialise l’état à partir d’un vecteur de bits.
* `rule_to_binary(int rule)` — convertit le numéro de règle (0–255) en table binaire de 8 sorties.
* `evolve(const std::vector<int>& state, const std::vector<int>& rule_table)` — applique la règle sur l’état (bord périodique).
* `display_state(...)` — visualisation simple (0/1).

### Principe de fonctionnement

* Pour chaque cellule, on lit (gauche, centre, droite) → forme un index 0..7 → lookup dans la table de la règle → nouvel état.
* Bords traités en mode circulaire (wrap-around) pour éviter effets de bord.

### Vérification (tests)

* Tests unitaires : motifs connus (ex. état initial `0001000`) et comparaison étapes attendues.
* Table de vérité complète : appliquer chaque triplet (000..111) et vérifier la sortie attendue.

### Résultats observés (extrait du rapport)

* Rule 30 reproduit le comportement chaotique attendu.
* Rule 90 produit le triangle de Sierpiński (symétrie).
* Rule 110 montre un comportement complexe (motifs mobiles et stables).
  Tous les tests de table de vérité et motifs connus sont corrects.

---

## 2) Fonction de hachage basée sur l’automate cellulaire

### 2.1 Fonction demandée (signature)

```cpp
std::string ac_hash(const std::string& input, uint32_t rule, size_t steps);
```

### 2.2 Mode de conversion du texte d’entrée en bits

* Chaque caractère du texte → ASCII (0..255).
* Chaque octet est converti en **8 bits MSB first** (par ex. `'A'` → `01000001`).
* Résultat : vecteur de bits (`std::vector<uint8_t>`) contenant 0/1.

### 2.3 Processus pour produire un hash fixe (256 bits)

Étapes principales :

1. **Conversion** du message en bit-stream.
2. **Padding** : ajout d’un bit `1`, zéros jusqu’à ce que la longueur soit compatible avec blocs 256 bits et réserve finale de 64 bits ; on concatène la longueur initiale (64 bits big-endian).
3. **Initialisation** d’un état interne de 256 bits (initialisé à 0).
4. **Construction** de la table de la règle (8 sorties) à partir du paramètre `rule`.
5. **Absorption bloc par bloc (256 bits)** : XOR du bloc avec l’état interne → appliquer `steps` itérations de l’automate (chaque itération : appliquer `evolve` sur l’état).
6. **Finalisation** : après absorption de tous les blocs, faire un nombre fixe d’itérations supplémentaires (ex. 10) pour améliorer diffusion.
7. **Conversion** de l’état final (256 bits) en hex (64 hex-chars) → sortie.

### 2.4 Vérification de non-collision (test simple)

* Test fourni : deux entrées différentes produisent deux outputs différents (aucune collision détectée dans les tests exécutés).

---

## 3) Intégration de `ac_hash()` dans la blockchain existante

### Objectifs

* Ajouter une option pour sélectionner le mode de hachage (`SHA256` ou `AC_HASH`).
* Modifier le code de minage pour utiliser `ac_hash()` si sélectionné.
* Vérifier que la validation de bloc (PoW) reste fonctionnelle.

### Principe d’intégration

* Ajout d’un flag / variable globale `currentHashMode` (ex. `SHA256_MODE`, `AC_HASH_MODE`).
* `Block::calculateHash()` appelle la fonction de hachage correspondante suivant le mode.
* Minage (PoW) : la boucle qui incrémente le nonce et calcule le hash utilise la fonction unifiée ; la comparaison de difficulté (préfixe de zéros) ne change pas.

### Vérification

* Tests réalisés montrent que `ac_hash()` s’intègre correctement et que les blocs sont validables avec le mécanisme PoW adapté.

---

## 4) Comparaison : AC_HASH vs SHA256 (minage)

### 4.1 Mesure du temps moyen de minage (10 blocs)

* **SHA256** : **0.1513 ms** (temps moyen par bloc sur 10 blocs).
* **AC_HASH** : **27.6592 ms** (temps moyen par bloc sur 10 blocs).

> AC_HASH est significativement plus lent (≈183× plus lent dans cette configuration).

### 4.2 Nombre moyen d’itérations pour trouver un hash valide

* **SHA256** : **17.1** itérations en moyenne.
* **AC_HASH** : **20.9** itérations en moyenne.

### 4.3 Résultats synthétiques (tableau extrait du rapport)

| Méthode | Temps moyen (ms) | Itérations moyennes | Temps min (ms) | Temps max (ms) |
| ------- | ---------------: | ------------------: | -------------: | -------------: |
| SHA256  |           0.1513 |                17.1 |          0.036 |          0.336 |
| AC_HASH |          27.6592 |                20.9 |          6.239 |         61.440 |

**Interprétation :** SHA256 reste beaucoup plus efficace en pratique. AC_HASH produit des sorties pseudo-aléatoires mais coûteux en temps CPU.

---

## 5) Analyse de l’effet avalanche de AC_HASH

### 5.1 Méthode

* Générer paires de messages ne différant que par **un seul bit** (ex. 400 paires).
* Calculer leurs hachages et compter le nombre de bits différents entre les deux hachages.
* Exprimer en pourcentage (bits différents / 256).

### 5.2 Résultats (extrait)

* **Nombre de tests :** 400
* **Collisions détectées :** 0
* **Pourcentage moyen de bits différents :** **33,04 %**
* **Min :** 27,15 % — **Max :** 39,65 %
* Distribution : ≈90 % des résultats dans [30 %, 40 %].

### Interprétation

* L’effet avalanche est **partiel** : la moyenne est loin des ~50 % attendus pour une fonction cryptographique idéale (ex. SHA256).
* Explications possibles : nombre d’itérations insuffisant, règle/absorption pas assez mélangées, ou topologie de l’automate pas optimale.

---

## 6) Analyse de la distribution des bits produits par AC_HASH

### 6.1 Méthode

* Générer de nombreux hashs, extraire au moins **≥ 10⁵ bits** et compter la proportion de `1`.
* Test du χ² pour vérifier l’équilibre statistique.

### 6.2 Résultats (extrait)

* **Bits analysés :** 100 096
* **Bits à 1 :** 49 878 → **49,83 %**
* **Bits à 0 :** 50 218 → **50,17 %**
* **χ² calculé :** 1,1549 < seuil 3,8410 (α=0,05) → pas de déviation statistiquement significative.

### Interprétation

* La distribution globale des bits est **équilibrée** (≈50 % de 1), ce qui est une bonne propriété malgré un effet avalanche incomplet.

---

## 7) Tests multi-règles (30, 90, 110)

### 7.1 Exécution

* Exécuter `ac_hash()` avec **Rule 30**, **Rule 90**, **Rule 110** sur ensembles de messages aléatoires.

### 7.2 Résultats comparatifs (extrait résumé)

| Règle    | Effet avalanche moyen (%) | Distribution 1 (%) | Temps d’exécution (ms) | Stabilité |
| -------- | ------------------------: | -----------------: | ---------------------: | --------: |
| Rule 30  |                     49.89 |              50.46 |                   3326 |    Stable |
| Rule 90  |                     49.81 |              50.15 |                   3170 |    Stable |
| Rule 110 |                     49.84 |              49.99 |                   2977 |    Stable |

> Ces valeurs proviennent d’un ensemble de tests spécifiques (voir rapport) ; elles montrent que, selon la configuration testée, les règles peuvent donner des avalanches proches de 50 % dans certains jeux de tests (différents tests ont donné des mesures différentes, voir sections 5 et 7 du rapport).

### 7.3 Conclusion

* **Rule 110** retenue comme **meilleur compromis** (complexité, diffusion et performances) pour le hachage expérimental, d’après les tests présentés.

---

## 8) Avantages potentiels d’un hachage AC dans une blockchain

* Simplicité conceptuelle (règles locales, opérations bit à bit).
* Potentiel de parallélisme élevé (SIMD / FPGA).
* Diversification algorithmique (alternative aux primitives classiques).
* Paramétrable (règle, voisinage, nombre d’itérations).

---

## 9) Faiblesses / vulnérabilités possibles

* Manque de preuves formelles (résistance aux collisions / préimages non garanties).
* Effet avalanche insuffisant dans certaines configurations → vulnérabilités exploitables.
* Performances nettement inférieures à SHA-family (coût CPU élevé).
* Paramètres mal choisis peuvent introduire biais statistiques.

---

## 10) Proposition d’amélioration / variante

* **AC-Hash+** : règle dynamique, voisinage variable (3/5/7), état 512 bits, finalisation plus longue (20 itérations), permutation initiale des octets, compression 512→256.
* Combinaison hybride : AC_HASH puis SHA256 (double mélange) pour tirer parti des deux mondes.

---

## 11) Résultats de tous les tests (tableaux)

(les tableaux ci-dessous reprennent les résultats clés extraits du rapport)

**Comparaison AC_HASH vs SHA256 (extrait)**

| Méthode | Temps moyen (ms) | Itérations moy. | Avalanche (%) (selon test) | Bits à 1 (%) |
| ------- | ---------------: | --------------: | -------------------------: | -----------: |
| SHA256  |           0.1513 |            17.1 |              ≈50 (attendu) |         50.1 |
| AC_HASH |          27.6592 |            20.9 |     33.04 (test avalanche) |        49.83 |

**Comparaison par règle (extrait section tests multi-règles)**

| Règle   | Avalanche (%) | Distribution 1 (%) | Temps (ms) |
| ------- | ------------: | -----------------: | ---------: |
| Rule 30 |         49.89 |              50.46 |       3326 |
| Rule 90 |         49.81 |              50.15 |       3170 |
| Rule110 |         49.84 |              49.99 |       2977 |

> Voir le PDF pour tableaux complets et jeux de données détaillés.

---

## 12) Mode d’exécution automatique (tests)

* Le rapport contient un script d’automatisation `run_tests.bat` (Windows) qui compile, exécute et collecte les résultats dans `results.txt`.

