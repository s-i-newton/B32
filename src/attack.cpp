#include "../include/attack.h"
#include <vector>
#include <cmath>

using namespace std;

Attack::Attack(vector<bitset<BLOC_LENGTH>> plaintexts_, vector<bitset<BLOC_LENGTH>> ciphertexts_, vector<bitset<BLOC_LENGTH>>SBox_)
    : LinearApproxMatrix(SBox_),  Chiffre(SBox_), plaintexts(plaintexts_), ciphertexts(ciphertexts_),
    one_active_pairs(vector<pair<unsigned int, unsigned int>>()), two_active_pairs(vector<pair<unsigned int, unsigned int>>())
{
    for(auto couple : minima)
    {
            if(isOneActiveBox(couple.second))
                one_active_pairs.push_back(couple);
            else
                two_active_pairs.push_back(couple);
    }

    for(auto couple : maxima)
    {
            if(isOneActiveBox(couple.second))
                one_active_pairs.push_back(couple);
            else
                two_active_pairs.push_back(couple);
    }
    for(unsigned int i=0; i<SBox_.size(); ++i)
        invSBox[SBox_[i].to_ulong()]=bitset<BLOC_LENGTH>(i);
}

/**
* Devine la clef K2 pour une ou deux actives boxes à la position donnée.
**/

unsigned int Attack::makeGuess(bool active_box, unsigned int position)
{
    unsigned int nb_keys = 1<<PIECE_LENGTH;
    vector<int> biais(nb_keys,0);
    vector<pair<unsigned int, unsigned int>> couples;
    if (active_box) /// Si on travaille avec une active box
        couples = one_active_pairs;
    else
        couples = two_active_pairs;
    for(unsigned int k = 0; k < couples.size(); ++k)
    {
        bitset<BLOC_LENGTH> A(couples[k].first);
        A = moveBitsets(A, PIECE_LENGTH*position);
        bitset<BLOC_LENGTH> PB(couples[k].second);
        PB = moveBitsets(PB, PIECE_LENGTH*position+2);
        for(unsigned int key = 0; key < nb_keys; ++key)
        {
            int frequency = 0;
            bitset<BLOC_LENGTH> K(key);
            K = moveBitsets(K,PIECE_LENGTH*position+2);
            for(unsigned int i = 0; i < ciphertexts.size(); ++i)
            {
                bitset<BLOC_LENGTH> x1 = depasse(K,ciphertexts[i]);
                if (produitScalaire(A,plaintexts[i]) == produitScalaire(PB,x1))
                    frequency++;
            }
            biais[key] += abs(frequency - static_cast<int>(ciphertexts.size())/2);
        }
    }

    /// Une bonne devinette maximise le biais
    unsigned int good_guess = 0;
    for(unsigned int key = 0; key < nb_keys; ++key)
        if (biais[key] > biais[good_guess])
            good_guess = key;

    return good_guess;
}

/**
* Grâce à la fonction précédente, on trouve toute la clef K2
**/

bitset<BLOC_LENGTH> Attack::findK2(bool active_box)
{
    bitset<BLOC_LENGTH> result(0);
    for(unsigned int position = 0; position < BLOC_LENGTH/PIECE_LENGTH; ++position)
    {
        unsigned int guess = makeGuess(active_box, position);
        bitset<BLOC_LENGTH> subkey(guess);
        subkey = moveBitsets(subkey,PIECE_LENGTH*position + 2);
        result |= subkey;
    }
    return result;
}

/**
* Construit tous les x1, une fois K2 trouvée
**/

void Attack::depasseCiphertexts(bitset<BLOC_LENGTH> K)
{
    for(unsigned int i = 0; i < ciphertexts.size(); ++i)
        ciphertexts[i] = depasse(K,ciphertexts[i]);
}

/**
* Vérification des relation de la question 9.
* On s'en sert également pour trouver K0 et K1
**/

bool Attack::checkKeys(bitset<BLOC_LENGTH> K0, bitset<BLOC_LENGTH> K1, bitset<BLOC_LENGTH> K2, unsigned int position)
{
    switch (position)
    {
    case 0:
        return ( (K0[28] == K2[14]) && (K0[29] == K2[14]) && (K0[30] == K2[15]) && (K0[31] == K2[19])
                 && (K1[26] == K2[15]) && (K1[28] == K2[14]) && (K1[29] == K2[16]) );
    case 1:
        return ( (K0[24] == K2[5]) && (K0[25] == K2[13]) && (K0[27] == K2[5])
                 && (K1[23] == K2[5]) && (K1[25] == K2[16]) );

    case 2:
        return ( (K0[22] == K2[22])
                 && (K1[18] == K2[14]) );

    case 3:
        return ( (K0[16] == K2[13]) && (K0[17] == K2[8]) && (K0[18] == K2[31])
                 && (K1[14] == K2[24]) && (K1[15] == K2[13]) && (K1[16] == K2[22]) && (K1[17] == K2[8])  );

    case 4:
        return ( (K0[13] == K2[19])
                 && (K1[10] == K2[8]) );

    case 5:
        return ( (K0[8] == K2[9]) && (K0[9] == K2[23]) && (K0[11] == K2[18])
                 && (K1[6] == K2[13]) && (K1[9] == K2[2])  );

    case 6:
        return ( (K0[4] == K2[13]) && (K0[5] == K2[1]) && (K0[6] == K2[5]) && (K0[7] == K2[12])
                 && (K1[2] == K2[8]) && (K1[39] == K2[1]) );

    case 7:
        return ( (K0[0] == K2[9]) && (K0[1] == K2[30]) && (K0[2] == K2[16]) && (K0[3] == K2[31])
                 && (K1[30] == K2[9]) && (K1[1] == K2[16]) );

    default:
        cerr << "position incorrecte :" << position;
        return 0;
    }
}

/**
* Trouve les bouts des clefs K0 et K1 (Question 9)
**/

pair<bitset<BLOC_LENGTH>, bitset<BLOC_LENGTH>>  Attack::findSubK0K1(bitset<BLOC_LENGTH> K2, unsigned int position)
{
    unsigned int nb_keys = static_cast<unsigned int>(pow(2.0,static_cast<double>(PIECE_LENGTH)));
    vector<unsigned int> check(nb_keys,0);
    for(unsigned int key = 0; key < nb_keys; ++key)
    {
        bitset<BLOC_LENGTH> K0(key); /// devine K0
        K0 = moveBitsets(K0, static_cast<unsigned int>(PIECE_LENGTH*position));
        for(unsigned int i = 0; i < ciphertexts.size(); ++i)
        {
            bitset<BLOC_LENGTH> K1 = passe(ciphertexts[i],plaintexts[i]^K0);
            if (checkKeys(K0,K1,K2,position))
                check[key]++;
        }
    }

    /// Trouver la sous-clé de K0 qui satisfait le plus de couples (c'est à dire tous les couples)
    unsigned int subkey0 = 0;
    for(unsigned int key = 0; key < nb_keys; ++key)
        if (check[key] > check[subkey0])
            subkey0 = key;

    /// Construction des deux sous clefs et retour de ces valeurs
    bitset<BLOC_LENGTH> good_K0(subkey0);
    good_K0 = moveBitsets(good_K0, static_cast<unsigned int>(PIECE_LENGTH*position)); // Décaler la sous-clé K0
    bitset<BLOC_LENGTH> good_K1 = passe(ciphertexts[0],plaintexts[0]^good_K0); // Calculer K1 (en utilisant nimporte quel couple (m,c)
    bitset<BLOC_LENGTH> masque(15);
    masque = moveBitsets(masque, PIECE_LENGTH*position+2);
    good_K1 &= masque; // Garder seulement les bits pouvant etre devinés
    return pair<bitset<BLOC_LENGTH>, bitset<BLOC_LENGTH>>(good_K0,good_K1) ;
}

/**
* Utilise K0, K1 et K2 pour retrouver la clef secrète K (Juste utiliser la table...)
**/

bitset<BLOC_LENGTH> Attack::buildSecretKey(bitset<BLOC_LENGTH> K0, bitset<BLOC_LENGTH> K1, bitset<BLOC_LENGTH> K2)
{
    bitset<BLOC_LENGTH> K(0);
    K[0] = K2[14];
    K[1] = K2[22];
    K[2] = K1[22];
    K[3] = K1[21];
    K[4] = K2[31];
    K[5] = K2[16];
    K[6] = K1[11];
    K[7] = K0[26];
    K[8] = K1[0];
    K[9] = K0[10];
    K[10] = K1[24];
    K[11] = K2[8];
    K[12] = K2[28];
    K[13] = K1[27];
    K[14] = K1[20];
    K[15] = K2[24];
    K[16] = K2[17];
    K[17] = K2[19];
    K[18] = K2[5];
    K[19] = K2[2];
    K[20] = K2[13];
    K[21] = K2[12];
    K[22] = K2[27];
    K[23] = K2[29];
    K[24] = K2[30];
    K[25] = K0[15];
    K[26] = K2[20];
    K[27] = K0[20];
    K[28] = K2[1];
    K[29] = K2[23];
    K[30] = K2[3];
    K[31] = K2[15];
    return K;
}

/**
* Combine les fonctions précédentes pour trouver toutes les clefs
**/

void Attack::findAllKeys(bool active_box)
{
    bitset<BLOC_LENGTH> K2 = findK2(active_box);
    depasseCiphertexts(K2);
    bitset<BLOC_LENGTH> K0(0);
    bitset<BLOC_LENGTH> K1(0);
    for(unsigned int position = 0; position < BLOC_LENGTH/PIECE_LENGTH; ++position)
    {
        pair<bitset<BLOC_LENGTH>, bitset<BLOC_LENGTH>> paire = findSubK0K1(K2,position);
        K0 |= paire.first;
        K1 |= paire.second;
    }
    bitset<BLOC_LENGTH> K = buildSecretKey(K0,K1,K2);
    cout << "K0 = " << K0 << endl;
    cout << "K1 = " << K1 << endl;
    cout << "K2 = " << K2 << endl;
    cout << "K = " << K << endl;
}

/**
* Deux fonctions de support
**/

/**
* Déplace un bloc de taille 4, pour deviner les blocs successifs
**/

std::bitset<BLOC_LENGTH> moveBitsets(std::bitset<BLOC_LENGTH> Key, unsigned int position)
{
    int shift = BLOC_LENGTH - static_cast<int>(position) - PIECE_LENGTH;
    if (shift >= 0)
        return (Key<<static_cast<size_t>(shift))|(Key>>(BLOC_LENGTH-static_cast<size_t>(shift)));
    else
        return (Key>>static_cast<size_t>(-shift)|(Key<<(BLOC_LENGTH+static_cast<size_t>(shift))));
}

/**
* Test si on a une active box
**/

bool inline isOneActiveBox(unsigned int b) // hypothese : 0 < b < 16
{
    return ((b < 4) || (b % 4 == 0));
}
