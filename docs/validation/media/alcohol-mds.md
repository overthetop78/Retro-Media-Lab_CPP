# Validation Alcohol MDS/MDF

Le 16 novembre 2026 : MediaEngine compile en Debug MSVC x64 (`build/alcohol-mds-build.log`), sans avertissement observé. Les 30 DLL de catalogues compilent (`build/alcohol-mds-catalogs.log`). Quatre diagnostics traduisibles sont générés par Argos dans les 29 langues et relus/corrigés : négation italienne inversée, retours anglais, termes de bloc final/secteur et chinois traditionnel notamment. Audit : PASS 1161 fichiers C++ (`build/alcohol-mds-audit.log`).

Le contrôle natif temporaire réussit : `PASS MDS header/profile/structures, Latin1/UTF16 wildcard footer, five modes, shared MDF, interleaved SUB strides, lead-out, ranges, incomplete tail, cancellation` (`build/alcohol-mds-smoke.log`).

Les fixtures synthétiques couvrent cinq pistes sur un MDF partagé, avec AUDIO, MODE1/2048, MODE2/2336, MODE1/2352, MODE2/2352 ; tailles stockées avec/sans SUB et quatre vues de sous-canaux entrelacés. Vérification octet par octet des secteurs stockés, fenêtres utilisateur et SUB avec stride. Les limites à deux secteurs par piste et l’ignorance des trois octets de fin sont contrôlées, ainsi que les métadonnées et la déduplication du fichier associé.

Une variante à deux sessions contrôle leur affectation et leurs lead-out séparés. Une variante sans lead-out avec fin déclarée non utilisable contrôle le repli à max(startSector)+1 et la dernière piste réduite à un secteur. Les footers Latin-1 et UTF-16LE pointent vers le wildcard *.mdf ; la reconnaissance réussit aussi avec une extension primaire différente. Un extraOffset nul ignore le bloc concerné, comme dans le C#.

Les refus ciblés couvrent version majeure 2, bit DVD, mauvais format demandé, signature, en-tête tronqué, session vide ou sans numéro, firstTrack nul, blocs hors descripteur, piste sans numéro/taille/footer, offset UInt64 non représentable, noms hors fichier ou sans terminaison, MSF invalide, payload inconnu, MDF trop court/absent et annulation. Le premier essai de contrôle traitait à tort extraOffset nul comme une erreur ; seule cette attente du banc a été corrigée, sans changer le comportement de l’application.

Les projets, binaires et fixtures MDS/MDF temporaires sont supprimés après consignation. Les tests ne sont pas ajoutés au workflow GitHub. Aucun média utilisateur n’est intégré au dépôt ; le C# reste en lecture seule. Ces contrôles concernent ce reader et ne prouvent pas la composition globale de MediaEngine, MediaFileSystems, MediaAnalysis ou leur utilisation depuis App.
