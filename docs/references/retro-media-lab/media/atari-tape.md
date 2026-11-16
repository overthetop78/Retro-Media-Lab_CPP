# Atari CAS et décodage FSK

Références C# consultées en lecture seule le 16 novembre 2026 : Constants/AtariCasConstants.cs ; Images/Formats/Tape/AtariCas/AtariCasFormat.cs, AtariCasReader.cs, AtariCasWriter.cs ; Contracts/FskSerialDecodeResult.cs ; Functions/FskSerialFunctions.cs et SequentialDecodeFunctions.cs ; Images/Reading/Decoding/Sequential/Atari/AtariTapeDecoder.cs. Leurs huit SHA-256 correspondent à l’inventaire. DiskSystemIds.Atari8Bit est vérifié dans Images/Reading/Recognition/DiskSystemIds.cs : atari-8bit. L’encodeur Atari est seulement examiné pour ses dépendances et reste à porter.

## Conteneur

Le reader conserve FUJI, baud, data, fsk et chunks inconnus via des ranges possédés. Il reprend noms ASCII (octets hors ASCII remplacés comme dans le C#), trim de fin nul/espace, noms internes distincts, types de chunks dans leur ordre, débits triés et compteurs. Les données de débit inattendues et chunks inconnus sont conservés avec avertissements traduisibles. Les seules lectures de payload nécessaires au parsing concernent noms et pulsations, pas toutes les données.

Les temps sont en ticks 100 ns. Les pauses avant data produisent des porteuses ; la durée des données vaut longueur×10/débit si le débit est positif. FSK conserve les unités de 0,1 ms, les polarités space/mark et le chunk brut en plus des pulsations. L’addition des temps est bornée pour éviter un débordement. Le writer trie de façon stable, n’écrit que les segments munis d’un chunkId et copie les sources via le helper atomique existant. Les segments de signal calculés ne dupliquent pas les données.

Améliorations de validation : image vide refusée sans FUJI ; chunks data/FSK vides et pulsations nulles refusés avant les validations du modèle ; writer refusant identifiants non ASCII, auxiliaires invalides et data/FSK structurellement vides/impairs au lieu de produire une source rejetée ensuite. Le writer ne convertit pas les chunks inconnus.

## FSK partagé

Le résultat possède octets, positions et compteurs. La classification utilise les passages par zéro sans dépendre de la phase, les demi-ondes admissibles 0,5–1,5, et le recalage d’horloge après au moins 64 demi-ondes de porteuse. Chaque trame UART est synchronisée séparément ; les bits start/data/stop et les positions sont repris. DecodeFrames traite aussi une suite de niveaux booléens stockée en octets 0/1.

La génération PCM conserve la grille arrondie au pair de chaque bit et les demi-ondes/alternances du C#. Les tailles, paramètres et calculs de positions sont bornés ; les boucles prennent en charge l’annulation. Le compteur valide du chemin PCM est calculé explicitement avant transfert des vecteurs : un premier contrôle avait trouvé les octets corrects avec une confiance nulle ; cette erreur du portage est corrigée et le contrôle passe.

## Protocole Atari

Le décodeur accepte CAS, WAV, CSW et RAW séquentiels pour la machine Atari 8 bits, comme le C#. Les conteneurs CSW et les autres formats de bandes restent indépendamment à porter. En CAS, les records standards 132 octets, records partiels, fins logiques et checksum SIO sont interprétés. Le padding étendu nul valide est consommé sans devenir un fichier. Les records invalides restent non décodés ; une somme de contrôle fausse invalide le bloc logique mais ne supprime pas ses données. Métadonnées de compteurs, positions sources, débits et fin de record sont conservées.

Les pulsations CAS peuvent produire des trames série FSK. Pour PCM, seul le premier canal explicitement numéro zéro est décodé, par segment, avec fréquence propre et précision native ; la confiance est trames valides/trames testées. Tous les segments non consommés sont conservés.

Aucun nom de fichier n’est inventé : les records CAS n’en portent pas dans ces métadonnées C#. Le passage à l’adaptateur MFS reste donc capable de signaler des blocs anonymes sans créer de fausses entrées. Sources, données et résultats locaux sont possédés/RAII, y compris en cas d’annulation. L’encodeur Atari, la composition automatique et l’intégration App ne sont pas déclarés terminés.
