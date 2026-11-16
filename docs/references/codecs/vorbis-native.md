# Références natives Vorbis pour le portage CUE

Consultées le 16 novembre 2026 ; aucune dépendance n’est encore intégrée par cette note.

La page officielle https://www.xiph.org/downloads/ publie libogg 1.3.6 et libvorbis 1.3.7 comme versions stables. Archives et SHA-256 :

- https://downloads.xiph.org/releases/ogg/libogg-1.3.6.tar.xz : 5c8253428e181840cd20d41f3ca16557a9cc04bad4a3d04cce84808677fa1061
- https://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.xz : b33cc4934322bcbf6efcbacf49e3ca01aadbea4114ec9589d1b1e9d20f72954b

API officielle : https://xiph.org/vorbis/doc/vorbisfile/reference.html

https://xiph.org/vorbis/doc/vorbisfile/ov_open_callbacks.html permet de fournir une source et des callbacks propres ; l’application doit fermer ses ressources si l’ouverture échoue. Un callback close nul laisse cette responsabilité à l’application. Cette API évite les problèmes de mélange des CRT Windows signalés par la documentation de ov_open.

https://xiph.org/vorbis/doc/vorbisfile/ov_read.html rend du PCM entrelacé avec choix de l’endianness, de la taille et du signe. Le nombre d’octets retournés peut être inférieur à la demande. Les flux chaînés peuvent changer de fréquence ou de nombre de canaux ; ne pas se limiter au premier lien.

Avant remplacement éventuel du décodeur C#, vérifier ses règles de granules, chaînes, conversion float/PCM, trous, seek et annulation. Le C# possède un décodeur interne et une source avec lectures non alignées à la frame ; le profil CD exige stéréo 44100 Hz. Il ne faut pas déclarer cette partie portée sur la seule base de la présence d’une bibliothèque native.
