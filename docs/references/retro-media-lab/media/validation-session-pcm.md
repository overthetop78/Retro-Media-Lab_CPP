# Session PCM — bloc 230

Référence en lecture seule : `Images/Reading/Audio/PcmPlaybackSession.cs` de MediaEngine C#.

La session native expose les canaux, la fréquence, le nombre et la position des trames, la lecture signée 16 bits entrelacée, le déplacement et une fermeture idempotente. L’ouverture réutilise la reconnaissance `tape.audio` existante. La création depuis un document déjà décodé évite un nouveau décodage. Les interfaces publiques ne révèlent aucune classe MediaFileSystems ni aucun composant graphique.

Le tampon de travail de 8 192 octets est réutilisé. Une lecture consomme uniquement des trames complètes, respecte le décalage de la plage source, et n’avance la position qu’après la réussite de toute la demande. Les plages invalides sont refusées avant utilisation. Les erreurs de données, de position ou d’accès à une session fermée réutilisent le diagnostic traduit `AudioPcmData`.

Les sources sont partagées par des propriétaires RAII, sans libération manuelle répétée d’un même objet. La fermeture abandonne les références du document et de la plage avant la remise à zéro de la mécanique. Si celle-ci échoue, les références sont déjà libérées et une nouvelle fermeture n’effectue pas une seconde remise à zéro. Le destructeur assure ce nettoyage sans laisser sortir d’exception. La création échouée remet également à zéro la mécanique du document.

Compilation Debug réussie, limitée au nouveau fichier et à la liaison de MediaEngine. Le contrôle temporaire a vérifié : lecture multicanale traversant plusieurs tampons, fin de flux, destination trop courte pour une trame, déplacement aux bornes, annulation, erreur de source sans avancement, métadonnées invalides, fermeture répétée, destruction automatique et libération malgré une erreur de remise à zéro. Il utilise des données générées en mémoire et ne reteste aucun décodeur audio. Son dossier et ses artefacts ont été supprimés après usage.
