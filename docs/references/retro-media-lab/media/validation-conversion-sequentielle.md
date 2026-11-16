# Conversion séquentielle native

Références C# en lecture seule : SequentialMediaConversionPlan.cs, SequentialMediaConversionService.cs et TapeSignalRepresentationConverter.cs. Empreintes enregistrées dans l’inventaire.

Le service assemble les registres de lecture, les sept protocoles et les writers injectés. La préparation ne crée aucun fichier. Le choix automatique de machine utilise les blocs dont l’intégrité est validée ; une égalité exige une sélection explicite. Les segments non décodés sont proposés à l’encodeur avant un essai avec perte. Ces pertes sont des recettes de traduction, jamais du texte anglais imposé. L’écriture est refusée avant appel du writer si elles ne sont pas acceptées. Les paramètres source.* sont transmis à la reconnaissance sans leur préfixe ; RAW est demandé explicitement.

La conversion de signal UEF/Atom vers WAV/CSW/RAW et PCM vers UEF reste séparée de la reconnaissance des fichiers contenus dans la bande. Elle annonce les pertes de structure, de chronologie ou d’amplitude. Les sources des documents restent possédées par shared_ptr ; fichiers temporaires nettoyés par RAII, y compris en exception.

Vérification ciblée Debug réussie : enregistrement des protocoles, destinations TAP/WAV, taux par défaut 48000, plan sans écriture, lecture et écriture réelle TAP vers WAV, signal WAV vers UEF puis WAV, plan invalide, annulation, segment inconnu conservé dans la source et déclaré perdu dans la cible, refus sans création de fichier puis réussite après acceptation. Pas de reprise des batteries de codecs déjà validées.

Compilation Debug MediaEngine réussie, sans avertissement du compilateur. Audit des constantes et clés : 1343 fichiers C++, 56 clés App et 29 langues. Catalogues de conversion générés avec Argos puis relus, notamment correction du sens de writer et des formulations de chronologie et d’informations hors signal.

Le service natif est synchrone pour les appels d’orchestration ; les lecteurs et writers conservent leurs futures et l’annulation. Le branchement à la façade générale, à la progression applicative et au GUI reste à faire. La lecture audio compressée et l’assemblage général des formats restent également à terminer. Cette validation ne constitue pas une validation complète du portage.
