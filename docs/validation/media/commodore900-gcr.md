# Validation du codec Commodore 900 GCR — 15 novembre 2026

## Méthode

Le banc compile 39 fichiers C# originaux, sans modifier le dépôt source. Vingt-deux sources et contrats concernés correspondent aux SHA-256 de l’inventaire. Les fixtures sont synthétiques et temporaires ; aucun média utilisateur ou test matériel.

1 748 cas de référence couvrent les cylindres zéro, frontières de zones, 80 et 255, les deux faces, les numéros jusqu’à 255, les quantités de secteurs, les secteurs répétés, champs ignorés, tailles invalides et bornes de temporisation. Les pistes synthétiques vérifient synchronisations, marques erronées, troncatures, corruption GCR/checksum, en-têtes sans données, données sans en-tête, appariement et adjacence stricte. Les primitives couvrent les 256 valeurs d’octet, les chaînes binaires/hexadécimales valides ou invalides, FM, MFM, double FM, gaps et représentations compactes.

Les bits, intervalles, identifiants, index, octets décodés, positions et longueurs de structures sont comparés exactement. Les secteurs comprennent toutes leurs métadonnées et états facultatifs, charges utiles, tags et ECC. Les descriptions sont résolues en anglais et comparées aux descriptions C#. Les valeurs flottantes finies utilisent une tolérance relative de 1e-12.

Les contrôles supplémentaires C++ vérifient les diagnostics, l’annulation préalable de l’encodage/décodage et la résolution d’une même structure en français puis en anglais sans relancer son décodage. Les seize clés des 29 langues sont chargées depuis les DLL installées et comparées aux catalogues, soit 464 textes. Cela ne prétend pas valider une annulation en cours par une autre tâche ni les aides ISO sans consommateur porté.

## Résultats

- Compilation complète et déploiement Debug et Release réussis avec MSVC x64 et Qt 6.12.
- Dans chaque configuration : **3 963 contrôles, aucune différence, 89 pistes encodées, 62 cas de flux et 464 textes traduits**.
- Audit : 313 fichiers C++, 56 clés d’erreur, 29 langues ; aucun littéral applicatif hors constantes nommées.
- SHA-256 du manifeste de résultats C# : `7cf052cf253c028bc67c0596d1bc9eb2f510860b6b13dd95ce67f6c9e2e6c0b6`.

Le banc `build/verification-commodore900-gcr/` et ses journaux temporaires sont supprimés après validation, avec projets, exécutables, fixtures et références. Aucun test ajouté aux workflows GitHub. Les registres, conversions et chaînes de fichiers de flux restent à porter.
