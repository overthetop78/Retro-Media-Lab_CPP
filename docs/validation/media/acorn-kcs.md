# Protocole de bande Acorn KCS

Références C# inchangées : `Images/Reading/Decoding/Sequential/Acorn/AcornTapeDecoder.cs` et `Images/Writing/Encoding/Sequential/Acorn/AcornTapeEncoder.cs` dans `F:/GW GUI`. SHA vérifiés dans l’inventaire ; constantes UEF et CRC16 déjà partagés réutilisés.

Les champs de blocs CFS, le scanner de synchronisation et les CRC d’en-tête/données sont regroupés dans un codec de blocs. Le décodeur expose les noms, adresses 32 bits, numéro, flags, adresse suivante, offset série et éventuel repère échantillon. Il conserve les blocs à CRC invalide comme tels, sans inventer une vérification de séquence globale absente du C#. La construction accepte les blocs d’au plus 256 octets, avec métadonnées et nombres vérifiés sans recopier le payload pour la sélection.

UEF implicite est parcouru par position ; les autres codages UEF demandant une synthèse suivent le chemin PCM. Le premier canal PCM est continu malgré les frontières de segments. Les deux débits 1200 et 300 sont décodés ; les CRC sélectionnent les blocs valides, triés par position échantillon et dédoublonnés suivant les mêmes champs que le C#. Si aucun CRC n’est valide, la liste la plus fournie reste présentée avec son intégrité. Les segments consommés/non consommés suivent le C#.

L’encodeur reconstruit UEF avec porteuses, données et gaps temporisés, ou le PCM FSK à la cadence choisie. La métadonnée reconstructedSignal indique la reconstruction. Les chunks UEF reçoivent aussi le contexte nécessaire à une synthèse directe en mémoire, comme dans le socle signal déjà porté. Les copies renumérotées partagent les octets immuables.

Deux incohérences de la source sont corrigées : les noms à octet nul sont refusés (ils rompent le framing malgré leur acceptation ASCII par le C#) ; les sorties PCM CSW/RAW directes exigent également un sampleRate, au lieu de pouvoir passer la sélection puis encoder à zéro hertz. La borne de signal est vérifiée avant allocation.

Compilation Debug réussie sans avertissement. Contrôle ciblé PASS : CRC d’en-tête et de données corrompus, tous les champs de bloc, bruit et framing tronqué, renumérotation partagée, registres UEF/PCM, écriture/relecture UEF, frontières PCM au milieu d’un caractère (canal zéro puis canal implicite), UEF cycles et bande mêlant des blocs à 1200 et 300 bauds dans leur ordre réel. Fichier vide, noms/tailles/profils invalides, borne et annulation vérifiés. Aucun média utilisateur ; banc et artefacts temporaires supprimés.

Les six diagnostics sont générés avec Argos, relus/corrigés dans les 29 langues et compilés dans les 30 DLL de catalogues. Audit PASS : 1280 fichiers C++, 56 clés d’erreur App et 29 langues.

Les registres du contrôle sont composés explicitement. La composition automatique générale et les services globaux de conversion/ouverture restent à porter ; le raccordement depuis App n’est pas encore validé.
