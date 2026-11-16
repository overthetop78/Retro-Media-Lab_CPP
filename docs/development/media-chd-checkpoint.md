# Point de sauvegarde CHD

Les codecs communs sont implémentés : bitstream MSB, Huffman, DEFLATE brut, LZMA, Zstandard et FLAC sans STREAMINFO. Les bibliothèques de compression déjà embarquées sont réutilisées.

La compilation MediaEngine et le contrôle synthétique ciblé ont réussi. Celui-ci couvre les arbres Huffman, les consommations de flux, les CRC FLAC, les prédicteurs et la stéréo, les troncatures et l’annulation. L’audit des littéraux passe sur 1115 fichiers C++.

Les catalogues de diagnostics sont générés dans les 29 langues. Leur relecture et leurs corrections restent à terminer avant de déclarer ce bloc achevé. La compilation des catalogues mis à jour reste également à faire.

Le conteneur CHD, les cartes de hunks et l’intégration aux lecteurs ne sont pas encore portés. Ces commits constituent un point de sauvegarde du travail en cours.
