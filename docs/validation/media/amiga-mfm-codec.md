# Validation du codec Amiga MFM — 15 novembre 2026

Le banc compile trente-cinq sources C# originales sans modification ni remplacement algorithmique, dont les quatre composants Amiga, encodage de base, codage MFM, construction/reconstruction de flux et modèles réels. Les empreintes SHA-256 correspondent à l’inventaire.

Les 2 449 cas comprennent une matrice exhaustive des 65 536 paires d’octets dans chacun des deux sens odd/even (empreintes comparées), 38 encodages et 38 décodages de buffers, 1 584 plages de parité, 72 empaquetages piste/face, 282 encodages de pistes, 203 corruptions/troncatures de pistes encodées, 216 cadres bruts et 15 séries d’intervalles.

Les encodages comparent nombre et SHA-256 des cellules 0/1, index et intervalles uint32 en ordre little-endian ; des allers-retours sont aussi comparés. Ils couvrent coordonnées/numéros limites, tailles incorrectes, piste vide, 11/22/255 secteurs et refus au-delà, durées nulles/extrêmes, métadonnées ignorées et ordre des erreurs. Le décodage compare tous les secteurs, données/octets, tags absents, codes, CRC enregistrés, intégrités, structures/descriptions et cellule estimée. Les mutations touchent synchronisation, bits d’horloge/données, information, label, octets inutilisés et parités indépendantes ; les coupes couvrent les frontières d’en-tête et de secteur. Les cadres bruts croisent longueur disponible, nombre de synchronisations et position initiale.

779 cas retournent un résultat et 1 670 sont refusés. Parmi les refus, 208 plages dont offset+count déborde int32 passent le premier garde C# puis lèvent ArgumentOutOfRangeException(index) pendant l’accès au tableau. Le C++ les refuse avec AmigaMfmParityRange avant accès. Le comparateur normalise uniquement ce diagnostic, pour une opération de parité, une somme non négative supérieure à int32 maximal et ce diagnostic C++ précis. Le manifeste garde les exceptions C# originales. Aucun écart fonctionnel n’est observé sur les données produites.

Onze contrôles supplémentaires vérifient l’annulation initiale des six points d’entrée, puis deux pistes synthétiques de 11/22 secteurs : intégrité/contenu, choix automatique du registre, collecte physique/adressée, écriture/lecture SCP et cache fichier. Les 145 textes des DLL des 29 langues sont comparés aux catalogues relus. Ces tests ne prouvent pas une acquisition matérielle ni les politiques de reconnaissance Amiga encore non portées.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **2 605 contrôles réussis**, 2 449 cas, 145 textes ; **208 écarts de diagnostic normalisés explicitement**, aucun écart fonctionnel.
- Audit : **423 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `0bd93d05705aa5d0265bf945e0d8b663f7dce2090a7efcb576a2cf9e42cde398`.

Le banc build/verification-amiga-mfm, captures synthétiques, binaires et journaux de comparaison sont supprimés après vérification. Aucun média utilisateur ou ajout au workflow GitHub.
