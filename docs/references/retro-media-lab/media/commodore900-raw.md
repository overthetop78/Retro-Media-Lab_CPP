# Commodore 900 — conteneur brut COHERENT zoné

## Référence et découpage

Les cinq sources C# `Commodore900Geometry`, `CoherentRawImageReader`, `CoherentRawImageWriter`, `CoherentRawImageExceptions` et `Commodore900SectorImageBuilder` ont été consultées, compilées sans modification dans le banc et vérifiées par SHA-256 contre l’inventaire. Les modèles/contrats du banc sont des adaptateurs minimaux. Le dépôt C# reste inchangé.

L’implémentation privée `MediaEngine/Formats/Commodore900/` sépare les constantes, conversions de géométrie, reconnaissance/lecture et écriture. `CoherentImages.h` est un contrat interne au moteur, pas la façade finale pour App. Le système de fichiers reste dans MediaFileSystems et la classification est appelée par cette bibliothèque ; aucun lien direct d’App à MediaFileSystems/MediaAnalysis n’est ajouté.

## Géométrie et données

Les 80 cylindres à deux faces ont des secteurs de 512 octets. Les zones sont : cylindres 0–38 à 16 secteurs, 39–52 à 15, 53–63 à 14 et 64–79 à 13. Cela représente **2 392 secteurs, 1 224 704 octets**. Le numéro physique du secteur commence à zéro ; l’ordre est cylindre, face, secteur. Les conversions vérifient toutes leurs bornes, notamment le nombre de secteurs propre à chaque cylindre.

Les images construites sur source possèdent désormais une fonction facultative de conversion bloc/adresse physique. En son absence, leur adressage régulier existant est conservé. Le lecteur Commodore 900 utilise cette fonction pour les zones ; le support et ses données restent possédés par l’image et chargés à la demande. Chaque secteur complet du dump est marqué d’intégrité vraie comme dans le C# ; cela décrit le dump décodé, pas une nouvelle vérification GCR.

La géométrie nominale indique toujours 16 secteurs au maximum par piste, mais le nombre logique et la capacité sont ceux des octets réellement fournis. Une image tronquée à un nombre entier de secteurs n’est pas complétée jusqu’à la capacité physique maximale.

## Reconnaissance et lecture

La reconnaissance vérifie la demande éventuelle d’identifiant, sans casse, puis longueur maximale et alignement sectoriel ; elle utilise les noms fixes du superbloc COHERENT. Comme dans le C#, elle ne valide pas à ce stade le nombre déclaré de blocs. Le lecteur effectif contrôle dans l’ordre : superbloc reconnu, alignement, nombre déclaré d’au moins trois blocs et inférieur ou égal au dump, puis capacité physique maximale. Les déclarations non représentables restent des erreurs codées du système de fichiers.

La lecture ne recharge que les 1 024 octets de l’en-tête pour ces contrôles. Le C# chargeait le dump complet avant de construire tous ses blocs ; le C++ réutilise le contrat de source partagé et conserve les mêmes octets/adresses. Une source fichier en lecture seule n’est pas un instantané immuable de tous ses octets.

L’exploration appelle COHERENT puis l’analyse des entrées. Le résolveur de famille C# ne classe pas l’identifiant `commodore900.coherent` dans le préfixe `commodore.` ; son repli Unknown est conservé, sans inventer une famille de classification.

## Écriture et diagnostics

Le writer exige identifiant et géométrie zonée attendus, et au plus 2 392 blocs. Il vérifie pour chaque numéro logique : bloc présent, adresse physique exacte et charge utile de 512 octets. Les octets sont transmis dans l’ordre logique au writer atomique partagé. Aucune adresse n’est corrigée silencieusement, aucun secteur n’est rempli implicitement.

Le C# matérialisait toute la sortie avant sa publication. Le C++ écrit progressivement dans le temporaire isolé. Une erreur ou annulation après quelques secteurs conserve la destination antérieure et détruit le temporaire. Cela peut changer la priorité entre une erreur d’ouverture du temporaire et un défaut d’un secteur ultérieur ; aucun fichier incomplet n’est publié.

Neuf nouveaux diagnostics sont disponibles dans les 29 langues ; les erreurs communes de secteur manquant/taille et d’écriture atomique réutilisent les clés existantes. Argos a fourni les traductions, ensuite relues et corrigées : cylindre, zonage, capacité maximale, divisibilité, ordre physique et protection du nom Commodore 900.

Les registres/documents/façades, conversion de fichiers vers COHERENT et codecs physiques GCR restent à porter. Le bloc ne réalise aucun accès matériel. [Validation](../../../validation/media/commodore900-raw.md).
