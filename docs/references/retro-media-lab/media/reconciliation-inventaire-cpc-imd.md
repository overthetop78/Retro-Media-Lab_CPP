# Réconciliation du suivi CPC DSK / ImageDisk

23 entrées marquées « non porté » possédaient déjà leur équivalent natif. Le suivi avait été mis à jour pour les lecteurs et adaptateurs, sans y rattacher les types, constantes, builders et writers associés. Ces composants ne doivent pas être portés une seconde fois.

Correspondances confirmées dans les sources courantes :

- Les records C# CPC image/piste/secteur et le type de conteneur correspondent à `Formats/CpcDsk/CpcDsk.h`.
- Les signatures, positions des champs, masques et limites correspondent à `CpcDsk/Constants.h`.
- Le builder et le writer correspondent à `CpcDsk/Builder.cpp`, `Writer.cpp` et `Internal.h`. Ils conservent les adresses enregistrées, statuts, taille nominale, ordre et données natives. Les exceptions sont remplacées par les diagnostics `CpcDsk*` traduits.
- Les records ImageDisk, modes, sections et types d’enregistrement correspondent à `Formats/ImageDisk/ImageDisk.h` ; les prédicats de données/compression/intégrité/suppression et la sélection du type d’enregistrement correspondent à `Models.cpp`.
- Les masques, positions, tailles et identifiants de métadonnées correspondent à `ImageDisk/Constants.h`.
- La construction de l’image sectorielle correspond à `ImageDisk/Models.cpp`, la sérialisation à `Writer.cpp` et la conservation/restauration structurelle à `Metadata.cpp`.

Références de validation existantes conservées : `readers-imd-cpc-hfe.md`, `adf-dfs-cpc-writers.md`, `epson-imagedisk-writers.md`. Aucun backend n’a été réécrit, aucun test de codec ni compilation supplémentaire n’a été lancé pour cette correction documentaire. Cette réconciliation corrige le suivi ; elle ne constitue pas une nouvelle validation exhaustive de ces formats.
