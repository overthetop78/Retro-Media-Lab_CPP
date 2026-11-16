# Reconstruction SCP par définition et DEC RX02

## Structure

`DefinedReconstruction::Reader` reçoit le registre de décodeurs et le lecteur de capture. Il utilise le catalogue de définitions déjà embarqué et le collecteur ISO existant, sans catalogue parallèle. Les surcharges acceptent un chemin, une représentation de flux protégée ou une définition détenue par l'appelant pendant la lecture synchrone. Les registres capturés par l'opération de décodage restent possédés.

La construction `Definitions::createImage` est extraite du lecteur IMG pour être partagée avec SCP : taille maximale, capacité selon `imageSize`, nombre de blocs selon le layout, compte maximal par piste, tailles variables et conservation des positions indisponibles. Le lecteur IMG conserve sa validation de longueur et son ordre de découpage.

## Règles issues du C#

- Un passage par codec distinct de la définition, avec le même routage FM/MFM, Apple GCR, Data General, HP et Micropolis.
- Remplacement des instances natives Micropolis par leur variante conservant les 275 octets de l'enregistrement. Extraction des 256 octets de données seulement lorsque la définition le demande ; sinon enregistrement complet conservé.
- Sélection par adresse physique, cylindre enregistré, face attendue paramétrable par `h` et taille. Micropolis ignore la face enregistrée comme le C#.
- NorthStar utilise directement le décodeur matériel, révolution par révolution, avec mode FM/MFM et nombre de secteurs de la définition ; sélection du meilleur candidat à chaque emplacement. Cette branche ne fabrique pas de progression absente du C#.
- RX02 inverse la permutation physique/logique existante. Il suit les adresses décodées, sans ajouter une contrainte de concordance avec la piste de la capture que le C# n'impose pas. Deux secteurs physiques de 256 octets constituent un bloc de 512 octets. Il faut deux intégrités explicitement vraies ; la révolution retenue est le maximum des deux. Les blocs incomplets restent indisponibles.
- Les diagnostics génériques SCP distinguent aucun secteur décodé et aucune paire utilisable ; le dernier inclut les comptes réellement observés.

## Contrôles du bloc

Banc unique temporaire : définition mixte FM/MFM sur deux faces, face enregistrée forcée à zéro, tailles variables, positions absentes, progression, erreurs de format et de codec. Une piste Micropolis encodée avec le codec existant vérifie le remplacement du décodeur, puis la restitution 256/275 octets. Deux secteurs matériels NorthStar synthétiques vérifient leur chemin dédié. RX02 vérifie la permutation, la sélection entre révolutions, la concaténation, l'intégrité inconnue, les paires absentes, les deux erreurs et la progression. Annulation propagée.

Compilation Debug de MediaEngine sans avertissement, Constants et 29 langues ; audit des 1 451 fichiers C++ réussi. Catalogues Argos relus et corrigés, notamment négation japonaise et invariants altérés. SCP/ISO/FM/MFM/DOS/IBM ajoutés au fichier commun des invariants pour les générations suivantes. Paramètres vérifiés dans les 30 catalogues.

Le banc et ses fichiers temporaires sont supprimés après utilisation. Aucun test de codec répété en masse ni ajout au workflow.

## Référence et suivi

Sources C# `DefinedScpSectorImageReader.cs`, `DecRx02ScpSectorImageReader.cs`, `DefinedRawImageReader.cs` et constructeurs sectoriels consultées en lecture seule. Empreintes et correspondances actualisées dans l'inventaire. `UniformSectorImageLayout` possédait déjà son équivalent natif ; son état d'inventaire est corrigé après comparaison. Les autres constructeurs linéaires/réguliers et la reconstruction Apple restent suivis séparément.
