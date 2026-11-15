# HFE v1/v3 et conversions de flux

Les douze classes C# de `Images/Formats/Floppy/Hfe` définissent le portage : modèles, métadonnées, packing, reader v1/v3, writer et adaptateurs de pistes protégées. Les fichiers originaux restent en lecture seule. Les conversions `FluxBitCellConverter` et `ProtectedTrackFluxAdapter` sont regroupées dans `Models/Flux/Conversion`, réutilisable par d’autres conteneurs.

HFE range les bits du poids faible au poids fort dans chaque octet. Le packer MSB commun à NIB/WOZ n’est donc pas appliqué. Les faces de pistes sont entrelacées par blocs de 256 octets dans des blocs physiques de 512 octets. La table contient un offset de bloc et une longueur cumulée sur deux faces, même pour une seule tête. Les têtes absentes et fins de blocs v1 gardent le remplissage 0x88 ; les blocs d’opcodes v3 utilisent NOP.

Le lecteur valide signatures HFE/HFE v3, révision, géométrie, débit, table et accès aux faces. Le décodeur v3 conserve exactement les durées variables, cellules partielles, marque d’index et régions faibles des opcodes rate/skip/index/random. Les opcodes inconnus et paramètres manquants sont ignorés comme en C#. Les instants de transition sont arrondis à l’entier pair ; la durée interne reste en ticks de 25 ns. Les descriptions des caractéristiques utilisent les clés traduisibles du modèle LocalizedText existant.

L’écriture v1 accepte un modèle HFE ou des pistes encodées FM/MFM uniformes. La conversion des pistes protégées exige un timing uniforme représentable exactement, utilise le PGCD des durées pour unifier les pistes et refuse le padding qui changerait les données. La résolution d’encodage consulte d’abord les métadonnées HFE puis les types SCP explicitement compatibles. L’écriture protégée se replie vers v3 pour les cas de non-représentation/overflow prévus par le C# ; elle ne masque pas les erreurs de données.

Le writer v3 conserve les intervalles de flux par changements de débit et cellules partielles. Il échappe les octets dont le bas vaut 0x0f, refuse les régions faibles non préservables, les révolutions multiples, les intervalles non représentables à 36 MHz et les dépassements. Les conversions neutres vérifient aussi les exigences du contrat C# MediaFluxRevolutionData : durée d’index et intervalles strictement positifs.

Les données et buffers appartiennent aux modèles C++. Toutes les écritures par chemin réutilisent le service atomique existant et RAII, y compris en annulation ou exception. Aucune logique matérielle, UI ou dépendance MediaAnalysis n’est ajoutée.
