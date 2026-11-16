# FFmpeg intégré et conversion audio

Demande explicite : embarquer FFmpeg pour permettre WAV vers un autre format, un format compressé vers WAV et les conversions entre formats compressés. Aucun nouvel écran n’est ajouté ici.

La distribution Windows contient lib/FFmpeg/ffmpeg.exe, LICENSE, README.txt et package.json. La construction utilise le cache vérifié build/dependencies/ffmpeg/9.0.2. En son absence, le script télécharge l’archive fixe et vérifie sa somme SHA-256 avant extraction puis celle de l’exécutable. Une installation correspondant au manifeste peut aussi alimenter ce cache. L’application ne dépend pas du PATH de l’utilisateur. Le binaire statique est séparé des DLL utilisées par le backend Qt.

API publique : src/MediaEngine/include/RetroMediaLab/MediaEngine/AudioConversion.h. AudioConversionService reçoit des chemins natifs, un AudioOutputFormat et des paramètres optionnels de fréquence, nombre de canaux et débit. Les cibles sont WAV PCM16, MP3, OGG Vorbis, FLAC, AAC ADTS, M4A AAC et WMA. Les autres futures opérations peuvent réutiliser executable(). App ne lance ni ne pilote le processus.

Les arguments sont transmis à QProcess séparément sans shell. Aucun remplacement de la source n’est permis. Sans overwrite, un fichier existant est refusé ; avec overwrite, une sortie atomique est produite après conversion réussie. L’annulation ou une erreur tue et attend le processus si nécessaire et supprime le temporaire. Le processus Windows est caché. Les erreurs du service utilisent des clés de traduction ; aucun texte stderr de FFmpeg n’est imposé au GUI.

Validation ciblée Debug réussie : les sept sorties à partir d’un WAV, MP3 vers FLAC vers WAV, sélection mono 22050 Hz, identité des échantillons PCM après WAV-FLAC-WAV, chemins comportant espaces et accents, refus d’écrasement, source invalide sans altération d’une destination existante, écrasement explicite réussi, source identique refusée, annulation préalable et active, absence de temporaires restants, résolution du chemin embarqué. Le premier contrôle a révélé un renommage Windows bloqué par le handle de QTemporaryFile : corrigé avec rename sur le propriétaire puis transfert de propriété de la sortie.

Sources primaires consultées et conservées :
- https://ffmpeg.org/ffmpeg.html : options de sélection du flux audio, encodeur, conteneur, fréquence, canaux et absence de lecture de stdin.
- https://www.gyan.dev/ffmpeg/builds/ : construction Windows statique, licence et code source correspondant.
- https://raw.githubusercontent.com/microsoft/winget-pkgs/master/manifests/g/Gyan/FFmpeg/9.0.2/Gyan.FFmpeg.installer.yaml : archive fixe et empreinte publiée.

La vérification de cette API ne valide pas toute l’architecture MediaEngine/MediaFileSystems/MediaAnalysis. Les façades globales et leur raccordement à App restent dans l’objectif général.

Construction Debug complète et préparation du paquet réussies. Une conversion a été exécutée depuis le répertoire de distribution avec AudioConversionService par défaut et un PATH limité aux DLL du paquet et à Windows ; aucune installation hôte de Qt ou FFmpeg n’a été utilisée. L’empreinte du ffmpeg.exe distribué correspond au manifeste. Les DLL FFmpeg et le plugin Qt Multimedia sont présents dans les catégories prévues. Le contrôle temporaire copié dans la distribution a été supprimé.

Audit final : 1351 fichiers C++, 56 clés App, 29 langues, constantes et diagnostics conformes. Les six nouveaux diagnostics ont été générés par Argos, relus et corrigés dans les langues nécessitant une correction, puis leurs catalogues ont été compilés dans la construction complète.
