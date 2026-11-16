# Lecteur HxC Stream

Référence : `F:/GW GUI/src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/HxcStream/HxcStreamReader.cs`, consultée en lecture seule ; empreinte actuelle inchangée par rapport à `inventaire-csharp.csv`.

## Décodage

`Reading.cpp` possède le tampon natif transmis et valide chaque chunk CHKH : taille minimale, limite de 64 Mio, plage disponible et CRC. Il traite les métadonnées, blocs E/S et blocs d'impulsions ; les types inconnus sont refusés. Les tailles compressées/décompressées restent bornées ; le résultat LZ4 doit avoir exactement la taille déclarée. Les blocs E/S doivent contenir un nombre pair d'octets et sont décodés en mots little-endian. Le nombre d'impulsions déclaré est borné à 500 millions par bloc, sans réserver ce nombre avant validation.

`Pulses.cpp` décode les quatre longueurs du code natif. Une impulsion nulle est comptée mais n'ajoute pas de transition, conformément au C#. Les octets après le nombre d'impulsions déclaré sont ignorés comme dans la source. Les tailles externes des blocs compressés ne remplacent pas les champs internes de taille utilisés par le lecteur C#.

`Metadata.cpp` décode le texte UTF-8, retire les caractères nuls finaux et découpe les lignes CR/LF nettoyées. La première valeur d'une clé est conservée. Chaque champ sample_rate_hz valide à 25 ou 50 MHz peut néanmoins modifier la résolution effective, même s'il répète la clé : c'est le comportement explicite du C#.

`Revolutions.cpp` repère les fronts montants de l'index dans les mots d'E/S. Les impulsions sont cumulées en temps absolu puis réparties entre index, en incluant les impulsions de fin de période et en excluant celles précédant ou coïncidant avec le début. Sans deux index, une seule révolution est formée depuis la somme des intervalles si ceux-ci existent. Les durées et conversions de ticks sont contrôlées.

## Jeu de fichiers

`Files.cpp` sélectionne les fichiers du même préfixe dont le nom se termine par NN.S.hxcstream, avec deux chiffres et une face 0 ou 1. Les cylindres supérieurs à 83 sont exclus. Les pistes sont classées par cylindre et tête. Un nom incorrect ou un jeu vide produit une erreur traduite ; un dossier source absent ou inaccessible produit l'erreur d'accès existante.

L'adaptateur du registre annonce raw.hxcstream, l'extension et extension associée .hxcstream, la signature CHKH et la représentation flux. Il ouvre les fichiers physiques, conserve chaque capture native en base64 et range les champs de métadonnées sous leur préfixe physique. La protection globale est le OU des captures ; les empreintes de toutes les pistes utilisent ensuite cette même valeur globale.

Le document reçoit les fichiers associés, leur longueur totale et le format explicitement demandé, comme dans le C#. Les sources de fichiers, tampons et révolutions ont une durée de vie possédée ; les erreurs et annulations libèrent les ressources par RAII.

Ce bloc termine le lecteur et les constantes du conteneur. Le writer HxC Stream reste à porter avec la publication récupérable du jeu de fichiers : le lecteur ne constitue pas la fin du portage média global.
