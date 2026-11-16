# État du portage média au 16 novembre 2026

Le portage de MediaEngine, MediaFileSystems et MediaAnalysis est en cours. Les commits ci-dessous enregistrent le travail existant ; ils ne déclarent pas la migration globale achevée.

## Points de reprise Git

- d0455582 : protection des invariants et paramètres de traduction, audits des littéraux et des frontières entre bibliothèques.
- 07121601 : MediaAnalysis, reconnaissance/classification et catalogues traduits.
- aa75c0c0 : MediaFileSystems, modèles, accès, partitions et explorateurs avec leurs catalogues.
- ffef9cc5 : MediaEngine, modèles, sources, registres, codecs, reconstruction et formats déjà portés, avec leurs catalogues. Intégration globale encore en cours.
- Le commit contenant ce document raccorde ces bibliothèques à CMake et aux projets Visual C++, puis archive les tâches, références et validations du portage.

## Dernier travail terminé

Les tâches 124, 125 et 126 portent respectivement les writers Epson/ImageDisk, la conversion des pistes protégées vers SCP et les modèles/ressources/lecteur des définitions de disquettes. Les contrôles ciblés ont réussi ; leurs programmes temporaires sont supprimés. La tâche 127 exclut les caches Python générés du suivi Git. La tâche 130 ajoute le catalogue principal et les factories brutes par défaut. La tâche 131 porte les catalogues dynamiques et l'adaptation aux formats logiciels annoncés par Greaseweazle. Les tâches 128 et 129 ajoutent la lecture/écriture brute générique et les adapters SCP/HFE au registre commun, avec contrôles ciblés réussis et programmes temporaires supprimés.

## Travail restant

La composition complète des lecteurs et writers, les services globaux de lecture/conversion/ouverture et leur raccordement à l'application restent à terminer. Plusieurs familles de conteneurs disque dur, optiques et bandes restent à porter. Les lecteurs/writers bruts génériques et le catalogue principal sont portés : 98 formats réservés et 90 définitions brutes après exclusion complète. Les fonctions existantes ne doivent pas être prises pour une composition par défaut complète.

L'inventaire détaillé est dans ../references/retro-media-lab/media/inventaire-csharp.csv ; il distingue les sources portées, partielles et non portées. Les tâches sont dans ../tasks/media/, les validations dans ../validation/media/. Les validations ciblées et audits statiques ne prouvent pas le fonctionnement global depuis App.

Aucun média utilisateur, binaire de compilation ou cache Python n'est inclus dans ces commits. Les sources C# restent en lecture seule.
