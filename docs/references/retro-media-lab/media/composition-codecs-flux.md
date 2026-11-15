# Composition des codecs de flux — 15 novembre 2026

Référence C# locale en lecture seule : `F:/GW GUI/src/RetroMediaLab.MediaEngine`. FluxDecoderCatalog.CreateDefault déclare 25 décodeurs ; FluxEncoderCatalog.CreateDefault déclare 24 encodeurs, dans le même ordre à l’exception du flux brut, réservé au décodage. Les identifiants et noms proviennent de Definitions/FluxCodecIds.cs et FluxCodecDisplayNames.cs.

La composition est isolée dans Composition/FluxCodecs.h et FluxCodecs.cpp. Chaque appel construit de nouvelles instances, sans état global. Le constructeur sans argument de Decoding/Registry utilise cette composition ; le constructeur explicite, le cache faible, la synchronisation et les règles de score restent ceux du bloc 33.

Ordre : ISO MFM, ISO FM, Amiga, Apple II GCR, RWTS18, Macintosh, Lisa FileWare, Commodore GCR, HP MMFM, Data General, Micropolis, Membrain, AED 6200P, QD MO5, Centurion, NorthStar, Heathkit, Micral N, E-mu, TYCOM, RX02, Arburg, Victor 9000, Commodore 900, puis flux brut pour le décodage seul.

Le registre C# exclut explicitement AppleLisaFileWareGcrDecoder de sa collection automatique, y compris avec une collection de décodeurs fournie par l’appelant. Le port partage la classe AppleIwm::Decoder entre Macintosh et Lisa ; son contrat participatesInAutomaticDetection n’avait pas encore spécialisé le cas Lisa. Le raccordement révèle cette règle manquante. Elle est corrigée dans AppleIwm/Gcr.h : la variante Lisa retourne faux, Macintosh conserve vrai. Le décodage explicite Lisa reste disponible.

La composition référence les contrats des codecs ; les algorithmes de reconnaissance et d’encodage restent dans leurs composants. Aucun couplage vers App, Infrastructure ou MediaAnalysis n’est ajouté. FluxEncoderRegistry et les politiques de reconstruction restent des blocs distincts.

Précision sur Lisa : AppleLisaFileWareGcrDecoder et AppleMacGcrDecoder ne spécialisent que Id et DisplayName ; ils utilisent actuellement le même algorithme AppleIwmGcrDecoder. L’exclusion évite donc un candidat automatique équivalent qui ne permet pas de distinguer Lisa de Macintosh. Cette conséquence est vérifiée dans le code ; la motivation historique de l’exclusion n’y est pas documentée. Cela ne prouve pas l’équivalence de tous les formats physiques Lisa/Macintosh.
