# Références natives Vorbis pour le portage CUE

Consultées le 16 novembre 2026. Tremor et libogg sont désormais intégrés ; libvorbis flottant n’est pas intégré.

La page officielle https://www.xiph.org/downloads/ publie libogg 1.3.6 et libvorbis 1.3.7 comme versions stables. Archives et SHA-256 :

- https://downloads.xiph.org/releases/ogg/libogg-1.3.6.tar.xz : 5c8253428e181840cd20d41f3ca16557a9cc04bad4a3d04cce84808677fa1061
- https://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.xz : b33cc4934322bcbf6efcbacf49e3ca01aadbea4114ec9589d1b1e9d20f72954b

API officielle : https://xiph.org/vorbis/doc/vorbisfile/reference.html

https://xiph.org/vorbis/doc/vorbisfile/ov_open_callbacks.html permet de fournir une source et des callbacks propres ; l’application doit fermer ses ressources si l’ouverture échoue. Un callback close nul laisse cette responsabilité à l’application. Cette API évite les problèmes de mélange des CRT Windows signalés par la documentation de ov_open.

https://xiph.org/vorbis/doc/vorbisfile/ov_read.html rend du PCM entrelacé avec choix de l’endianness, de la taille et du signe. Le nombre d’octets retournés peut être inférieur à la demande. Les flux chaînés peuvent changer de fréquence ou de nombre de canaux ; ne pas se limiter au premier lien.

Avant remplacement éventuel du décodeur C#, vérifier ses règles de granules, chaînes, conversion float/PCM, trous, seek et annulation. Le C# possède un décodeur interne et une source avec lectures non alignées à la frame ; le profil CD exige stéréo 44100 Hz. Il ne faut pas déclarer cette partie portée sur la seule base de la présence d’une bibliothèque native.

## Décision appliquée et provenance

Le C# (`VorbisSynthesis.cs`, `VorbisTransform*.cs`, `OggPacketReader.cs`) utilise des entiers : conversion PCM par décalage de 9 bits puis saturation 16 bits. La branche principale Tremor officielle reprend ce chemin (`vorbisfile.c`, `CLIP_TO_15(src[j]>>9)`), contrairement à libvorbis flottant. Dépôt officiel indiqué par https://wiki.xiph.org/Tremor : https://gitlab.xiph.org/xiph/tremor.git ; révision intégrée `820fb3237ea81af44c9cc468c8b4e20128e3e5ad`, vérifiée par Git. Le site GitLab est refusé par l’outil web (403), mais le clone Git officiel réussit. L’URL d’index https://media.xiph.org/vorbis/ renvoie 404 ; aucun échantillon n’en est récupéré.

`third_party/tremor` conserve les 13 sources C de Makefile.am, les en-têtes et COPYING ; les programmes d’exemple, documentation HTML et anciens projets Visual Studio ne sont pas intégrés. Les deux tables suppriment un `const` redondant puisque LOOKUP_T le contient déjà. Le CMake local charge stdlib avant les macros min/max sous MSVC ; ailleurs il configure endianness/alloca et les calculs entiers sans optimisation de débordement signé. Le mode de précision réduite et les optimisations assembleur ARM optionnelles ne sont pas activés.

`third_party/ogg` conserve libogg 1.3.6 : bitwise.c, framing.c, crctable.h, ogg.h, os_types.h et COPYING. Le config_types.h local emploie stdint.h pour les six types entiers de largeur fixe ; le CMake local ne compile aucun exemple/test. SHA-256 de l’archive vérifié avant extraction, valeur ci-dessus. Les bibliothèques sont statiques dans MediaEngine et leurs licences sont installées séparément ; aucun réseau au build.

L’adaptateur `PcmSource.cpp` possède le décodeur et sa source, sérialise ses lectures, intercepte toutes les exceptions dans les callbacks C et les relance côté C++. Son close callback est nul : la source reste possédée par le C++. L’ouverture native invalide libère elle-même son état ; une ouverture réussie est toujours suivie de ov_clear lors de la destruction ou d’une erreur de lecture. Après une telle erreur, une lecture ultérieure rouvre le décodeur. La sortie native est convertie en little-endian sur les hôtes big-endian.

Les chaînes de même profil sont prises en charge. Les changements de canaux/fréquence entre liens sont explicitement refusés : une source PCM à frames de taille fixe ne doit pas les concaténer silencieusement sous le profil du premier lien. BIN/CUE impose en plus stéréo 44100 Hz et ignore seulement la fraction terminale de secteur CD, comme le C#. Les lectures peuvent commencer et finir au milieu d’une frame. Une erreur/trou du décodeur est remontée, sans remplacement silencieux par du silence.

Compilation et preuves ciblées : `docs/validation/media/bin-cue-vorbis.md`. La compilation Windows ne constitue pas une validation d’exécution Linux/macOS.
