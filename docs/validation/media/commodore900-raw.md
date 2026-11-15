# Validation Commodore 900 brut — 15 novembre 2026

Les cinq fichiers C# originaux ont été compilés sans modification avec des adaptateurs de contrats/modèles ; leurs cinq empreintes correspondent à l’inventaire. Le banc réutilise le décodeur canonique et les contrôles de superbloc C# originaux pour la validation des dumps. Les octets sont synthétiques et temporaires.

## Couverture

4 822 cas de référence : 2 395 conversions bloc → adresse (dont tous les 2 392 blocs valides et les bornes invalides), 2 296 conversions adresse → bloc, 82 cylindres et 49 dumps. Les dumps couvrent les frontières de toutes les zones, tailles tronquées, alignement, superbloc malformé, nombres déclarés invalides/non représentables et demande d’identifiant différente ou de casse différente.

Les 21 dumps effectivement lus sont écrits par les writers C# et C++, puis comparés par SHA-256 de tous leurs octets. Leurs métadonnées de géométrie, capacité/nombre réel, adresses de chaque secteur et intégrité sont comparées. Les autres dumps vérifient les refus et la distinction entre reconnaissance et lecture.

Les contrôles supplémentaires vérifient :

- refus de secteurs manquants, courts ou d’adresse physique incorrecte ;
- rejet des cinq paramètres de géométrie incompatibles ;
- préservation d’une destination existante après erreur et annulation, y compris après écriture de plusieurs secteurs, sans résidu temporaire ;
- parcours image brute → COHERENT → analyse d’un véritable fichier synthétique ;
- lecture initiale limitée à l’en-tête, puis lecture d’un seul secteur demandé ;
- comportement inchangé de l’adressage régulier par défaut ;
- possession de la fonction d’adressage et libération de son état capturé avec l’image ;
- neuf clés × 29 langues, soit 261 textes, chargés depuis les DLL installées et comparés aux catalogues relus.

## Résultats

Compilation complète et déploiement Debug/Release réussis avec MSVC x64 et Qt 6.12. Dans chaque configuration : **29 505 contrôles, aucune différence, 21 écritures brutes et 261 textes traduits**. Audit des littéraux : 289 fichiers C++, 29 langues. `git diff --check` passe.

SHA-256 du manifeste des résultats C# : `75e6dbb4e04ee4959bee30b428c8518b993b8821cdfefd1f5d887e3468c5d78a`.

Le banc `build/verification-commodore900/`, ses images, références, projets, exécutables et journaux de comparaison sont supprimés après validation. Aucun média utilisateur, test matériel ou ajout aux workflows GitHub. Les codecs GCR et conversions de fichiers ne sont pas validés par ce bloc.
