# Composition générale de reconnaissance

Référence : MediaRecognitionComposition.cs, ConsoleCartridgeReader.cs et ConsoleCartridgeConstants.cs du dépôt C# en lecture seule. Les empreintes figurent dans l’inventaire.

Les 65 lecteurs sont assemblés dans l’ordre de la liste C#. Les classes C++ communes sont spécialisées avec leur variante explicite (Atari, Commodore, brut IBM/MSX, brut COHERENT/UCSD). Le lecteur SCP enregistré est la même instance que celle exposée à ses futurs consommateurs : son cache n’est pas dupliqué. L’union des extensions est normalisée et sans doublon.

Le lecteur console reprend 40 extensions pour 21 systèmes ; les identifiants ont été comparés aux constantes C# de MediaFileSystems. Les données ne sont ni transformées, ni permutées, ni débarrassées de leur en-tête. Chaque banque possède la source de lecture ; la dernière banque peut être partielle. La capacité logique arrondie n’invente pas des octets dans cette plage manquante. Les noms de modèles sont des invariants conservés dans le tableau des profils. Les échecs sont traduisibles, et le nombre de banques est borné avant allocation.

Écart corrigé pendant le raccordement : le lecteur audio RAW utilisait une comparaison sensible à la casse de l’identifiant explicite, contrairement au C#. supportsFormatId et la sonde acceptent maintenant TAPE.RAW comme tape.raw.

Contrôle ciblé Debug réussi : 65 lecteurs, présence du lecteur console à la place attendue, cache SCP partagé, extensions réunies, toutes les extensions console avec dernière banque de trois octets, accès aux octets sans copie, capacité et métadonnées ; reconnaissance automatique NES, WAV et UEF ; RAW explicite en deux casses ; incompatibilité extension/format ; limite simulée sans créer un fichier géant ni lire ses données ; annulation. Aucun ancien banc de codec n’a été relancé.

Compilation Debug sans avertissement. Diagnostics générés avec Argos, relus puis corrigés notamment pour distinguer banques mémoire et banques financières. Trente catalogues compilés. Audit : 1361 fichiers C++, 56 clés App, 29 langues.

Le service global de lecture et les mécaniques de lecteurs restent à raccorder ; la propriété ReadingService de la composition C# n’est donc pas déclarée portée. Les façades générales et l’intégration App restent également à faire. Ce contrôle ne conclut pas à la fin du portage global.
