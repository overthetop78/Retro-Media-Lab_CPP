# Bloc de conversion C#

Source examinée en lecture seule le 14 novembre 2026 : `F:/GW GUI/src`.

- `RetroMediaLab.App/Options/Controllers/TagOptionsController.cs` : activation immédiate, sélection des modèles prédéfinis, aperçu pendant la saisie, sauvegarde du modèle personnalisé en fin de saisie, activation des récents et changement d'exemple.
- `RetroMediaLab.App/Dictionaries/Options/TagPresetDefinitions.cs` : cinq modèles, avec espace final conservé.
- `RetroMediaLab.App/Dictionaries/Options/TagVariableDefinitions.cs` : dix variables présentées dans l'aide.
- `RetroMediaLab.App/Functions/Options/Tags/RecentTagPatternFunctions.cs` : cinq récents au maximum, suppression des doublons sans tenir compte de la casse, modèle mémorisé après Trim ; les prédéfinis ne rejoignent pas l'historique.
- `RetroMediaLab.App/Functions/Options/Tags/TagPatternFormatter.cs` : quatre exemples fixes, date du 6 août 2026 à 14:35:42, remplacement sans tenir compte de la casse, ajout du nom lorsqu'il n'est pas dans le modèle puis de l'extension en minuscules. `{TAG}` reste accepté pour compatibilité, sans être proposé parmi les dix variables.
- `RetroMediaLab.Infrastructure/Settings/Operations/OperationUiSettings.cs` : AddTags désactivé par défaut, modèle `[{FAMILY}-{FORMAT}] ` et liste de récents initialement vide.
- `RetroMediaLab.App/Theme/Base/Views/Controls/Options/OptionsGeneralSection.xaml` : bloc précédemment situé dans Général ; le C++ le place dans un onglet Conversion distinct selon la demande de l'utilisateur.
- `RetroMediaLab.App/Resources/<langue>/Options.resx` : textes repris dans les 29 catalogues C++. Relecture et corrections ciblées des contresens famille/format/extension, dont les extensions téléphoniques traduites à tort en japonais et en polonais. Aucun catalogue préexistant n'a été remplacé.

Le formateur C++ ajouté ici est celui de l'aperçu des préférences. Le moteur de conversion métier reste à porter à partir de MediaEngine ; ce bloc ne crée pas de moteur parallèle dans App.
