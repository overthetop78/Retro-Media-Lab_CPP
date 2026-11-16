# Descriptions des structures de flux

Le helper C# de construction de texte est remplacé par des recettes localisées propres aux codecs et le résolveur `Decoding/LocalizedText.cpp`. Les identités, intégrités, marques, variantes, états tronqués et données non appariées sont transportés par les recettes et leurs paramètres ; il n’y a pas de nouvelle interprétation de flux dans ce remplacement.

| Consommateur C# | Méthodes du helper utilisées | Implantation C++ existante |
|---|---|---|
| AppleIwmGcrDecoder.cs | Complete, Truncated, UnpairedData, WithIntegrity | src/MediaEngine/src/Codecs/AppleIwm/Gcr.h; src/MediaEngine/src/Codecs/AppleIwm/Decoder.cpp |
| Aed6200pMfmDecoder.cs | Complete, Truncated, UnpairedData, WithIntegrity | src/MediaEngine/src/Codecs/Aed6200p/Decoder.cpp;src/Translations/Catalogs/Constants/MediaEngine/aed6200p-mfm.json |
| AmigaMfmDecoder.cs | CompleteWithChecksums | src/MediaEngine/src/Codecs/Amiga/Decoder.cpp; src/MediaEngine/src/Codecs/Amiga/Mfm.h |
| AppleIIGcrDecoder.cs | Complete, Truncated, UnpairedData, WithIntegrity | src/MediaEngine/src/Codecs/AppleII/Decoder.cpp; src/MediaEngine/src/Codecs/AppleII/Gcr.h |
| AppleRwts18Decoder.cs | WithIntegrity | src/MediaEngine/src/Codecs/AppleRwts18/Decoder.cpp; src/MediaEngine/src/Codecs/AppleRwts18/Gcr.h |
| ArburgDecoder.cs | WithIntegrity | src/MediaEngine/src/Codecs/Arburg/Decoder.cpp |
| CenturionMfmDecoder.cs | Truncated | src/MediaEngine/src/Codecs/Centurion/Decoder.cpp |
| IsoMfmDecoder.cs | UnclassifiedMark | src/MediaEngine/src/Codecs/Iso/MfmDecoder.cpp; src/MediaEngine/src/Codecs/Iso/MfmRead.cpp; src/MediaEngine/src/Codecs/Iso/Mfm.h |
| MembrainMfmDecoder.cs | Complete, Identity, Integrity, Truncated, UnpairedData | src/MediaEngine/src/Codecs/Membrain/Decoder.cpp;src/MediaEngine/src/Codecs/Membrain/Fields.cpp;src/Translations/Catalogs/Constants/MediaEngine/membrain-mfm.json |
| MicralNFmDecoder.cs | Identity, Integrity, Truncated | src/MediaEngine/src/Codecs/MicralN/Decoder.cpp; src/MediaEngine/src/Codecs/MicralN/Fm.h |
| MicropolisMfmDecoder.cs | Identity, Integrity, Truncated | src/MediaEngine/src/Codecs/Micropolis/Decoder.cpp |
| NorthstarMfmDecoder.cs | Identity, Integrity, Truncated | src/MediaEngine/src/Codecs/Northstar/Decoder.cpp |
| QdMo5MfmDecoder.cs | Complete, Identity, Integrity, Truncated, UnpairedData | src/MediaEngine/src/Codecs/QdMo5/Decoder.cpp;src/MediaEngine/src/Codecs/QdMo5/Fields.cpp;src/Translations/Catalogs/Constants/MediaEngine/qd-mo5-mfm.json |
| RawFluxDecoder.cs | UnclassifiedMark | src/MediaEngine/src/Codecs/Raw/Raw.h; src/MediaEngine/src/Codecs/Raw/Raw.cpp |
| TycomFmDecoder.cs | Complete, Identity, Integrity, Truncated, UnclassifiedMark | src/MediaEngine/src/Codecs/Tycom/Decoder.cpp;src/MediaEngine/src/Codecs/Tycom/Fields.cpp;src/MediaEngine/src/Models/Flux/BitReader.cpp;src/Translations/Catalogs/Constants/MediaEngine/tycom-fm.json |
| Victor9kGcrDecoder.cs | Complete, Identity, Integrity, Truncated, UnpairedData | src/MediaEngine/src/Codecs/Victor9000/Decoder.cpp; src/MediaEngine/src/Codecs/Victor9000/Fields.cpp; src/MediaEngine/src/Codecs/Victor9000/Gcr.h |
| CenturionMfmDescriptions.cs | Complete, Truncated, UnpairedData, WithIntegrity | src/Translations/Catalogs/Constants/MediaEngine/centurion-mfm.json |
| Commodore900GcrDescriptions.cs | Integrity, UnclassifiedMark, UnpairedData, WithIntegrity | src/MediaEngine/src/Codecs/Commodore/Commodore900Constants.h |
| CommodoreGcrDescriptions.cs | Complete, Identity, Integrity, UnclassifiedMark | src/MediaEngine/src/Codecs/Commodore/Commodore1541Decoder.cpp |
| DataGeneralFmDescriptions.cs | Identity, WithIntegrity | src/MediaEngine/src/Codecs/DataGeneral/Decoder.cpp; src/Translations/Catalogs/Constants/MediaEngine/datageneral-fm.json |
| DecRx02Descriptions.cs | Complete, Truncated, UnclassifiedMark, WithIntegrity | src/MediaEngine/src/Codecs/DecRx02/Descriptions.h; src/MediaEngine/src/Codecs/DecRx02/Descriptions.cpp; src/Translations/Catalogs/Constants/MediaEngine/rx02-track.json |
| EmuFmDescriptions.cs | Complete, UnclassifiedMark, WithIntegrity | src/MediaEngine/src/Codecs/Emu/Decoder.cpp; src/Translations/Catalogs/Constants/MediaEngine/emu-fm.json |
| HeathkitFmDescriptions.cs | Complete, Truncated, UnpairedData, WithIntegrity | src/MediaEngine/src/Codecs/Heathkit/Decoder.cpp; src/Translations/Catalogs/Constants/MediaEngine/heathkit-fm.json |
| HpMmfmDescriptions.cs | WithIntegrity | src/MediaEngine/src/Codecs/HpMmfm/Decoder.cpp; src/Translations/Catalogs/Constants/MediaEngine/hp-mmfm.json |
| IsoFmDescriptions.cs | Complete, UnclassifiedMark, WithIntegrity | src/MediaEngine/src/Codecs/Iso/FmDecoder.cpp; src/Translations/Catalogs/Constants/MediaEngine/iso-fm-decoding.json |
| IsoMfmDescriptions.cs | Complete, UnclassifiedMark, WithIntegrity | src/MediaEngine/src/Codecs/Iso/MfmDecoder.cpp; src/Translations/Catalogs/Constants/MediaEngine/iso-mfm-decoding.json |

93 clés de codec sont présentes dans Constants et dans les 29 catalogues de langues. Ce contrôle vérifie la présence des textes, pas la justesse linguistique complète ; la relecture des traductions figure dans les blocs de codec correspondants. Aucun décodeur n’a été relancé.
