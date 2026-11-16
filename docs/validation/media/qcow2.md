# Validation ciblée QCOW2

Le 16 novembre 2026, compilation Debug MediaEngine réussie sous MSVC 19.44, sans avertissement observé (`build/qcow2-build.log`).

Le programme temporaire vérifie deux sorties, versions 2 et 3, avec clusters de données non nuls, cluster intermédiaire explicitement nul et dernier cluster logique partiel. Les octets produits sont contrôlés aux emplacements L1/L2 attendus, les clusters physiques alloués portent les références égales à un, la référence suivante reste nulle et le remplissage du dernier cluster est nul. La taille physique est celle des métadonnées et des deux clusters non nuls.

Les images relues restituent exactement le contenu logique et les métadonnées. Une entrée L1 supprimée donne une plage non allouée relue en zéros. Les profils exclus ou structures endommagées sont refusés : en-tête court, signature, version, parent, taille des clusters/disque, chiffrement, snapshots, fonctionnalités incompatibles/autoclear, ordre de références, longueur d'en-tête, bloc de références absent, compteur nul, compression, cluster hors fichier et décalage désaligné.

L'annulation conserve le fichier de destination préexistant. Un document provenant d'un autre format choisit la version 3 même si ses métadonnées contiennent une version 2.

Résultat : `PASS QCOW2 v2/v3 sparse tables/refcounts/logical ranges/profile rejections/atomic cancellation` (`build/qcow2-smoke.log`). Aucun média utilisateur ni ancienne suite de codecs utilisé. Programme, données et binaires temporaires supprimés après consignation.

Les 23 erreurs sont traduites avec Argos dans les 29 langues, relues et corrigées, notamment les compteurs de références, négations, clusters compressés et termes restés en anglais. Les 30 DLL Constants/langues sont construites (`build/qcow2-catalogs.log`). Audit statique réussi sur 1085 fichiers C++ et 29 langues (`build/qcow2-audit.log`). La validation ne prouve pas la composition globale des services ou leur intégration à App.
