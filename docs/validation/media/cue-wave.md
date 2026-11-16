# Validation intermédiaire CUE/WAVE

Le 16 novembre 2026, compilation Debug MediaEngine réussie sans avertissement observé après correction d’une collision de résolution entre l’aide de guillemets et std::quoted (`build/cue-wave-build.log`). Le contrôle natif temporaire réussit : `PASS CUE syntax/immutable-structure/5-modes/metadata/loops/quotes/time/serialization/WAVE-PCM/chunks/bounds/rejections/cancellation` (`build/bin-cue-smoke.log`).

Les fixtures couvrent commandes, catalogue, métadonnées de feuille/piste, boucle transmise de FILE à TRACK, remplacement LOOP/NOLOOP, trois types de fichier, forçage Vorbis par .ogg, cinq modes, guillemets doublés, index multiples et temps CD. La sérialisation est canonique sur les cas validés et peut être relue. Les commandes mal placées, valeurs et temps invalides, doublons, pistes sans index, index négatifs en sortie, métadonnées inconnues et UTF-8 invalide sont refusés.

Les chunks RIFF/WAVE synthétiques vérifient padding impair, fmt PCM, source de données, lecture traversant une frontière de secteur, bornes, échantillons et alignement CD optionnel. Les mauvais profils, chunks hors fichier, padding absent et données incomplètes sont refusés. L’annulation est vérifiée. Le descripteur temporaire est également lu depuis un fichier réel.

Les sept erreurs sont générées avec Argos dans les 29 langues puis relues et corrigées ; les 30 DLL de catalogues compilent (`build/cue-wave-catalogs.log`). Audit réussi sur 1145 fichiers (`build/cue-wave-audit.log`). Le programme, le descripteur et les binaires temporaires sont supprimés après consignation.

Cette validation ne couvre pas encore le raccordement des pistes BIN/CUE, la publication de son jeu de fichiers ou Vorbis. La tâche 154 reste ouverte ; le contrôle final de ces parties devra être réalisé dans le même emplacement de vérification recréé pour ce travail.
