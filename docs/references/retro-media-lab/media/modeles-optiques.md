# Modèles optiques communs

Référence C# consultée en lecture seule. OpticalTrack et OpticalImage fournissent les contrats MFS et la vue de représentation Engine, sans logique App.

Les validations conservent les dimensions signées vérifiées, la fenêtre utile, les prégaps stockés et les sous-canaux avec stride distinct de leur longueur utile. L’overload stocké à destination fournie admet le prégap ; l’overload qui alloue son résultat le refuse, conformément au C#. La lecture de sous-canaux absents retourne une valeur absente après validation du secteur, sans inventer de données.

Les index sont ordonnés de façon stable ; leurs positions relatives peuvent être négatives. Les sessions sont triées et dédupliquées ; les listes de pistes absentes et vides restent distinctes. Les métadonnées des pistes utilisent une comparaison insensible à la casse. Les callbacks mécaniques capturent seulement les coordonnées nécessaires et partagent la durée de vie des sources, sans cycle de possession.

La source de compatibilité conserve la longueur annoncée mais refuse les lectures avec une erreur Unsupported. Les lectures différées conservent l’état du descripteur et ses sources. Les options remises aux consommateurs sont en lecture seule après construction.

La liaison optique conserve pistes, index, flags, catalogNumber, ISRC, prégaps, sous-canaux, métadonnées, couches, faces et fichiers associés. Elle reprend les positions logiques et le découpage par secteur/stride C#. La liaison des données de secteurs reste à porter séparément.

| Source | SHA-256 vérifié |
| --- | --- |
| `src/RetroMediaLab.MediaEngine/Contracts/Optical/OpticalTrackDescriptor.cs` | `0daf66c583d8c7a0d551bbfe3bde718648bc7cefa06f88d2c57769431187c015` |
| `src/RetroMediaLab.MediaEngine/Contracts/Optical/OpticalTrackIndex.cs` | `51d5178489a04430b7b4b3b84eaec383b9bb30a603fa7769d25d2fa3a881efa1` |
| `src/RetroMediaLab.MediaEngine/Enums/OpticalTrackMode.cs` | `b565450f8900704690ac1937f83cc9e7df4a457ad7b0d7f2d88bb991b9e24bbc` |
| `src/RetroMediaLab.MediaEngine/Images/Models/Optical/OpticalMediaImageRepresentation.cs` | `8b2747937f020a3a575fb36f7a1084058a00237ee87316e3955b10dab01ae675` |
| `src/RetroMediaLab.MediaEngine/Images/Reading/Optical/OpticalSectorReader.cs` | `701111e346a940058d26ef80539e48be94efe7c2b8c37472ce56ae9fc3093c74` |
| `src/RetroMediaLab.MediaEngine/Images/Reading/Sources/UnavailableRandomAccessData.cs` | `6692769a9b9ce18b1a7806b45d89da34f340818721562e940b659bc9758c9192` |
