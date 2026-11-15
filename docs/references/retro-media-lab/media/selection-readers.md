# PrÃ©sÃ©lection des lecteurs de mÃ©dia

RÃ©fÃ©rences C# consultÃ©es en lecture seule : `Images/Reading/Recognition/MediaRecognitionRegistry.cs` et `MediaRecognitionCandidate.cs`.

Le C++ sÃ©pare le classement des candidats de leur future exÃ©cution. `Images/ReaderRegistry` possÃ¨de les readers ; sa future copie ces propriÃ©taires et le contexte, sans retenir le pointeur du registre.

Les indices sont calculÃ©s dans lâ€™ordre du C# : format explicite (1), extension (0,5), fichier associÃ© (0,55), signature (0,9), sonde du reader (0,8). La confiance est le maximum, pas une somme. Un format explicitement demandÃ© exclut les readers incompatibles avant leurs sondes. Une extension seule peut conserver un candidat mÃªme si sa sonde ne confirme pas le format ; la lecture complÃ¨te devra le valider ensuite.

Le tri utilise confiance dÃ©croissante, nombre dâ€™indices dÃ©croissant puis ordre dâ€™enregistrement conservÃ© par tri stable. La reconnaissance de signatures attend la taille maximale annoncÃ©e par le reader, comme en C#, mÃªme lorsquâ€™une signature plus courte pourrait suffire. Les erreurs des sondes et lectures restent propagÃ©es. Les longueurs de signature hors limite de lecture en mÃ©moire sont rejetÃ©es avant conversion numÃ©rique.

Les raisons anglaises brutes du C# sont remplacÃ©es par `RecognitionEvidence`. Le candidat nâ€™est construit que par le registre : reader possÃ©dÃ©, confiance bornÃ©e par les constantes et liste dâ€™indices non vide. Cela permet une prÃ©sentation traduite ultÃ©rieure sans modifier le classement. Le diagnostic de reader nul dispose de sa clÃ© traduite.

Le classement a Ã©tÃ© complÃ©tÃ© par `RecognizeAsync`, les scopes et la collecte des Ã©checs au bloc 109 : voir `execution-recognition.md`. La composition globale et le helper gÃ©nÃ©rique de sonde restent Ã  porter.
