# Reader Apple et routeur de conteneurs

Références locales C# consultées en lecture seule : `Images/Formats/Floppy/Apple/AppleDiskImageReader.cs`, `AppleContainerRouter.cs`, `AppleContainerExceptions.cs`, `Constants/AppleDiskImageFormatFamilies.cs` et les readers des conteneurs appelés.

Le reader commun annonce les 19 identifiants, dix extensions et trois signatures du C#. Les cinq préfixes Apple II, Apple III, Lisa, Macintosh et Mac sont aussi acceptés, sans distinction de casse.

Le routeur réutilise les backends natifs dans l’ordre source : signature 2IMG, mot privé DiskCopy, signatures WOZ1/WOZ2, extension NIB, puis lecture sectorielle brute. Une signature reconnue mais invalide ne provoque pas de repli silencieux vers un autre conteneur. La lecture du document conserve séparément la structure DiskCopy et ses métadonnées, comme le chemin spécifique du C#.

La sonde effectue une vraie validation du contenu, sans se contenter de l’extension. Elle rejette les diagnostics de données invalides ou de variante non prise en charge produits par ces backends. Les débordements, erreurs d’arguments, annulations et erreurs d’E/S ne sont pas avalés. La lecture du contexte précède le traitement des erreurs de validation. Les futures gardent le contexte, pas le pointeur du reader.

L’erreur de routage fournit extension et format demandé. Sa clé figure dans Constants et les 29 catalogues de langues, après génération Argos et relecture/corrections. Le helper C# `UnsupportedVariant`, non appelé par ce chemin, reste à intégrer avec ses futurs appelants ; l’inventaire le signale comme partiel.

WOZ/NIB restent ici des sources de secteurs reconstruits, conformément au reader Apple C#. Cela ne modifie pas leurs backends natifs de pistes. La composition globale des lecteurs et les autres contrats de MediaEngine restent en cours.
