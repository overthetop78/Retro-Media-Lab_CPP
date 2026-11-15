# Reconnaissance et lecture Apple brute

Bloc 66 : AppleRawImageProbe, AppleRawImageReader, AppleII525RawImageReader, Apple35RawImageReader, AppleRawImageReadResult et AppleIISectorOrderConverter.

Les sondes de conteneur/capacité et la construction des images restent dans `MediaEngine/Formats/Apple`. Les structures de systèmes de fichiers restent dans `MediaFileSystems/FileSystems/Apple/Recognition.cpp`, accessibles par le contrat AppleRecognition. Aucun accès direct à Analysis, App ou au matériel n’est ajouté.

Le service MFS expose la validation VTOC DOS, la validation structurelle du bloc racine ProDOS, la signature du bloc maître Macintosh, les en-têtes Lisa et le marqueur SOS borné. La validation ProDOS ne prétend pas porter le décodage complet du nom, du bitmap ou des dates : ces fonctions restent à porter. Lisa DecodeName conserve le décodage Latin1, coupe au premier nul et retire uniquement les espaces périphériques. Les sondes bornent les accès avant toute lecture.

La reconnaissance `.img` accepte Lisa Office ou une capacité Macintosh avec signature MFS/HFS ; `.dsk` accepte aussi la capacité Apple II 140 Kio. Comme en C#, requestedFormat n’intervient pas dans cette sonde. Le lecteur brut accepte les capacités cataloguées indépendamment de cette sonde d’extension : D13, 140 Kio, 400/800 Kio et 1,44 Mio.

À 140 Kio : PO/SOS, indice PO, ProDOS direct, DOS direct, SOS réordonné, ProDOS réordonné, puis repli DOS 3.3. À 400/800 Kio et 1,44 Mio : Lisa Office avant Macintosh, puis ProDOS pour les capacités applicables, sinon diagnostic de structure inconnue. Les numéros de secteurs sont à base zéro. Le résultat conserve la preuve ValidatedStructure, ExtensionHint ou GeometryFallback.

DOS→ProDOS compose les permutations géométriques existantes. La permutation composée est involutive : le C# utilise donc aussi DosToProDos pour ProDosToDos. L’entrée vide est acceptée ; toute longueur non multiple d’une piste est refusée. Les buffers sources ne sont pas modifiés. Les images construites possèdent leurs copies sectorielles, libérées par RAII en cas d’erreur ou d’annulation. Le token d’annulation est une extension explicite du lecteur C++.

Les erreurs de longueur, géométrie inconnue et annulation réutilisent les diagnostics existants. Deux clés ajoutées concernent le numéro de secteur Apple II et les capacités connues sans structure reconnue. Les 29 langues sont générées par Argos puis relues/corrigées ; paramètres et invariants sont contrôlés. L’erreur de capacité absente du catalogue utilise le diagnostic générique RawUnknownGeometry plutôt que le message détaillé des sondes C#.

Les writers Apple, conteneurs et parseurs complets des systèmes de fichiers restent à porter ; ce service de reconnaissance ne les remplace pas.
