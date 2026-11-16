# Validation ciblée VMDK

Le 16 novembre 2026, compilation Debug MediaEngine réussie sous MSVC 19.44, sans avertissement observé (`build/vmdk-build.log`). La compilation finale remplace deux indices littéraux d'analyse du descripteur par leurs constantes nommées (`build/vmdk-final-build.log`).

Un programme natif temporaire a contrôlé :

- paire FLAT avec données logiques identiques, métadonnée Fixed et chemin associé ; nom Unicode contenant `%7` conservé littéralement dans le descripteur ;
- sparse avec trois grains logiques, celui du milieu nul et le dernier partiel : taille physique, overhead, répertoire et entrées des tables aux valeurs attendues, plages logiques et contenu relu ;
- réécriture sparse vers son fichier source et relecture ;
- étendue FLAT démarrant au secteur un et conservation du BOM UTF-8 dans la métadonnée du descripteur ;
- refus des sources trop courtes, versions/flags exclus, état non propre, géométrie, descripteur/répertoire hors fichier, table absente et grain hors fichier ;
- refus des parents, étendues multiples/manquantes, décalage négatif et source FLAT trop courte ;
- entrée de grain nulle conservée comme plage non allouée ; annulation conservant la destination préexistante.

Résultat : `PASS VMDK flat pair/sparse grains/descriptor paths and BOM/ranges/rejections/atomic cancellation` (`build/vmdk-smoke.log`). Aucun média utilisateur ni ancienne suite de codecs utilisé. Programme, médias synthétiques et binaires temporaires supprimés après consignation.

Les 25 erreurs sont générées avec Argos dans les 29 langues, relues et corrigées, notamment les grains, étendues, décalages, négations et état non propre. Les 30 DLL Constants/langues sont compilées (`build/vmdk-catalogs.log`). Audit réussi sur 1095 sources C++ et 29 langues (`build/vmdk-audit.log`). La composition globale des services et leur intégration à App ne sont pas déclarées achevées.
