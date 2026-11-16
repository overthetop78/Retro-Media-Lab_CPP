# Signaux de bande et sons mécaniques publics

Références C# : méthodes ReadTapeAsync/WriteTapeAsync et contrats de sons/mécaniques de IMediaRuntimePreparation et MediaRuntimePreparationService ; chemin de banque d’enregistrements et ProtectMix de MechanicalSoundFunctions. Les algorithmes de signal et de rendu déjà portés sont réutilisés.

## Contrats

TapeSignal.h est désormais public : la même déclaration de signal et de bloc est déplacée, et l’ancien en-tête interne devient un relais. Les échantillons PCM sont conservés dans leur stockage partagé immuable existant ; les blocs et métadonnées restent possédés. Les fonctions asynchrones publiques capturent leurs chemins, signal et paramètres par valeur. Elles délèguent aux fonctions TapeSignal existantes et transmettent l’annulation. La liste des extensions d’enregistrement est une copie des extensions du service existant.

MechanicalSoundService expose les profils, noms, rendu, durée et préchargement. Les catégories valides utilisent la banque enregistrée, comme le chemin déjà actif dans le C#. Un profil absent/inconnu se replie sur le profil de base ; le rendu conserve sa validation du périphérique, des tailles et du volume. Le dossier d’enregistrements et l’exécutable FFmpeg peuvent être fournis par le consommateur ; sans paramètres, le service et les mécaniques partagent la même banque et son cache, via une fonction unique. Le service ne possède ni périphérique audio, ni horloge, ni boucle de rendu. Le préchargement permet au consommateur de décoder les sons avant sa boucle d’émulation ; la vérification ne prétend pas mesurer un gain FPS.

La protection du mélange est extraite du rendu de mécanique et partagée avec l’API publique : pas d’amplification, atténuation seulement lorsque le pic dépasse le PCM, arrondi existant et saturation. Les valeurs non finies sont refusées par un diagnostic déjà traduit, ce qui évite une conversion flottant/entier indéfinie en C++.

MediaDriveService conserve Create(MediaKind) pour les disques reconnus, avec son comportement C# existant : aucune mécanique automatique pour Tape. La nouvelle création par nom technique permet explicitement Tape/Floppy/HardDisk/Optical et choisit le relais vers l’audio hôte. Sans relais, aucun callback hôte n’est enregistré. Avec relais, une référence faible vers la mécanique évite un cycle, et le destinataire peut conserver le shared_ptr reçu.

## Vérification ciblée

Compilation Debug réussie, sans avertissement signalé. Le banc n’inclut que les en-têtes publics de MediaEngine. Les signaux WAV et RAW sont relus à l’identique, avec profil RAW explicite. Une UEF produite par l’API est relue comme signal non vide. La copie d’un signal conserve ses échantillons après remplacement de la valeur source. Annulation de lecture/écriture et absence de fichier créé sont vérifiées.

Le mélange conserve les petites amplitudes, atténue un pic de 65534 à 32767 et refuse une valeur infinie. Une mécanique Tape sans audio hôte reste active sans notifier le callback ; avec audio hôte, le callback conserve la mécanique puis la libération complète fait expirer sa référence faible. L’absence de mécanique automatique pour MediaKind::Tape est aussi vérifiée.

La banque de production existante est consultée en lecture seule, sans recopier ses FLAC dans le banc. Le dossier est `src/MediaEngine/Assets/MechanicalAudio` ; une première invocation du banc avait indiqué le mauvais dossier racine puis a été corrigée. Le FFmpeg déjà intégré est fourni depuis son cache vérifié. Le préchargement du profil Tape, le rendu PCM possédé, le repli d’un profil inconnu, le volume nul, la durée et les paramètres invalides sont vérifiés. Les tests ne vérifient pas une sortie audio hôte : aucune n’est ouverte.

Audit des constantes : 1 435 fichiers C++, 29 langues, aucune valeur applicative hors constantes nommées. Aucun diagnostic supplémentaire n’est ajouté. Le banc et ses sorties/exécutables temporaires sont supprimés après utilisation ; la banque de production reste intacte.

## Suite

La conversion SCP Atari/Amiga reste à raccorder pour terminer les opérations de préparation C#. Les autres services de préparation sont répartis par responsabilité dans les contrats publics existants ; App reste un consommateur de ces services.
