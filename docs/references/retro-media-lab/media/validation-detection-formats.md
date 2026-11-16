# Détection des formats

Référence C# en lecture seule : les onze contrats/classes de `Images/Formats/Detection/`, `Interfaces/Formats/Detection/IImageFormatDetectionRule.cs` et les ressources `RetroMediaLab.App/Resources/<langue>/Detection.resx` pour les explications effectivement utilisées.

`ImageFormatDetector` possède son catalogue. Son résultat possède le format choisi et les candidats ; sa durée de vie ne dépend pas du détecteur. L'ordre reste SCP brut, ADF/Acorn, Atari, Apple, Macintosh, IBM, puis repli selon les candidats d'extension. La liste de candidats est fondée sur les extensions de sortie déclarées du catalogue, indépendamment du résultat d'une règle.

La taille connue reste une indication explicite. Le détecteur ne consulte pas la taille du fichier pour la remplacer lorsqu'elle est absente. Les règles de taille et d'ordre peuvent donc aboutir sans ouverture du fichier. Si une règle connaît un identifiant absent du catalogue, sa confiance reste celle de la règle, mais l'absence du format impose le choix utilisateur, comme en C#.

Les sondages portent sur trois octets DOS Plus, quatre ATX, dix MSA et deux octets Macintosh à l'offset 1 024. La règle MSA conserve ses bornes et ne décode pas les pistes. Le chemin ADFS partiel réutilise `Adf::describePartial` et le lecteur partiel déjà porté, après lecture du préfixe puis de la longueur connue nécessaire à la validation. Les sources sont possédées par des pointeurs RAII locaux. Les erreurs d'accès/IO entraînent un repli de sonde ; les autres erreurs et l'annulation ne sont pas absorbées.

Les douze explications conservent leurs clés et sont exposées avec le domaine de traduction MediaEngine. Les catalogues sont copiés des ressources C# existantes pour les 29 langues ; en-US reprend la base anglaise. Une formulation française incorrecte de `Detection.IbmAmbiguous` a été corrigée dans le port C++ uniquement. Aucun modèle de traduction n'a été relancé pour des textes déjà traduits.

Validation : compilation Debug et des catalogues, audit de 1 567 sources C++, contrôle ciblé de priorité DOS Plus/Mac/IBM, profils de taille, candidats et ambiguïté, en-têtes minimaux et tronqués, limites MSA, accès refusé simulé, erreur non IO et annulation propagées, durée de vie des sources et résultats, détecteur public. Tous ces contrôles passent. Le banc temporaire ne créait aucun média et a été supprimé. Les codecs et lecteurs antérieurs n'ont pas été retestés.
