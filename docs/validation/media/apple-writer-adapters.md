# Validation des writers Apple

- Compilation Debug réussie dans `build/apple-writer-adapters-build.log`, puis `build/apple-writer-adapters-final-build.log` après raccordement du callback d’écriture injecté ; aucun avertissement signalé.
- Contrôle ciblé natif : `build/apple-writer-adapters-smoke.log`, PASS. Vérifie couples profil/extension, alias ProDOS, refus de changement de système de fichiers, filtres des quatre adapters, rejet d’extension inconnue de la façade, layouts Macintosh/Lisa.
- Écriture réelle DiskCopy par l’adapter Macintosh puis relecture : nom binaire contenant un octet non ASCII et un octet nul, format de disque, octet de format et données conservés. Les secteurs synthétiques possèdent les tags nécessaires à une lecture détaillée sans système de fichiers reconnaissable.
- La première version du banc utilisait un identifiant RWTS18 dans le mauvais espace de noms et des données synthétiques sans tags ni structure reconnue ; ces erreurs du banc ont été corrigées. Aucun défaut du backend DiskCopy n’est déduit de ces échecs.
- Sortie NIB effective par l’adapter : taille attendue de 35 pistes × 6656 octets. Sortie WOZ effective par la façade : signature WOZ1. Callback injecté : octets NIB fournis sans création de fichier par la façade. Aucun relancement des suites des codecs.
- Les sorties du banc sont protégées par RAII ; programme, binaires et dossier de contrôle supprimés après consignation. Aucun média utilisateur utilisé.
- Deux messages générés avec Argos dans 29 langues puis relus et corrigés. Le catalogue préexistant touché par la collision de nom a été restauré depuis ses données générées antérieures, avant compilation des catalogues.
- Compilation des 30 catalogues : `build/apple-writer-adapters-catalogs.log`, succès. Audit des littéraux : PASS, 946 fichiers C++, 29 langues. Projets Visual C++ régénérés.

Ces résultats couvrent les adapters et la façade. L’intégration globale dans le service de conversion et dans l’application reste à terminer.
