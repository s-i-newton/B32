#include "../include/chiffre.h"
#include<cmath>
#include<iostream>

using namespace std;
/**
* Déchiffre un texte chiffré
**/

vector<bitset<BLOC_LENGTH>> Chiffre::dechiffrer(vector<bitset<BLOC_LENGTH>> ciphertext)
{
    vector<bitset<BLOC_LENGTH>> plaintext(ciphertext.size());

    for(unsigned int i=0;i<ciphertext.size();++i)
        plaintext[i]=dechiffrerBloc(ciphertext[i]);

    return plaintext;
}

/**
* Déchiffre un bloc de 32 bits
**/

bitset<BLOC_LENGTH> Chiffre::dechiffrerBloc(bitset<BLOC_LENGTH> ciphertext)
{
    for(size_t i=clefs.size()-1;i>0;--i)
        ciphertext=depasse(clefs[i], ciphertext);

    ciphertext = ciphertext^clefs[0];
    return ciphertext;
}

/**
* Fait une passe de déchiffrement
**/

bitset<BLOC_LENGTH> Chiffre::depasse(bitset<BLOC_LENGTH> clef, bitset<BLOC_LENGTH> input)
{
    input ^= clef;
    input=((input>>(BLOC_LENGTH-2))|(input<<2));
    size_t nbPiece = input.size()/static_cast<size_t>(PIECE_LENGTH);
    bitset<BLOC_LENGTH> output=0;

    for(size_t i=0;i<nbPiece;++i)
    {
        bitset<BLOC_LENGTH> masque = creerMasque(PIECE_LENGTH*i, PIECE_LENGTH*(i+1)-1);
        bitset<BLOC_LENGTH> piece = input & masque;
        piece >>= PIECE_LENGTH*i;
        piece = invSBox[piece.to_ulong()];
        piece <<= PIECE_LENGTH*i;
        output |= piece;
    }

    return output;
}

/**
* Chiffre un texte
**/

vector<bitset<BLOC_LENGTH>> Chiffre::chiffrer(vector<bitset<BLOC_LENGTH>> plaintext)
{
    vector<bitset<BLOC_LENGTH>> ciphertext(plaintext.size());

    for(unsigned int i=0;i<plaintext.size();++i)
        ciphertext[i]=chiffrerBloc(plaintext[i]);

    return ciphertext;
}

/**
* Chiffre un texte de 32 bits
**/

bitset<BLOC_LENGTH> Chiffre::chiffrerBloc(bitset<BLOC_LENGTH> plaintext)
{
    plaintext = plaintext^clefs[0];
    for(unsigned int i=1;i<clefs.size();++i)
        plaintext=passe(clefs[i], plaintext);

    return plaintext;
}

/**
* Fais une passe de chiffrement
**/

bitset<BLOC_LENGTH> Chiffre::passe(bitset<BLOC_LENGTH> clef, bitset<BLOC_LENGTH> input)
{
    size_t nbPiece = (input.size())/static_cast<size_t>(PIECE_LENGTH);
    bitset<BLOC_LENGTH> output=0;

    for(size_t i=0;i<nbPiece;++i)
    {
        bitset<BLOC_LENGTH> masque = creerMasque(PIECE_LENGTH*i, PIECE_LENGTH*(i+1)-1);
        bitset<BLOC_LENGTH> piece = input & masque;
        piece >>= PIECE_LENGTH*i;
        piece = SBox[piece.to_ulong()];
        piece <<= PIECE_LENGTH*i;
        output |= piece;
    }

    return ((output<<BLOC_LENGTH-2)|(output>>2))^clef;
}

/**
* Fonction auxiliaire qui crée un masque de '1' entre les positions début et fin
**/

bitset<BLOC_LENGTH> Chiffre::creerMasque(size_t debut, size_t fin)
{
    bitset<BLOC_LENGTH> sortie=0;
    for(size_t i=debut; i<=fin; ++i)
        sortie |= bitset<BLOC_LENGTH>(1)<<i;

    return sortie;
}

/**
* Constructeur pour se servir du chiffre
**/

Chiffre::Chiffre(vector<bitset<BLOC_LENGTH>> clefs_, vector<bitset<BLOC_LENGTH>>SBox_) :
clefs(clefs_), SBox(SBox_), invSBox(vector<bitset<BLOC_LENGTH>>(SBox_.size()))
{
    for(unsigned int i=0;i<SBox.size();++i)
        invSBox[SBox[i].to_ulong()]=bitset<BLOC_LENGTH>(i);
}

/**
* Constructeur pour l'attaque. Les clefs sont inconnues
**/

Chiffre::Chiffre(vector<bitset<BLOC_LENGTH>> SBox_) :
clefs(vector<bitset<BLOC_LENGTH>>()), SBox(SBox_), invSBox(vector<bitset<BLOC_LENGTH>>(SBox_.size()))
{
    for(unsigned int i=0;i<SBox.size();++i)
        invSBox[SBox[i].to_ulong()]=bitset<BLOC_LENGTH>(i);
}


