# Images Acorn Winchester DAT/DSC

Références C# en lecture seule : `Images/Formats/HardDisk/Acorn/AcornWinchesterFormat.cs`, `AcornWinchesterReader.cs`, `AcornWinchesterWriter.cs`, puis `Adaptec/AdaptecMfmGeometry.cs` et les constantes physiques associées. Les empreintes correspondent à l'inventaire.

Le port sépare le descripteur DSC, le lecteur, le writer et les paramètres physiques Adaptec. Seule la projection `WriteInitialParameters` d'Adaptec est portée à cette étape ; la lecture des identités physiques et le codec de pistes restent à faire et sont marqués partiels dans l'inventaire.

La géométrie DSC accepte 256, 512 ou 1024 octets par secteur. Le code de taille historique nul représente 256 octets. Les nombres de secteurs sont 33/32, 18/17 ou 9, selon la taille et la longueur exacte du DAT ; une image partielle conserve la disposition interleavée habituelle. Les bornes du conteneur et celles des paramètres physiques Adaptec restent distinctes.

La mise à jour DSC conserve tous les octets si la géométrie n'a pas changé. Sinon seuls les champs prescrits par le C# sont réécrits. La projection physique reprend les mots transformés de courant réduit, les bits de tête, pas et zone d'atterrissage du code C#, sans inventer de paramètres supplémentaires.

Le lecteur reconnaît une demande explicite, un DAT accompagné d'une géométrie DSC valide ou les signatures Hugo aux deux positions du C#. Il choisit d'abord un DSC dans les fichiers associés, sinon la même base de nom avec l'extension dsc. Le descripteur n'est lu qu'une fois par opération de lecture. La capacité annoncée par le DSC est conservée ; les secteurs manquants deviennent des plages indisponibles. Les avertissements C# en texte brut deviennent des recettes de message traduisibles. Les métadonnées contiennent la taille de bloc et le DSC natif en base64.

Le writer exige une géométrie prise en charge et une couverture complète. Il conserve le descripteur natif, agrandit les descripteurs trop courts à 33 octets et refuse un base64 invalide. DAT et DSC sont publiés par le mécanisme commun récupérable. `createAsync` refuse tout remplacement et propose une préallocation optionnelle du DAT, dans les limites de l'adaptation de plateforme existante.

Les trois sources Acorn sont portées ; le matériel Adaptec, les autres conteneurs et l'intégration globale depuis App restent en cours.
