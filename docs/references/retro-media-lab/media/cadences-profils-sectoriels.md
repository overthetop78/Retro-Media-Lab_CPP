# Cadences et profils sectoriels — 15 novembre 2026

Sources C# de référence : SectorImageTrackTimingCatalog, TrackEncodingTimings, TrackEncodingDefaults, RareTrackEncodingProfiles, RareDiskImageFormatIds et leurs constantes de formats/codecs. Elles sont consultées et compilées en lecture seule. Les cadences sont exprimées dans les ticks communs du moteur, pas en nanosecondes.

Encoding/Timings.h et Timings.cpp conservent l’ordre des règles du C# : exceptions Amiga HD/IBM 2880, famille Amiga, Atari simple densité et Acorn DFS, formats haute densité, familles double densité, puis valeurs par défaut. Les durées nominales de révolution distinguent IBM 1200 à 360 RPM des autres formats à 300 RPM.

Encoding/RareProfiles.h et RareProfiles.cpp regroupent les quatorze correspondances format/codec/cadence. Les identifiants de codecs viennent des composants déjà portés, les identifiants de formats de Formats/FormatIds.h ; une valeur de profil retournée ne dépend d’aucun objet temporaire. L’absence de correspondance retourne optional vide. Les familles Apple/Commodore zonées seront raccordées avec leurs politiques ; elles ne passent pas implicitement par ces profils rares.

Les identifiants communs déjà définis dans le contrat de MediaFileSystems sont réutilisés ; les identifiants/préfixes propres aux catalogues du moteur sont internes à MediaEngine. Aucun service de matériel ou d’interface n’est déplacé dans ces fichiers.

Les clés de ce catalogue sont ASCII. Les comparaisons de Primitives/Identifiers.h ignorent uniquement la casse ASCII et conservent les autres octets. Cela reproduit le chemin ASCII de StringComparison.OrdinalIgnoreCase pour ces clés, sans assimiler ſ à s ni K à k, contrairement à la comparaison Qt insensible à la casse. Les fonctions portent explicitement le préfixe ascii ; elles ne prétendent pas remplacer une comparaison ordinale Unicode générale. La validation des identifiants de registres conserve séparément son décodage UTF-8 avec U+FEFF initial préservé.
