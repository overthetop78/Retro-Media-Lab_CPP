# WAVE séquentiel

Référence C# en lecture seule : Constants/WavConstants.cs et Images/Formats/Tape/Wav/WavTapeFormat.cs, WavTapeReader.cs, WavTapeWriter.cs dans F:/GW GUI/src/RetroMediaLab.MediaEngine. SHA-256 des quatre sources vérifiés contre l’inventaire le 16 novembre 2026.

Le profil est distinct de l’audio CD BIN/CUE : PCM entier, 1 à 32 canaux, fréquence 1 à 768000 Hz, profondeur 8/16/24/32 bits. Le reader borne les chunks au conteneur RIFF, conserve tous les chunks data et les inconnus, utilise la dernière description fmt, et n’exige pas le byte rate que le C# ne vérifie pas. Les segments partagent la source possédée du contexte et gardent offsets, temps et canaux. Les octets suivant RIFF produisent un avertissement traduisible.

Le writer reprend les métadonnées sans rééchantillonnage. Tous les segments doivent être des échantillons possédant une source ; seuls les canaux non spécifiés ou zéro sont copiés. Les chunks inconnus empêchent une suppression silencieuse. Les données sont copiées par blocs de 64 KiB dans le writer atomique existant ; aucune seconde copie temporaire ni média entier en mémoire n’est nécessaire.

Corrections de fiabilité par rapport au C# : un chunk inconnu vide est conservé avec longueur absente, car le modèle interdit une longueur explicite nulle ; la longueur RIFF écrite inclut l’octet de remplissage impair et respecte réellement UINT32_MAX ; un range de sortie interrompant une trame PCM est refusé plutôt que produire un fichier rejeté ensuite par le reader. Les limites d’alignement et de taille restent vérifiées avant publication ; les ressources sont partagées/RAII et l’annulation conserve la destination préexistante.

La composition globale et le raccordement de ce reader/writer restent séparés et ne sont pas déclarés terminés.
