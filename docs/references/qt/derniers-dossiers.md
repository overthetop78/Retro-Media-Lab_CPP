# Derniers dossiers par plateforme

La section fileDialogDirectories de preferences.json conserve la destination de Lecture et le dernier dossier de chaque sélecteur. Le format et les clés sont communs à Windows, Linux et macOS, avec l’emplacement prévu par PreferencesStore.

La première ouverture d’un fichier de préférences sans cette section importe le groupe QSettings FileDialogDirectories. Cette ancienne source utilisait le registre utilisateur Windows, un fichier .conf sous Linux et les préférences .plist sous macOS. L’import garde les chemins qui pointent vers un support indisponible et conserve les valeurs déjà présentes dans le JSON. La présence de la section, même vide, termine la migration. Les anciennes valeurs Qt sont conservées.

Source officielle : https://doc.qt.io/qt-6/qsettings.html#locations-where-application-settings-are-stored

Contrôle en Release sous Windows avec des préférences isolées : import de dossiers existants, absents et vides, conservation du dossier par onglet après annulation, réouverture du JSON, priorité aux valeurs déjà migrées. Les fichiers temporaires ont été supprimés.
