# Référence C# des journaux

Source locale examinée en lecture seule le 14 novembre 2026 : F:/GW GUI/src, révision a1e92c1ca4ccdfb6d73588c9db4989606e9f06be. Pour chaque fonctionnalité portée, examiner son implémentation C# avant de définir les tâches C++ ; le C# est la référence du portage depuis le début.

Fichiers utiles :

- RetroMediaLab.App/Dictionaries/Options/LogActionDefinitions.cs : read, write, convert, erase, clean, info, bandwidth, rpm, seek, pin, reset, delays, update, align, dans cet ordre, avec leurs clés de traduction.
- RetroMediaLab.Infrastructure/Settings/Logging/LoggingSettings.cs : activation par défaut, maximum 1024 KiB, archives désactivées ; réglages par action et repli sur les valeurs globales.
- RetroMediaLab.App/Theme/Base/Views/Controls/Options/OptionsLogsSection.xaml et Options/Controllers/LoggingOptionsController.cs : 14 cartes, sauvegarde des cases et de la taille à la perte de focus ; texte invalide ou négatif restauré ; zéro sans limite ; création et ouverture du dossier.
- RetroMediaLab.Infrastructure/Processes/ConsoleLogSession.cs : nom d'action nettoyé en minuscules, fichier action.log ; début avec séparateur, date ISO et commande ; texte UTF-8 ; contrôle des réglages à chaque écriture. Dépassement avec archives : renommage daté et suffixe en cas de collision. Sans archives : conservation des lignes récentes tenant dans la limite. Une entrée individuelle trop grande peut être omise lors de la réduction ; en cas de nouveau fichier/archivage le C# peut écrire une entrée dépassant à elle seule la limite. Reprendre ces détails sans inventer une troncature par octets.
- RetroMediaLab.App/Services/Logging/ErrorLog.cs : errors-YYYYMMDD.log, warnings-YYYYMMDD.log et information-YYYYMMDD.log, indépendants des réglages par action ; contexte, application, environnement et détail. Les erreurs d'accès aux journaux n'empêchent pas le fonctionnement de l'application.
- RetroMediaLab.App/Services/Storage/StoragePaths.cs et Constants/Storage/ApplicationBrandMigrationConstants.cs : %APPDATA%/Retro Media Lab/Logs ; portable.flag à côté de l'exécutable donne Data/Logs.

RotatingOperationLogWriter.cs existe aussi dans Infrastructure mais aucun appel n'a été trouvé dans App. Il ne remplace pas ConsoleLogSession pour les réglages montrés par l'utilisateur.

La page est portée avec son service d'écriture. Les opérations médias non encore portées n'émettent pas de faux journaux ; elles utiliseront ce service lors de leur portage. Les exceptions actuellement interceptées par l'application peuvent déjà alimenter le journal d'erreurs.

Références techniques utiles :

- Qt QDesktopServices::openUrl : https://doc.qt.io/qt-6/qdesktopservices.html#openUrl — ouverture du dossier ; le retour positif indique la transmission de la demande au système.
- Qt QStandardPaths : https://doc.qt.io/qt-6/qstandardpaths.html — emplacement utilisateur des adaptations Unix futures.
- Windows SHGetKnownFolderPath : https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath — Roaming AppData et libération de la mémoire retournée par CoTaskMemFree.
