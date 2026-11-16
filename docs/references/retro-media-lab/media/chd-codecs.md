# Codecs communs CHD

Référence C# en lecture seule : ChdBitstream, ChdHuffman, ChdFlac, ChdDeflate, ChdLzma et les cinq sources ChdZstd. Les empreintes sont vérifiées dans l’inventaire.

Bitstream gère les lectures MSB de 0 à 32 bits avec un accumulateur 64 bits, y compris les lectures non alignées de 32 bits. Le contrat de consommation et le remplissage par zéros sont conservés. Huffman possède ses tables canoniques et importe les arbres RLE ou Huffman.

Compression regroupe les contextes réutilisables zlib, liblzma et Zstandard déjà embarqués. Les implémentations C# de ces algorithmes sont remplacées par ces bibliothèques natives ; il ne s’agit pas d’une transposition ligne à ligne. Les flux DEFLATE sont bruts, leur sortie doit être exacte et les octets après fin de flux sont tolérés comme en C#. LZMA reprend les paramètres lc/lp/pb et l’arrondi du dictionnaire CHD, accepte le marqueur terminal facultatif et impose une consommation complète. Zstandard accepte les trames concaténées et skippables, vérifie les checksums et refuse les dictionnaires externes.

FLAC lit les trames sans STREAMINFO, les sous-trames constantes/brutes/fixed/LPC, les résidus Rice, les bits inutilisés, les CRC et les quatre modes stéréo. Le nombre d’octets consommés permet le raccordement ultérieur du sous-code CD. Les opérations sur les prédicteurs évitent les débordements signés indéfinis du C++.

Les contextes natifs sont possédés par RAII ; les buffers sont réutilisés et l’annulation est propagée. Un contexte de décompression est destiné à une lecture de hunk sérialisée, pas à des appels simultanés.

Le conteneur CHD, les cartes, les métadonnées et le réassemblage CD ne font pas partie de ce bloc.
