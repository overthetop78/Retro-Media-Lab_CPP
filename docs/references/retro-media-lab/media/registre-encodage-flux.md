# Registre d’encodage des pistes — 15 novembre 2026

Sources C# en lecture seule : Images/Writing/Encoding/FluxEncoderRegistry.cs et FluxEncoderRegistryExceptions.cs. Le registre accepte une collection vide, copie son ordre, puis contrôle chaque entrée dans cet ordre : objet nul, identifiant vide/blanc, doublon ordinal. Une erreur antérieure garde la priorité sur les suivantes. Les recherches sont exactes, sans conversion de casse ni normalisation Unicode.

Encoding/Registry.h et Registry.cpp isolent cette sélection des algorithmes de codecs. Le constructeur par défaut utilise Composition::defaultEncoders (24 encodeurs). Le registre possède des références partagées constantes ; get retourne une référence propriétaire qui reste valide après destruction du registre. Les clés sont copiées à la construction : si un encodeur personnalisé modifie ultérieurement son identifiant, sa clé initiale reste enregistrée, comme en C#.

encode recherche l’encodeur avant de lui transmettre la requête et l’annulation. Il n’ajoute pas de cache ni de validation sectorielle ; celles-ci restent à la charge de TrackEncoder et du codec. Les quatre diagnostics sont résolus dans Constants et les 29 catalogues de langues.

La comparaison Unicode révèle un défaut du port : QString::fromUtf8 supprime U+FEFF initial. Ce caractère ne constitue pas un identifiant blanc pour String.IsNullOrWhiteSpace en C#. Primitives/Identifiers.h utilise QStringDecoder avec ConvertInitialBom pour conserver ce contenu pendant la validation. Les registres d’encodage et de décodage partagent cette fonction. La déclaration du drapeau et son contrat de décodage ont été consultés dans les en-têtes Qt locaux qstringconverter_base.h et qstringconverter.h du kit 6.12.0.

Ce bloc ne porte pas encore SectorImageTrackEncoder ni les politiques de reconstruction des images sectorielles.
