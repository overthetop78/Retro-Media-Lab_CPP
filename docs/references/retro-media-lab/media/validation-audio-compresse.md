# Audio compressé

AudioTapeReader.cs est la référence du comportement. Six extensions conservées : MP3, OGG, FLAC, AAC, M4A et WMA. OGG réutilise le décodeur Vorbis à accès paresseux. Les autres utilisent Qt Multimedia depuis une QIODevice qui possède la source ; les callbacks ne propagent aucune exception à travers Qt. Les buffers sont normalisés en PCM16LE avec arrondi au pair et bornage, puis partagés entre les segments de canaux intercalés. Les variations de profil et dépassements sont refusés. L’arrêt du décodeur et sa destruction sont garantis sur toutes les sorties.

Contrôle Debug ciblé : véritables fichiers des six formats, stéréo 44100 Hz générée avec FFmpeg 9.0.2, profils et lecture des canaux vérifiés. La mémoire PCM survit à la suppression du fichier source ; fermeture puis suppression des fichiers réussies. Entrée invalide, annulation avant démarrage et pendant décodage, décodage sur thread de travail : réussis. Aucun média de test conservé.

Compilation Debug sans avertissement. Les catalogues audio des 29 langues sont générés, relus et compilés. Audit : 1348 fichiers C++, 56 clés App et 29 langues.

Le déploiement inclut Qt Multimedia et son plugin. Les DLL FFmpeg de Qt sont classées sous lib/FFmpeg, répertoire ajouté au chargement sécurisé du lanceur. Le contrôle windeployqt confirme leur collecte et celle du plugin. L’intégration de l’exécutable autonome et des conversions audio est une étape distincte, demandée ensuite.

Sources primaires :
- https://doc.qt.io/qt-6/qaudiodecoder.html : décodage asynchrone, source QIODevice, buffers, arrêt et erreurs.
- https://doc.qt.io/qt-6.12/qtmultimedia-index.html : backend FFmpeg et déploiement des bibliothèques.
- https://doc.qt.io/QT-6/windows-deployment.html : collecte des dépendances avec windeployqt.
- https://ffmpeg.org/ffmpeg.html : options de génération des fixtures.
