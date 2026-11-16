# Reconnaissance commune et adaptateurs historiques

Le C# contient encore un registre de reconnaissance sectorielle historique. Son constructeur prenant `MediaImageReadingService` est explicitement décrit comme adaptateur temporaire ; aucune construction de ce registre ni de ses politiques ne subsiste dans la composition MediaEngine actuelle. Le C++ conserve la chaîne commune et ses lecteurs, sans reconstruire une seconde chaîne devenue inutilisée.

| Élément historique | Implantation native du rôle conservé |
|---|---|
| DiskImageNotRecognizedException.cs | `src/MediaEngine/src/Images/RecognitionResult.h` ; `src/MediaEngine/src/Images/ReadRejection.cpp` |
| DiskImageRecognitionContext.cs | `src/MediaEngine/src/Images/RecognitionContext.h` ; `src/MediaEngine/src/Images/RecognitionContext.cpp` |
| DiskImageRecognitionExceptions.cs | `src/MediaEngine/src/Images/RecognitionResult.h` ; `src/MediaEngine/src/Images/ReaderRegistry.cpp` ; `src/MediaEngine/src/Images/ReadRejection.cpp` |
| DiskImageRecognitionFailure.cs | `src/MediaEngine/src/Images/RecognitionResult.h` |
| DiskImageRecognitionRegistry.cs | `src/MediaEngine/src/Images/ReaderRegistry.cpp` ; `src/MediaEngine/src/Images/ReadingService.cpp` |
| AmstradImageRecognitionPolicy.cs | `src/MediaEngine/src/Formats/CpcDsk/Reader.cpp` |
| AppleImageRecognitionPolicy.cs | `src/MediaEngine/src/Formats/Apple/Adapter.cpp` |
| CoherentImageRecognitionPolicy.cs | `src/MediaEngine/src/Formats/Raw/SpecializedReaders.cpp` ; `src/MediaEngine/src/Formats/Commodore900/Reader.cpp` |
| DecRx02ImageRecognitionPolicy.cs | `src/MediaEngine/src/Formats/Rx02/Adapter.cpp` |
| ExtensionHintRecognitionPolicy.cs | `src/MediaEngine/src/Images/ReaderRegistry.cpp` |
| MsxImageRecognitionPolicy.cs | `src/MediaEngine/src/Formats/Raw/Adapters.cpp` |
| RawImgRecognitionPolicy.cs | `src/MediaEngine/src/Formats/Raw/ImgReader.cpp` |
| ReaderBackedRecognitionPolicy.cs | `src/MediaEngine/src/Images/Readers.h` |
| ScpRecognitionPolicy.cs | `src/MediaEngine/src/Reconstruction/Scp/Reader.cpp` ; `src/MediaEngine/src/Images/ReadingService.cpp` |
| IAdditionalImageInterpretationPolicy.cs | `src/MediaEngine/src/Images/Recognition/Interpretations.h` ; `src/MediaEngine/src/Images/Recognition/Interpretations.cpp` |
| IDiskImageRecognitionPolicy.cs | `src/MediaEngine/src/Images/Readers.h` ; `src/MediaEngine/src/Images/ReaderRegistry.h` |
| IRecognizedImageNormalizer.cs | `src/MediaEngine/src/Images/Recognition/Interpretations.h` ; `src/MediaEngine/src/Images/Recognition/Interpretations.cpp` |

La sélection native tient compte du format explicitement demandé, des extensions, des signatures, des fichiers associés et du résultat de la sonde du lecteur. `ReaderRegistry` conserve les rejets et leurs causes ; les erreurs non classées comme rejet, les erreurs d’entrée/sortie et l’annulation ne sont pas transformées en échec silencieux. Les interprétations supplémentaires et les normalisations restent séparées de la lecture.

Point RX02 : le lecteur commun C# `DecRx02Reader` vérifie le format demandé et la capacité, exactement comme `Formats/Rx02/Adapter.cpp`. La sonde RT-11 supplémentaire appartenait à l’ancienne politique. Elle n’est pas introduite dans le lecteur actuel, afin de ne pas modifier le comportement de la chaîne commune.

Ce rapprochement est une revue des chemins actuels, pas une nouvelle campagne de tests des lecteurs. Les validations déjà réalisées lors de leur portage ne sont pas relancées.
