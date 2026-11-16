# Modèle séquentiel, PCM et RAW audio

Références C# consultées en lecture seule dans F:/GW GUI/src/RetroMediaLab.MediaEngine : SequentialMediaSegment, SequentialMediaImageRepresentation, les deux enums séquentielles, TapePcmProfile, PcmSampleFunctions, TapePcmFunctions et RawTapeReader/Writer. Les SHA-256 des neuf sources correspondent à l’inventaire.

Le modèle possède ses segments et métadonnées, distingue collection absente/vide et dérive les coordonnées distinctes triées. Les temps utilisent les ticks de 100 ns de TimeSpan, avec troncature de FromSeconds vérifiée dans le runtime .NET local.

Les fonctions communes conservent le PCM entier 8/16/24/32 bits et tous les canaux. Le RAW normalise le signe et l’ordre des octets sans réduire la précision. Le rééchantillonnage interpole dans le domaine signé 32 bits et utilise l’arrondi au pair de Math.Round. Les canaux supplémentaires reprennent le dernier canal source. Un segment par canal partage la même source entrelacée possédée. Les pulsations et silences réutilisent ces conventions ; les calculs intermédiaires évitent les débordements int32 et les extractions valident explicitement l’alignement.

Le RAW reste un choix explicite tape.raw ; il ne revendique pas automatiquement chaque fichier .raw. Le writer reprend Source.Parameters lorsqu’ils existent, sinon les métadonnées, et publie atomiquement après encodage. Les sources et buffers sont possédés par RAII, y compris en cas d’annulation ou d’exception.

Le repli RAW du service global, la composition par défaut, MediaTapeSignal et les autres conteneurs de bandes ne sont pas déclarés achevés. Le constructeur de document de signal accepte actuellement les échantillons et la fréquence directement.
