# Validation des primitives RX02 M²FM — 15 novembre 2026

Le banc compile quinze fichiers C# originaux inchangés. Leurs SHA-256 correspondent à l’inventaire. L’empreinte du manifeste est `ace36067f7c3615603b3ebf0c820a7d49bfd4bb35f2b63eb9dc35883bdd10dfb`.

Les 5 434 cas comprennent 4 127 transformations en place, 777 encodages suivis de décodages, 256 recherches de marque, 249 décodages directs, douze sélections par taille, douze sélections de marque et la comparaison complète des définitions. Les sept marques FM sont acceptées avec leurs quatre octets exacts et les 249 autres valeurs refusées. Les tailles 128/256, variantes supprimées et valeurs d’énumération hors domaine sont comparées au comportement source.

Les transformations couvrent les 2 048 motifs possibles de onze cellules aux deux parités, ainsi que répétitions, chevauchements, séquences vides et extrémités. Les 256 octets sont traités en séquences de un, trois et 258 octets ; des séquences déterministes de tailles zéro à 1 024 vérifient aussi la conversion complète. Les 777 conversions encodage/décodage retournent des octets et sont comparées sur toutes les cellules et tous les octets reconstruits.

Les décodages directs examinent les troncatures, positions négatives et extrêmes, compte nul/négatif/débordant, paires 11 invalides à toutes les positions et séquences aléatoires. Le C# retourne 68 suites d’octets, 146 absences et 35 exceptions pour ces 249 lectures. Au total, la référence produit 5 140 résultats et 294 refus (249 ArgumentOutOfRangeException pour les marques, 24 OverflowException pour les comptes, onze IndexOutOfRangeException et dix ArgumentException pour les tailles).

Neuf différences de validation sont attendues et vérifiées explicitement : six comptes invalides qui pouvaient retourner l’absence en C# sont systématiquement refusés par le diagnostic de compte C++ ; trois positions positives extrêmes provoquant le débordement int32 de start + longueur en C# retournent proprement l’absence en C++. Les conditions arithmétiques sont vérifiées par le banc, sans ignorer arbitrairement des numéros de cas. Aucun autre écart n’est accepté. Les départs négatifs non vides conservent un diagnostic explicite ; la lecture vide à départ négatif conserve le résultat vide source.

Deux contrôles vérifient l’annulation ; deux autres refusent les cellules non binaires que le type bool C# ne peut pas représenter. Les 87 textes des DLL des 29 langues correspondent aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64 et Qt 6.12.
- Chaque configuration : **5 525 contrôles, 0 écart inattendu, 6 gardes de compte, 3 bornes sans débordement, 5 434 cas, 87 textes**.
- Audit : **495 fichiers C++, 29 langues**, réussi.

Le banc build/verification-rx02-m2fm et ses journaux sont supprimés après vérification. Aucun média utilisateur ni test dans le workflow GitHub. Les pistes RX02 complètes, descriptions et politiques restent à porter séparément.
