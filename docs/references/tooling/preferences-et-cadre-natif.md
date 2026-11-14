# Préférences et cadre natif

Sources officielles consultées le 14 novembre 2026 :

- [QSaveFile](https://doc.qt.io/qt-6/qsavefile.html) écrit un fichier temporaire puis le remplace avec commit ; sa destruction abandonne une écriture non validée. Le mode de repli par écriture directe n'est pas activé.
- [QStandardPaths](https://doc.qt.io/qt-6/qstandardpaths.html) fournit AppConfigLocation pour les réglages propres à l'application et à l'utilisateur, selon la plateforme.
- [QDialog](https://doc.qt.io/qt-6/qdialog.html) permet des fenêtres de préférences avec propriétaire et modalité. Le dialogue du thème utilise show avec WindowModal ; aucune boucle exec imbriquée n'est introduite.
- [QFileDialog](https://doc.qt.io/qt-6/qfiledialog.html) fournit le choix de répertoire avec le mode Directory et l'option ShowDirsOnly. Parcourir emploie une instance possédée par la fenêtre de préférences, ouverte sans boucle exec imbriquée ; annuler conserve le chemin courant.
- [DWMWINDOWATTRIBUTE](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute) documente les attributs de couleur de titre et de texte comme attributs à définir avec DwmSetWindowAttribute, disponibles à partir de Windows 11 build 22000. Leur prise en charge ne se vérifie pas en les lisant avec DwmGetWindowAttribute.

L'adaptation Windows convertit les couleurs du thème en COLORREF et renvoie le résultat des deux écritures. Sur une plateforme ne les prenant pas en charge, l'habillage reste géré par le système. La fonction ne crée ni objet DWM ni référence de DLL supplémentaire. Les fenêtres restent responsables de leur destruction, même après initialisation partielle ou exception.
