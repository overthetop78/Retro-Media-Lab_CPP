# Relecture des diagnostics Amstrad et Famicom Disk System

17 novembre 2026. Les 22 messages traduisibles d’amstrad-fds.json ont été lus dans les 29 langues (638 valeurs). Deux invariants isolés restent dans Constants conformément à la règle en-US. 240 messages ont été corrigés dans 27 langues. Les traductions Argos existantes sont conservées lorsque leur sens convient ; les changements sont des corrections de relecture.

Corrections : signature RIFF AMS! déformée, longue répétition parasite japonaise, banques mémoire confondues avec des banques financières, données au-delà d’une page de 16 KiB, absence de banques cb00 à cb31, restitution d’une banque incomplète sans remplissage inventé, contiguïté et faces FDS. Les signatures, identifiants, unités et paramètres ont été vérifiés dans les 29 fichiers ; aucun échec. zh-Hant utilise les caractères traditionnels.

La liste commune d’invariants protège désormais explicitement RIFF AMS!, cb00, cb31 et 16 KiB pour les futures générations. Aucun reader, writer, algorithme natif ni média n’a été modifié ou retesté.

Les 27 DLL de langues modifiées ont été compilées ensemble en Release et installées dans lib/Languages. Journaux : build/media-amstrad-fds-catalogs-release.log et build/media-amstrad-fds-catalogs-install.log. Aucune bibliothèque média native recompilée.
