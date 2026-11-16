# Conteneur et codec Atom TAP

Références : `F:/GW GUI/src/RetroMediaLab.MediaEngine/Constants/AtomTapeConstants.cs` et `Images/Formats/Tape/AtomTap/{AtomTapReader,AtomTapWriter,AtomTapCodec}.cs`. Sources inchangées, SHA vérifiés dans l’inventaire.

Le lecteur valide les noms ASCII et le framing des fichiers TAP successifs (header de 22 octets), puis reconstruit les blocs Atom de 256 octets au maximum. Il reprend les quatre octets de synchronisation, les adresses big-endian de la série, les flags d’historique/continuation, le checksum, le débit 300, les porteuses et les gaps du C#. Les segments portent les métadonnées UEF existantes ; aucun accès mémoire machine n’est ajouté.

Le codec est séparé entre décodage des segments, parcours du flux série et encodage des fichiers. Il garde les segments non décodés et signale les blocs tronqués, fichiers incomplets, checksums/séquences invalides et octets série résiduels par recettes traduisibles. Le test de longueur protège aussi les blocs trop courts avant recherche du nom ; le C# pouvait appeler la recherche avec un index supérieur à leur taille. Les données restent possédées et les sorties sont nettoyées par RAII.

Le writer garde le contrôle de format du C#, refuse de supprimer les segments ou diagnostics non résolus, exige des fichiers complets/valides avec noms imprimables et adresses 16 bits, puis publie atomiquement les headers TAP et les données. Un fichier vide est un fichier valide ; une liste de fichiers vide est refusée.

Compilation Debug réussie sans avertissement. Contrôle ciblé PASS : fichiers successifs, fichier vide, nom sur 16 caractères, fichier de 65535 octets avec adresses traversant 0xffff, flags et checksums reconstruits, aller-retour exact TAP → segments → fichiers → TAP, décodage série et octets résiduels, refus des dommages/segments inconnus, bloc trop court, framing invalide et annulation conservant la destination. Aucun média utilisateur utilisé ; le banc et ses artefacts sont supprimés.

Diagnostics générés avec Argos dans les 29 langues, relus et corrigés, avec protection des noms Atom et Atom TAP. Catalogues compilés. Audit des constantes et des traductions réussi.

L’adaptateur Atom vers le registre des décodeurs, l’encodeur séquentiel Atom et son signal PCM restent à porter ; ces fichiers ne sont pas marqués portés dans l’inventaire. La composition globale et le raccordement à App restent également à terminer.
