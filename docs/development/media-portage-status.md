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

La t�che 140 termine le lecteur KryoFlux multipiste : opcodes, index hors bande, horloge, m�tadonn�es et reconnaissance des captures NN.S.raw. Compilation et contr�le cibl� r�ussis ; aucun writer KryoFlux n�existe dans la r�f�rence C#.

La t�che 141 porte la lecture/�criture VDI 1.1 fixe et dynamique, avec table des blocs, UUID et g�om�trie. La sortie atomique commune accepte d�sormais le positionnement pour compl�ter les en-t�tes sans seconde lecture des donn�es. Les rejets de lecture VDI/KryoFlux sont raccord�s au registre ; compilation et contr�le cibl� r�ussis.

La t�che 142 porte la lecture/�criture VHD fixe et dynamique, avec footer, checksums, BAT et bitmaps. Le choix de variante est partag� avec VDI ; compilation et contr�le cibl� r�ussis.

La t�che 143 porte la lecture/�criture et cr�ation Acorn Winchester DAT/DSC, leur g�om�trie et les param�tres Adaptec initiaux n�cessaires au descripteur. Les captures partielles conservent des plages indisponibles et des messages traduisibles ; compilation et contr�le cibl� r�ussis. Le d�codage des pistes physiques Adaptec reste � porter.

## Travail restant

La composition complète des lecteurs et writers, les services globaux de lecture/conversion/ouverture et leur raccordement à l'application restent à terminer. Plusieurs familles de conteneurs disque dur, optiques et bandes restent à porter. Les lecteurs/writers bruts génériques et le catalogue principal sont portés : 98 formats réservés et 90 définitions brutes après exclusion complète. Les fonctions existantes ne doivent pas être prises pour une composition par défaut complète.

L'inventaire détaillé est dans ../references/retro-media-lab/media/inventaire-csharp.csv ; il distingue les sources portées, partielles et non portées. Les tâches sont dans ../tasks/media/, les validations dans ../validation/media/. Les validations ciblées et audits statiques ne prouvent pas le fonctionnement global depuis App.

Aucun média utilisateur, binaire de compilation ou cache Python n'est inclus dans ces commits. Les sources C# restent en lecture seule.
