# Décodage de la banque mécanique

RecordingDecoder décode les FLAC mono ou stéréo de la banque via FFmpeg déjà embarqué. La structure STREAMINFO initiale est vérifiée pour déterminer fréquence et nombre de canaux. Les autres nombres de canaux sont refusés comme dans le C#. La durée est calculée sur le nombre d’échantillons réellement décodés à la fréquence native, et non sur une durée déclarée du catalogue.

Sortie : PCM mono flottant 32 bits little-endian dans un pipe, sans WAV intermédiaire, sans fichier temporaire, sans ouverture de périphérique audio. En stéréo, le filtre pan applique explicitement 0,5 à chaque canal ; le mono conserve son gain. FFmpeg réalise le rééchantillonnage à la fréquence demandée. Cela remplace le rééchantillonneur WDL du C# : le résultat n’est pas déclaré identique à chaque échantillon après changement de fréquence.

RecordedBank construit le RecordedRenderer avec les 25 profils et les loaders réels. Sa racine par défaut est Assets/MechanicalAudio sous le dossier de l’exécutable ; FFmpeg utilise le chemin embarqué du service de conversion. Les chemins natifs préservent UTF-8 sur Windows. Des chemins explicites permettent aussi l’utilisation du service hors de la distribution.

Le processus est lancé sans shell et masqué sous Windows. Un propriétaire RAII tue et attend le processus encore actif sur toute exception ; les flux sont drainés, la taille totale est bornée par la limite PCM commune et les flottants non finis sont refusés. L’annulation est vérifiée avant lancement, pendant l’attente et pendant la conversion du buffer. Les diagnostics réutilisent les clés PCM/FFmpeg déjà traduites ; stderr externe n’est pas affiché comme message applicatif.

## Vérification

Compilation Debug réussie. L’avertissement u8path C++20 a été corrigé sans le masquer. Contrôle ciblé réussi : FLAC mono constant 0,25 et stéréo 0,25/0,75 donnant exactement 0,5 en mono à la fréquence native ; conversion de 4 800 frames à 48 kHz en 4 410 frames à 44,1 kHz avec gain conservé ; source invalide ou absente, exécutable absent et annulation préalable ou déclenchée depuis un autre thread. Banque réelle Tape/Standard chargée : rendu, durée et préparation ; une fois les copies des sons supprimées, rendu et durée déjà chargés restent disponibles via le cache.

Audit des constantes : 1 389 fichiers C++, 56 erreurs App, 29 langues, aucune valeur applicative hors constantes nommées. Le contrôle temporaire, ses WAV/FLAC, sa copie de banque et ses exécutables ont été supprimés. Aucun changement dans le C#.

Le décodeur de ce composant accepte les FLAC de production ; le décodeur générique des autres médias audio reste distinct. prepareProfile prépare les PCM ; le cache des durées reste distinct, comme en C#. Les premiers chargements impliquent le lancement de FFmpeg : aucune mesure de latence de préparation, de temps réel ou de consommation mémoire en émulation n’est revendiquée. Les mécaniques de lecteurs et leur composition globale restent à raccorder.

## Sources conservées

- [FFmpeg, filtre pan](https://ffmpeg.org/ffmpeg-filters.html#pan-1) : coefficients explicites et disposition mono.
- [FFmpeg, formats PCM bruts](https://ffmpeg.org/ffmpeg-formats.html#Raw-PCM-muxers) : sortie f32le.
- [FFmpeg, options](https://ffmpeg.org/ffmpeg.html) : sélection du flux audio, fréquence et encodeur ; déjà référencé dans ffmpeg-audio.md.
- [FLAC, STREAMINFO](https://www.rfc-editor.org/rfc/rfc9639.html) : fréquence et canaux dans les métadonnées initiales ; référence déjà conservée dans validation-sons-mecaniques.md.

La page Qt 6.12 QAudioDecoder a expiré à la consultation ; aucune conclusion sur sa conversion de fréquence n’en a été tirée.
