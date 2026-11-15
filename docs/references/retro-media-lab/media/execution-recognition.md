# ExÃ©cution des lecteurs prÃ©sÃ©lectionnÃ©s

RÃ©fÃ©rences C# en lecture seule : `MediaRecognitionRegistry.RecognizeAsync`, `MediaRecognitionResult` et `Services/MediaDriveOperationScope`.

`ReaderRegistry::recognizeAsync` utilise la prÃ©sÃ©lection existante et retient le premier document valide. Les Ã©checs prÃ©cÃ©dents conservent chacun leur reader possÃ©dÃ© et une `exception_ptr`. Un reader enregistrÃ© plusieurs fois remplace son Ã©chec prÃ©cÃ©dent, comme le dictionnaire C#. Une absence de candidat et un rejet de tous les candidats produisent des diagnostics distincts ; le second conserve les exceptions individuelles. Un reader retournant un document nul provoque une erreur de contrat, sans repli silencieux.

La politique `ReadRejection` centralise les diagnostics natifs de lecture invalides/non pris en charge des conteneurs raccordÃ©s. Elle respecte aussi les catÃ©gories `InvalidData` et `Unsupported` des erreurs MFS. Elle ne prend pas tous les diagnostics Engine pour des donnÃ©es invalides : les erreurs inconnues, E/S, annulations, arguments et limites numÃ©riques restent propagÃ©es. Les futurs readers devront ajouter leurs diagnostics de rejet vÃ©rifiÃ©s ici si nÃ©cessaire ; cette table ne prÃ©tend pas classifier toutes les exceptions du portage encore inachevÃ©.

Un reader annonÃ§ant un seul type de mÃ©dia permet au contexte dâ€™assurer son lecteur mÃ©canique. Les scopes ne concernent que le lecteur crÃ©Ã© par ce contexte, comme en C# ; un lecteur fourni par lâ€™appelant reste sous son contrÃ´le. Lâ€™ordre est dÃ©but dâ€™opÃ©ration, dÃ©but dâ€™accÃ¨s, lecture, fin dâ€™accÃ¨s, fin dâ€™opÃ©ration. Les sorties normales propagent les erreurs de nettoyage ; les destructeurs assurent le nettoyage lors dâ€™une exception sans masquer lâ€™erreur initiale. Un dÃ©but dâ€™opÃ©ration Ã©chouÃ© tente Ã©galement sa terminaison, conformÃ©ment Ã  la rÃ¨gle de nettoyage de lâ€™initialisation partielle.

Les trois nouveaux messages sont traduits dans les catalogues. Le rÃ©sultat et lâ€™exception agrÃ©gÃ©e exposent des donnÃ©es structurÃ©es ; aucun assemblage de texte anglais nâ€™est utilisÃ© comme seule information dâ€™erreur.

La composition globale des readers, le service public de lecture et le helper gÃ©nÃ©rique C# de sonde restent Ã  intÃ©grer. Lâ€™inventaire du registre reste explicitement partiel pour ce helper.
