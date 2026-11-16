# État du portage média au 16 novembre 2026

Le portage de MediaEngine, MediaFileSystems et MediaAnalysis est en cours. Les commits ci-dessous enregistrent le travail existant ; ils ne déclarent pas la migration globale achevée.

## Points de reprise Git

- d0455582 : protection des invariants et paramètres de traduction, audits des littéraux et des frontières entre bibliothèques.
- 07121601 : MediaAnalysis, reconnaissance/classification et catalogues traduits.
- aa75c0c0 : MediaFileSystems, modèles, accès, partitions et explorateurs avec leurs catalogues.
- ffef9cc5 : MediaEngine, modèles, sources, registres, codecs, reconstruction et formats déjà portés, avec leurs catalogues. Intégration globale encore en cours.
- Le commit contenant ce document raccorde ces bibliothèques à CMake et aux projets Visual C++, puis archive les tâches, références et validations du portage.

## Dernier travail terminé

Les tâches 124, 125 et 126 portent respectivement les writers Epson/ImageDisk, la conversion des pistes protégées vers SCP et les modèles/ressources/lecteur des définitions de disquettes. Les contrôles ciblés ont réussi ; leurs programmes temporaires sont supprimés. La tâche 127 exclut les caches Python générés du suivi Git. La tâche 130 ajoute le catalogue principal et les factories brutes par défaut. La tâche 131 porte les catalogues dynamiques et l'adaptation aux formats logiciels annoncés par Greaseweazle. La tâche 132 termine le lecteur SCP commun, son contrat injectable et son cache partagé avec nettoyage des travailleurs ; les révolutions du document partagent les buffers immuables de la capture. Les tâches 128 et 129 ajoutent la lecture/écriture brute générique et les adapters SCP/HFE au registre commun, avec contrôles ciblés réussis et programmes temporaires supprimés.

La tâche 133 porte la structure native JFD : tables principales/delta, secteurs absents, conservation des pistes brutes, édition sans perte et gzip avec vérification CRC ; compilation et contrôle ciblé réussis. La tâche 134 termine la création de nouvelles images et les adaptateurs lecteur/writer des registres JFD, les métadonnées natives strictes et la compression gzip atomique ; les contrôles ciblés passent.

La tâche 135 termine le lecteur CP2 : groupes et descripteurs, reconstruction IBM et registre commun, avec contrôle ciblé réussi.

La tâche 136 termine le lecteur 86F, ses modèles, sa conversion en pistes protégées et son adaptateur du registre ; compilation et contrôle ciblé réussis.

La tâche 137 porte le codage exact HxC Stream, les blocs LZ4/CRC et les empreintes/clés de captures natives. La tâche 138 termine le lecteur multipiste et son adaptateur du registre avec captures natives, empreintes et protection globale ; compilation et contrôle ciblé réussis. Le writer et la publication du jeu multipiste restent à porter.

La tâche 140 termine le lecteur KryoFlux multipiste : opcodes, index hors bande, horloge, métadonnées et reconnaissance des captures NN.S.raw. Compilation et contrôle ciblé réussis ; aucun writer KryoFlux n’existe dans la référence C#.

La tâche 141 porte la lecture/écriture VDI 1.1 fixe et dynamique, avec table des blocs, UUID et géométrie. La sortie atomique commune accepte désormais le positionnement pour compléter les en-têtes sans seconde lecture des données. Les rejets de lecture VDI/KryoFlux sont raccordés au registre ; compilation et contrôle ciblé réussis.

La tâche 142 porte la lecture/écriture VHD fixe et dynamique, avec footer, checksums, BAT et bitmaps. Le choix de variante est partagé avec VDI ; compilation et contrôle ciblé réussis.

La tâche 143 porte la lecture/écriture et création Acorn Winchester DAT/DSC, leur géométrie et les paramètres Adaptec initiaux nécessaires au descripteur. Les captures partielles conservent des plages indisponibles et des messages traduisibles ; compilation et contrôle ciblé réussis. La tâche 144 complète le codec physique Adaptec, sa géométrie, les fenêtres DATA et le lecteur logique avec cache de piste et adaptation aux sources de blocs. Compilation et contrôle ciblé réussis ; le conteneur MfmEmu reste à raccorder.

La tâche 145 termine le conteneur physique MFM EMU : en-têtes conservés, fenêtres de transitions, pistes immuables et génération formatée à la demande, représentation physique et vue logique Adaptec, lecteur/writer et création vierge ou ADFS. Compilation Debug, contrôle natif ciblé, audit et compilation des catalogues réussis. La composition globale des services reste distincte de ce bloc.

La tâche 146 porte QCOW2 autonome en versions 2 et 3, avec tables L1/L2, validation des compteurs de références, plages stockées/zéro/non allouées et écriture sparse atomique conservant la version source. Les profils parents, snapshots, chiffrement et compression restent refusés comme dans le C#. Compilation et contrôle ciblé réussis.

La tâche 147 porte VMDK monolithicFlat et monolithicSparse autonomes : analyse des descripteurs, source FLAT avec décalage, grain directory/tables, plages stockées/zéro/non allouées, choix de variante et publication atomique. Compilation et contrôle ciblé réussis.

## Travail restant

La composition complète des lecteurs et writers, les services globaux de lecture/conversion/ouverture et leur raccordement à l'application restent à terminer. Plusieurs familles de conteneurs disque dur, optiques et bandes restent à porter. Les lecteurs/writers bruts génériques et le catalogue principal sont portés : 98 formats réservés et 90 définitions brutes après exclusion complète. Les fonctions existantes ne doivent pas être prises pour une composition par défaut complète.

L'inventaire détaillé est dans ../references/retro-media-lab/media/inventaire-csharp.csv ; il distingue les sources portées, partielles et non portées. Les tâches sont dans ../tasks/media/, les validations dans ../validation/media/. Les validations ciblées et audits statiques ne prouvent pas le fonctionnement global depuis App.

Aucun média utilisateur, binaire de compilation ou cache Python n'est inclus dans ces commits. Les sources C# restent en lecture seule.
