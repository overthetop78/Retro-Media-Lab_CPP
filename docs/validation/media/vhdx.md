# Validation ciblée VHDX

Le 16 novembre 2026, compilation Debug MediaEngine réussie sous MSVC 19.44, sans avertissement observé (`build/vhdx-build.log`). La compilation finale nomme les indices et multiplicateurs signalés par l'audit (`build/vhdx-final-build.log`).

Un programme natif temporaire vérifie la primitive CRC32C avec le vecteur 123456789 égal à E3069283. Les contrôles d'en-têtes et régions produits sont recalculés par un parcours bit à bit indépendant de la table de production. L'ordre binaire du GUID BAT est comparé aux octets du GUID C#.

Les sorties Fixed et Dynamic, les secteurs 512/4096, les métadonnées, la longueur minimale d'une sortie Dynamic nulle et la position du deuxième bloc logique présent après un premier bloc nul sont contrôlés. Le dernier bloc partiel restitue ses données. La copie de secours des en-têtes et régions fonctionne ; seul le journal de l'en-tête sélectionné par séquence exige une relecture.

Les rejets vérifiés couvrent signatures/CRC des copies, journal actif, régions obligatoires inconnues, métadonnées absentes/inconnues/hors région, tailles de blocs/secteurs/disque invalides, parent, états BAT invalides, bloc partiel et charge utile hors fichier. Les états NotPresent, Undefined et Unmapped restent non alloués. Un cas à capacité virtuelle supérieure à 4 GiB modifie uniquement les métadonnées/BAT d'une petite image synthétique : il vérifie le saut de l'emplacement de bitmap sans créer un média de plusieurs gigaoctets.

La reconnaissance reste limitée aux structures, comme le C# ; le contrôle des entrées BAT appartient à la lecture du document. L'annulation laisse la destination préexistante intacte.

Résultat : `PASS VHDX CRC32C/GUIDs/redundancy/metadata/512-4096 sectors/fixed-dynamic/BAT states and spacers/atomic cancellation` (`build/vhdx-smoke.log`). Aucun média utilisateur ni ancienne suite de codecs utilisé. Programme, données synthétiques et binaires temporaires supprimés après consignation.

Les 25 erreurs sont traduites avec Argos dans les 29 langues, relues et corrigées, notamment images différentielles, métadonnées, journaux, négations, offsets et bitmaps. Les 30 DLL Constants/langues sont compilées (`build/vhdx-catalogs.log`). Audit réussi sur 1107 sources C++ et 29 langues (`build/vhdx-audit.log`). La composition globale des services et leur intégration à App restent à terminer.
