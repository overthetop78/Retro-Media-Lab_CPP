# Représentation sectorielle et mécanique

Sources C# consultées en lecture seule. Le contrat SectorMediaImageRepresentation est porté dans une représentation qui possède l’image par shared_ptr et fournit simultanément la vue MFS et la vue Engine. Sa capacité est vérifiée avant exposition comme longueur signée.

Adaptation explicite de MediaDriveSectorData : le C# observe les accès à IReadOnlyList<byte> via indexeur/énumérateur ; le modèle C++ existant remet déjà un bloc contenant son vector d’octets. L’instrumentation se fait donc par requête de bloc, sans remplacer le conteneur d’octets ni recopier le bloc. Elle n’intercepte pas individuellement chaque indexation ultérieure d’un vector déjà remis au consommateur. Les données et toutes les métadonnées restent celles du bloc d’origine.

Le contrat MFS SectorImage expose désormais blockAddress, séparé de block. Tous les fournisseurs présents dans src l’implémentent sans lire de charge utile : images éparses, sources linéaires avec adresse explicite éventuelle, vues renommées, images Commodore zonées et vue ADFS empruntée. Cela permet de placer BeginRead/BeginWrite avant une E/S réelle, avec une adresse physique exacte. L’adresse enregistrée dans l’en-tête reste une métadonnée distincte de l’adresse physique.

Une adresse physique dont cylindre ou tête est négatif utilise le numéro logique, comme en C#. Les conversions vers DrivePosition sont contrôlées. L’énumération des blocs et la consultation des adresses ne déclenchent pas d’activité mécanique. Les sorties normales et par exception terminent l’accès via le garde RAII partagé. Les liaisons répétées à la même mécanique et au même mode conservent leur identité.

Le document commun sélectionne maintenant les trois liaisons prévues par le C# : blocs, optique et secteurs. Le service concret de simulation mécanique et le raccordement global des lecteurs/reconnaissances restent hors de cette étape.

| Source | SHA-256 vérifié |
| --- | --- |
| `src/RetroMediaLab.MediaEngine/Contracts/MediaImageDocument.cs` | `21c54aba4d16d6dae3e3cb887d14009c071603a2e0e54576931101ee88de3a33` |
| `src/RetroMediaLab.MediaEngine/Functions/MediaDriveBindingFunctions.cs` | `24c14b0aae4e33198f9aa94a7a483498110d04b1ae698086355a6975b79e4114` |
| `src/RetroMediaLab.MediaEngine/Images/Models/Sectors/SectorMediaImageRepresentation.cs` | `576d74f115e8028f22ffa81243b771ad4ead8a2d996981c67724754eb87f9ca2` |
| `src/RetroMediaLab.MediaEngine/Images/Reading/Sources/MediaDriveSectorData.cs` | `f34e24b09831c5f46d9fc4f1476684cd1afe243b274358c394291214db367d1a` |
