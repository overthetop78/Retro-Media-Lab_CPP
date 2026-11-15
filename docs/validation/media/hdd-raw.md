# Validation HDD brut

- Compilation Debug : `build/hdd-raw-final-build.log`, réussie sans avertissement signalé.
- Un contrôle natif sur petits fichiers synthétiques : `build/hdd-raw-smoke.log`, PASS. Choix explicite, refus automatique sans signature, marque MBR, priorité GPT 4096 sur 512, métadonnée de taille logique, représentation par blocs, détection de volumes MFS, réécriture identique et refus d’une longueur modifiée après création du contexte. Le writer refuse aussi une plage indisponible.
- Les données de test sont de petite taille et ne prouvent pas la prise en charge de toutes les tables GPT/MBR. Aucun disque physique ni média utilisateur utilisé. Aucun ancien test de codec relancé.
- Les fichiers synthétiques sont nettoyés par RAII même sur erreur, puis le banc et ses binaires sont supprimés après consignation.
- Cinq messages générés avec Argos dans les 29 langues, relus et corrigés pour les erreurs observées. Constants et les 29 DLL compilées : `build/hdd-raw-catalogs.log`.
- Après séparation de la fonction de compatibilité dans `Format.h`, compilation finale réussie. Audit des littéraux : 884 fichiers C++, 29 langues, PASS. Audit des frontières avant ce déplacement sans nouvelle dépendance : 871 fichiers, aucune violation ; projets Visual C++ régénérés. Ces contrôles ne constituent pas une validation globale du portage.
