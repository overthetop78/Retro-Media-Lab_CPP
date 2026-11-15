# Contrats communs ImageDisk, CPC DSK et HFE

Référence : sources C# locales en lecture seule sous `F:/GW GUI/src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/`, fichiers `ImageDisk/ImdReader.cs`, `CpcDsk/CpcDskReader.cs` et `Hfe/HfeReader.cs`.

Les adaptateurs résident auprès de chaque backend existant. Ils ne dupliquent ni décodage ni reconstruction et ne dépendent pas d’App.

- ImageDisk annonce IMD et les six formats Epson connus, accepte aussi le préfixe Epson dynamique, puis sonde les trois octets IMD. La lecture détaillée fournit l’image sectorielle et les métadonnées structurelles nécessaires à sa restauration.
- CPC DSK annonce DSK/EDSK et le format conteneur neutre. La sonde exige le bloc d’information minimal et l’une des deux signatures ; elle n’attribue aucune machine à partir de l’extension. La lecture réutilise les secteurs et leurs métadonnées natives du backend.
- HFE annonce une représentation flux, reconnaît les signatures v1/v3 après vérification de la longueur minimale, et conserve les pistes protégées ainsi que les métadonnées révision/encodage/débit. Aucune conversion implicite vers des secteurs.

Comme en C#, les sondes ne filtrent pas arbitrairement l’extension. Le format demandé est contrôlé avant lecture. Les erreurs d’E/S, d’annulation et de conteneur restent celles des composants existants ; aucun texte d’exception supplémentaire n’est introduit. Les futures possèdent une copie du contexte et ne capturent pas le pointeur du reader.

Ce raccordement n’est pas encore la composition globale de tous les lecteurs de MediaEngine. Les wrappers communs WOZ/NIB et d’autres formats restent à porter.
