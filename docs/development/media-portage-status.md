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

La tâche 148 porte VHDX autonome Fixed/Dynamic, la primitive partagée CRC32C, les structures redondantes, GUID natifs, métadonnées, secteurs 512/4096 et états BAT. Les journaux actifs et parents nécessitant des résolveurs absents, ainsi que les blocs partiellement présents, restent refusés comme en C#. Compilation et contrôle ciblé réussis.

La tâche 149 termine les codecs communs CHD : MSB/Huffman, DEFLATE brut, LZMA, Zstandard et FLAC sans STREAMINFO, avec réutilisation des bibliothèques natives embarquées. Compilation, contrôle ciblé, audit et catalogues réussis ; conteneur et intégration CHD restent à faire.

La tâche 150 termine le conteneur commun CHD v1-v5 : en-têtes, cartes brutes/compressées/RLE, métadonnées, références, CRC, composition des codecs et réassemblage CD/ECC. Compilation, contrôle ciblé, audit et catalogues réussis ; lecteurs HDD/optiques et writers CHD restent à raccorder.

La tâche 151 porte le profil HDD CHD V5 autonome non compressé : géométrie GDDD, lecture par plages, carte zéro, écriture atomique, hunks complétés et SHA1 logique. Compilation, contrôle ciblé, audit et catalogues réussis ; le CHD optique reste à raccorder.

La tâche 152 porte le lecteur CHD optique : layouts bruts/décodés, sources bornées avec cache sérialisé, CHTR/CHT2/DVD, pistes CD/DVD, index/pregaps/sous-canaux, propriétés et rejets. Compilation, contrôle ciblé, audit et catalogues réussis. Les hunks sont invalidés en cache avant décodage pour sécuriser la reprise après exception.

La tâche 153 porte le bloc GDI complet : descripteur Unicode, pistes associées, sources partagées, métadonnées, lecture et écriture atomique multipiste. Les dispositions de sortie que le lecteur aurait refusées ou tronquées sont désormais refusées. Compilation, contrôle ciblé, audit et catalogues réussis.

La tâche 154 termine le bloc BIN/CUE : modèles CUE possédés, parseur/sérialiseur, sources PCM WAVE/Vorbis, reconnaissance, pistes/index/pregaps et publication atomique multipiste. Tremor entier et libogg officiels sont intégrés statiquement ; les PCM stéréo, mono et chaînés sont identiques au C# sur les échantillons contrôlés. Une erreur de callback libère le décodeur et une lecture suivante le rouvre. Compilation Debug, contrôle ciblé, audit et catalogues réussis. Le raccordement à la composition globale reste distinct.

La tâche 155 porte le lecteur CloneCD 2/3 : descripteur CCD possédé, TOC/sessions/pistes, entiers signés/hexadécimaux, IMG brut et SUB optionnel, index/pregaps et propriétés. Compilation Debug, contrôle ciblé, audit et catalogues réussis ; aucun writer absent du C# n’est ajouté.

La tâche 156 porte Alcohol MDS/MDF : structures binaires séparées du reader, sessions et footers possédés, noms Latin-1/UTF-16LE, sources MDF partagées et sous-canaux entrelacés. Le profil CD autonome du C# est conservé, avec refus des structures DVD non décodées. Compilation Debug, contrôle ciblé, audit et catalogues réussis.

La tâche 157 porte Wii U WUD/WUX et Xbox XDVDFS : source indexée WUX possédée, blocs dédupliqués SHA256 et writer atomique sans deuxième temporaire ; descripteur XDVDFS, piste et volume correctement relié à l’identifiant MFS existant. Readers/writers, contrôle ciblé, compilation Debug, audit et catalogues réussis. L’exploration Xbox reste dans MFS ; la composition globale reste distincte.

La tâche 158 est partielle : audio associé explicite, résumé public indépendant de MFS, cœur des sessions, boucles, secteurs/pregaps/sous-canaux, précache transactionnel et fermeture RAII portés. Compilation Debug, contrôle ciblé, audit et catalogues réussis. L’ouverture par chemin et la conversion optique attendent les services/compositions globaux ; aucun registre parallèle ne les remplace.

La tâche 159 porte le modèle séquentiel, les profils PCM entiers, extraction et encodage 8/16/24/32 bits, pulsations/silence et lecteur/writer RAW explicite atomique. Compilation Debug, contrôle ciblé, audit et catalogues réussis. MediaTapeSignal et le raccordement au service global restent distincts.

La tâche 160 porte le RIFF/WAVE séquentiel : profil PCM natif jusqu’à 32 canaux, chunks multiples/inconnus, sources partagées, métadonnées et writer atomique sans rééchantillonnage. Les remplissages impairs et trames incomplètes de sortie sont sécurisés. Compilation, contrôle ciblé, catalogues et audit réussis.

La tâche 161 porte le reader/writer Creative Voice legacy Caprice32 : blocs son/continuation/silence/marqueur/texte, PCM commun, conversion mono et sortie atomique avec continuations 24 bits. Compilation, contrôle ciblé, catalogues et audit réussis. Les autres profils de bandes et la composition globale restent à réaliser.

La tâche 162 porte SIMH : parseur de framing complet, records et marqueurs séquentiels, avertissements traduisibles, tri stable et writer atomique avec mots encadrants/padding. Les objets après EOM sont refusés pour éviter des records invisibles. Compilation, contrôle ciblé, catalogues et audit réussis.

La tâche 163 porte le contenu séquentiel dans MFS : blocs possédés, regroupement fidèle des fichiers nommés, intégrité à trois états, métadonnées et avertissements traduits. Le détecteur déjà présent est contrôlé et son inventaire corrigé sans duplication. Compilation, contrôle ciblé, catalogues et audit réussis ; registre/adaptateur moteur et protocoles restent à raccorder.

La tâche 164 porte les contrats/résultats de décodage séquentiel, le registre et l’adaptateur moteur vers MFS. Le classement stable, les diagnostics et la durée de vie des opérations sont contrôlés ; le passage par MediaExplorer et les vues analysées est vérifié avec des décodeurs fournis au registre. Compilation, catalogues et audit réussis. Les protocoles réels et la composition automatique restent à porter.

La tâche 165 porte le conteneur Atari CAS, les fonctions FSK communes et le décodeur Atari CAS/PCM : chunks conservés, temps, records/checksums et récupération UART/horloge. Contrôle ciblé réussi avec polarité inversée, étirement de 5 % et passage au vrai adaptateur MFS sans inventer de noms ; un compteur valide nul du portage est corrigé. Compilation, catalogues et audit réussis. L’encodeur Atari et la composition automatique restent à terminer.

Les blocs décodés séquentiels partagent désormais leurs octets possédés et immuables lors des copies de requêtes. Cela évite de recopier les données pendant la sélection des encodeurs. Compilation et contrôle ciblé réussis.

La tâche 166 porte les contrats et le registre d’encodage séquentiel, puis l’encodeur Atari CAS avec reconstruction explicite des records logiques et conservation validée des chunks. Compilation, contrôle ciblé, catalogues et audit réussis. Les services globaux de conversion restent à porter.

La tâche 167 porte CSW 1/2 RLE et Z-RLE, avec décompression bornée par blocs et écriture atomique CSW 2 depuis le premier canal PCM natif. Compilation, contrôle ciblé, traductions relues, catalogues et audit réussis.

La tâche 168 porte le conteneur UEF direct/gzip, la conservation ordonnée des chunks et leur contexte, puis le writer atomique sans perte des payloads. Compilation, contrôle ciblé, traductions relues, catalogues et audit réussis. La synthèse UEF et le protocole Acorn restent à porter.

La tâche 169 porte le conteneur et le codec Atom TAP : fichiers successifs, reconstruction série, contrôle des checksums/séquences et writer atomique refusant les pertes. Compilation et contrôle ciblé réussis, traductions relues et catalogues compilés. Les adaptateurs du protocole Atom et la synthèse PCM restent à porter.

La tâche 170 porte le signal de bande possédé, l’extraction du premier canal PCM et les transformations UEF/Atom ↔ signal (sans les services globaux). Les repères, trames, cycles partiels, polarités et durées sont contrôlés. Compilation, contrôle ciblé, traductions relues, catalogues et audit réussis.

La tâche 171 porte les adaptateurs de décodage/encodage séquentiels Atom. Le contrôle via les registres couvre TAP, UEF série, UEF cycles, PCM FSK et conservation des erreurs. Le partage des blocs et la validation sans recopie du TAP sont intégrés. Compilation, traductions, catalogues et audit réussis ; la composition automatique reste à porter.

## Travail restant

La composition complète des lecteurs et writers, les services globaux de lecture/conversion/ouverture et leur raccordement à l'application restent à terminer. Plusieurs familles de conteneurs disque dur, optiques et bandes restent à porter. Les lecteurs/writers bruts génériques et le catalogue principal sont portés : 98 formats réservés et 90 définitions brutes après exclusion complète. Les fonctions existantes ne doivent pas être prises pour une composition par défaut complète.

L'inventaire détaillé est dans ../references/retro-media-lab/media/inventaire-csharp.csv ; il distingue les sources portées, partielles et non portées. Les tâches sont dans ../tasks/media/, les validations dans ../validation/media/. Les validations ciblées et audits statiques ne prouvent pas le fonctionnement global depuis App.

Aucun média utilisateur, binaire de compilation ou cache Python n'est inclus dans ces commits. Les sources C# restent en lecture seule.
