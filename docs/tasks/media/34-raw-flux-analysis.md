# Analyse du flux brut et codecs explicites du registre

- [x] Analyse temporelle
  - [x] Créer `src/MediaEngine/src/Codecs/Raw/Raw.h`, `Raw.cpp`, `Constants.h` : estimation commune non-FM, arrondi pair/bornage C#, classification des intervalles longs/impulsions courtes, positions et descriptions localisées, annulation et interface Decoder.
- [x] Intégration au registre
  - [x] Modifier `src/MediaEngine/src/Codecs/Iso/Mfm.h`, `Fm.h`, `Codecs/Commodore/Commodore1541.h`, `Commodore900.h` : adaptateurs Decoder concrets réutilisant les algorithmes portés, sans catalogue par défaut réduit.
- [x] Construction et traduction
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py` : compilation et constantes nommées de l’analyse brute.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/raw-flux-analysis.json` et les 29 catalogues : descriptions des deux anomalies et nom affiché du codec, traduction Argos puis relecture.
- [x] Validation
  - [x] Créer `build/verification-raw-flux/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : sources C# originales, seuils stricts, premier intervalle, arrondis et valeurs extrêmes, captures complètes et descriptions, annulation, intégration des cinq codecs au registre et collecte/cache ISO, langues Debug/Release.
  - [x] Supprimer `build/verification-raw-flux/`, `build/raw-flux-reference.log`, `build/raw-flux-bench-debug.log`, `build/raw-flux-bench-release.log` : retrait du banc et de ses artefacts.

Le catalogue complet de 25 codecs reste à assembler après portage des codecs manquants. Le nom descriptif source du résultat brut est conservé comme métadonnée de compatibilité ; une clé de traduction dédiée est fournie pour sa présentation, comme pour les descriptions.
