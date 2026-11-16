# Validation ciblée VHD

Le 16 novembre 2026 : compilation Debug MediaEngine réussie avec MSVC 19.44 (`build/vhd-build.log`), puis compilation incrémentale après correction explicite de la conversion du secteur size_t vers int32 (`build/vhd-final-build.log`). Les 30 DLL Constants/langues sont construites (`build/vhd-catalogs.log`).

Un programme natif temporaire a vérifié :

- lecture et écriture fixes/dynamiques sur blocs stockés, nuls et dernier bloc logique partiel ; reconstruction complète des octets ;
- BAT inutilisée pour le bloc nul, bitmaps et secteur absent ramené à zéro ; image entièrement nulle de 2560 octets ;
- copies initiale/finale du footer identiques et sommes de contrôle recalculées indépendamment dans le programme ;
- UUID imposé octet par octet dans l'ordre big-endian, géométrie CHS attendue, choix fixe via l'enum numérique avec espaces ;
- rejets pour footer/checksum/version, taille nulle, parent différentiel, type, bloc dynamique, couverture BAT, décalage/table hors plage, signature et checksum dynamiques, longueur fixe incorrecte ;
- annulation sans modification d'une destination existante.

Résultat : `PASS VHD fixed/dynamic/BAT/bitmaps/UUID/footer checksums/geometry/partial blocks/rejections/cancellation`, dans `build/vhd-smoke.log`. Aucun média utilisateur ni ancienne suite de codecs n'ont été utilisés. Le programme, ses données synthétiques et ses binaires sont supprimés après consignation.

Les 22 diagnostics sont générés par Argos dans les 29 langues, relus puis corrigés. Les messages équivalents utilisent les traductions VDI déjà relues. Les contresens sur cookie/footer, les checksums, décalages et parents différentiels sont corrigés ; VHD et BAT sont protégés comme invariants. Les unités en toutes lettres ne sont pas transformées en invariants anglais. Audit statique réussi sur 1043 sources C++ et 29 langues (`build/vhd-audit.log`).

Ces résultats ne prouvent pas l'intégration complète depuis App ni le fonctionnement Linux/macOS. Les autres conteneurs restent à porter.
