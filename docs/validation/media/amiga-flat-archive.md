# Validation des archives plates Amiga — 15 novembre 2026

Le banc compile directement les huit sources C# originales avec un adaptateur minimal des modèles/interfaces. Les huit empreintes sont inchangées. Les données sont synthétiques, produites temporairement ; aucun média utilisateur ni matériel n’est utilisé.

## Couverture

997 cas sont comparés au C# : blocs de 32 à 1 024 octets, blocs trop petits, noms ASCII et mutations sur les 256 valeurs d’octet, taille Reserved/ressources, limites de capacité, doublons, entrées vides, table et plages traversant les blocs, secteurs absents, courts et d’intégrité fausse/inconnue. Les 454 volumes lisibles sont comparés par identifiant, nom, capacité/espace libre, ordre et métadonnées d’entrées, SHA-256 du contenu et messages d’anomalies avec numéros de blocs. Les codes C++ sont projetés vers les libellés techniques C# pour comparer les diagnostics ; leur traduction est contrôlée séparément.

Les contrôles complémentaires vérifient les champs traduisibles du commentaire, l’annulation en reconnaissance/lecture et les limites de longueurs supérieures à 2 Gio avant allocation. Une image de capacité ADF réelle vérifie le trajet archive → exploration → analyse. Une image hybride reconnue à la fois comme AmigaDOS et archive prouve que la priorité C# est conservée.

Les cinq messages des 29 langues, soit 145 textes, sont lus depuis les DLL installées et comparés aux catalogues relus.

## Résultats

- Compilation complète et déploiement Debug et Release réussis, MSVC x64/Qt 6.12.
- Par configuration : **4 586 contrôles, aucune différence, 454 volumes lisibles et 145 textes traduits**.
- Audit des littéraux : 275 fichiers C++, 29 langues ; `git diff --check` passe.
- SHA-256 du manifeste de résultats C# : `d7581070571c09a551db3bb52d7c2ce77c99b3057204e98bf764f2f9d37f7a88`.

Le banc `build/verification-amiga-flat/`, ses fixtures, projets, exécutables et sorties sont supprimés après validation. Aucun test ajouté au workflow GitHub. La validation porte sur ce lecteur et son raccordement privé ADF, pas sur le portage complet des bibliothèques médias.
