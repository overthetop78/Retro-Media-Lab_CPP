# Readers communs ADF / DFS / IBM / MSX

Sources C# de référence en lecture seule : `Images/Formats/Floppy/Adf/AdfReader.cs`, `BbcDfs/BbcDfsReader.cs`, `Raw/IbmRawImageReader.cs`, `Raw/MsxRawImageReader.cs`, ainsi que `Images/Reading/Reconstruction/Sectors/LinearSectorImageBuilder.cs`.

Chaque reader annonce ses formats, extensions et représentation sectorielle et réutilise les backends natifs déjà portés. Les readers IBM/MSX acceptent les préfixes dynamiques correspondants. La détection IBM reprend BPB puis géométries connues ; MSX exige la sonde MSX et le profil capacité/descripteur. Les exceptions d’E/S ou d’annulation du contexte ne sont pas converties en simple échec de reconnaissance.

ADF reprend la sonde native, y compris les images Acorn partielles et les cas particuliers DOS Plus et `.640`. Une image plus courte que sa capacité logique produit le message structuré existant `MediaEngine.AdfInvalidLength`, sans explorer artificiellement les secteurs absents. La recette conserve les arguments et le domaine de traduction au lieu de figer un texte anglais dans le document.

DFS ne lit les en-têtes de catalogue pendant la sonde que pour les alias bruts et ADL ; SSD/DSD utilisent les bornes de longueur comme en C#. Les secteurs conservent leur base zéro et les informations de tête enregistrée.

Correction de fidélité découverte au raccordement : les readers natifs IBM/MSX utilisaient `SourceSectorMetadata` avec l’état d’intégrité implicite indéterminé. Le constructeur C# de secteurs complets les initialise à `true`. Les deux backends indiquent maintenant explicitement `trustCompletePayload=true`. Cette propriété représente l’état logique initial du conteneur brut ; elle ne certifie aucun CRC matériel.

La composition et le classement globaux des readers restent à porter. Les fonctions historiques d’exploration spécifiques ne sont pas remplacées par ce raccordement.
