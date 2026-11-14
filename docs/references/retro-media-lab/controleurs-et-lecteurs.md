# Contrôleurs et lecteurs

Demande du 14 novembre 2026 : l'onglet Contrôleurs et lecteurs doit contenir des sous-onglets pour les disquettes, les médias optiques et les bandes/cassettes audio. Conserver les périphériques configurés même débranchés. Le découpage en six catégories a été accepté : disquettes, médias optiques, bandes et cassettes audio, disques durs et supports amovibles, cartouches, bandes de données.

- Disquettes : catégorie indépendante de la marque du contrôleur ; Greaseweazle, puis autres contrôleurs selon les développements autorisés. Reprendre le comportement C# existant pour scanner, ajouter, sauvegarder et oublier contrôleurs/lecteurs, et gérer le chemin de gw.exe.
- Médias optiques : lecteurs sélectionnés par l'utilisateur et capacités d'extraction par média/fonction. Ne pas confondre identification du lecteur et compatibilité effectivement vérifiée ; état inconnu si aucune vérification n'a été réalisée.
- Bandes et cassettes : acquisition audio et sélection du périphérique/entrée, canaux et paramètres d'acquisition. Les lecteurs de bandes de données numériques ont leur propre sous-onglet.

Code C# consulté en lecture seule :

- RetroMediaLab.App/Options/Controllers/HardwareOptionsController.cs : détection des contrôleurs, ajout et sauvegarde des lecteurs, confirmation d'oubli, erreurs et remise en état après exception.
- RetroMediaLab.App/Theme/Base/Views/Controls/Options/OptionsHardwareSection.xaml : liste et paramètres disquettes, commandes de gw.exe.
- RetroMediaLab.App/Contracts/Storage/CompactDiscDriveSettings.cs : contrat modèle/vitesse, insuffisant à lui seul pour prouver des capacités d'extraction.

Nom probable recherché par l'utilisateur : KryoFlux. Le constructeur décrit un contrôleur USB de disquettes pour acquisition bas niveau du flux : https://kryoflux.com/ et https://www.kryoflux.com/?page=kf_features . Le HxC Floppy Emulator est un dispositif d'émulation de lecteur ; ce nom ne désigne pas la même fonction : https://hxc2001.com/floppy_drive_emulator/ . Consultation du 14 novembre 2026.

Les disques durs/supports amovibles, les cartouches et les bandes de données font partie du périmètre accepté.

## Références C# complémentaires

- Infrastructure/Hardware/GreaseweazleHardwareRegistry.cs, GreaseweazleDeviceMatcher.cs, Parsing/GwInfoParser.cs et Functions/Hardware/GreaseweazleHardwareScanFunctions.cs : filtrage USB, confirmation via `info --device`, identifiant série et compatibilité du cas où les informations matérielles accompagnent un avertissement réseau.
- App/Options/States/HardwareOptionsState.cs et Infrastructure/Settings/Hardware/HardwareSettings.cs : deux lecteurs par contrôleur, affectation A/B, valeurs taille/densité/vitesse, conservation des débranchés et sauvegarde explicite.
- App/Dictionaries/Options/HardwareChoices.cs : tailles 3, 3.5, 5.25, 8 ; densités inconnue/DD/HD/ED ; vitesses inconnue/300/360 RPM.
- Infrastructure/Hardware/Audio/AudioTapeOptions.cs : nombre et index explicites des canaux ; la sélection des paramètres dans les préférences C++ ne démarre pas d'enregistrement.
- Infrastructure/HostTools/GwInstallationManager.cs et App/Options/Controllers/HostToolsOptionsController.cs : référence du gestionnaire complet des outils C#. La reprise C++ comprend désormais sélection, recherche, vérification de version, téléchargement/installation et retour au chemin précédent ; le simple lien Versions de la première adaptation a été retiré.

## API Windows utilisées

Consultation du 14 novembre 2026, sources Microsoft :

- https://learn.microsoft.com/en-us/windows/win32/api/setupapi/nf-setupapi-setupdigetclassdevsw : ensembles de périphériques présents, à détruire avec SetupDiDestroyDeviceInfoList.
- https://learn.microsoft.com/en-us/windows-hardware/drivers/install/basic-installation-operations : propriétés et clés du périphérique via SetupAPI ; fermeture des clés de registre.
- https://learn.microsoft.com/en-us/windows/win32/coreaudio/device-properties : énumération des entrées audio, identifiants persistants et nom convivial ; libération COM, PROPVARIANT et CoTaskMemFree.

Les scans utilisés ici portent sur les métadonnées des périphériques. Les tests d'extraction des secteurs/flux/sous-canaux ou d'acquisition audio restent associés aux services médias à porter ; une détection ne valide pas ces capacités.


## Archives multiformats

Miniz est remplacé par libarchive avec zlib, BZip2, liblzma, Zstandard et LZ4 intégrés localement et liés statiquement. Révisions et licences : [archives](../dependencies/archives.md). Aucun téléchargement de dépendance pendant le build. Le premier usage reste l'installation Greaseweazle reprise du C#.

API officielle de version : https://api.github.com/repos/keirf/greaseweazle/releases/latest . Sélection greaseweazle-*-win64.zip et vérification SHA-256 lorsque le digest est fourni.

## KryoFlux

Source constructeur consultée le 14 novembre 2026 : [manuel KryoFlux, révision 1.31, DTC 3.50](https://www.kryoflux.com/download/kryoflux_manual.pdf), Installation et paramètres DTC. Deux lecteurs physiques 0 et 1 (option -d), numéros conservés après suppression. Outil dtc.exe sous Windows, kf_dtc sous Linux, version macOS aussi distribuée. Le chemin DTC reste distinct du chemin Greaseweazle.

Le scan Windows recherche l'identité USB/nom/service KryoFlux du pilote. Il ne confond pas tous les Atmel/BOSSA avec un KryoFlux. Aucun lancement DTC ni calibrage -c2 pendant ce scan de métadonnées. Un contrôleur hors ligne permet de préparer ses deux lecteurs ; une détection unique peut lui associer l'identité réelle. Les acquisitions DTC ne sont pas encore portées.
