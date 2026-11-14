# Aperçu isolé et confirmation

Référence officielle consultée le 14 novembre 2026 : [QWidget](https://doc.qt.io/qt-6/qwidget.html), méthodes grab/render et setStyle. Une vue peut produire une image hors écran. Le style transmis à setStyle reste possédé par l'appelant et doit survivre aux widgets qui l'utilisent ; les styles des enfants sont affectés explicitement.

La page conserve une apparence provisoire distincte de ThemeService.settings(), qui reste le choix actif. Choisir est le seul appel d'application. Le libellé de thème actif est résolu depuis le service actif, jamais depuis les boutons provisoires.

ThemeService.createPreview prépare une instance isolée : publication locale des données, sans palette ni police globale. RetroMediaLabConfiguration ne modifie pas QApplication et n'installe pas d'observateur global dans ce mode. Pour Système, l'aperçu utilise la dernière palette native connue du service : capturée avant les personnalisations et actualisée lors de l'application réelle du mode Système et de ses notifications. Il ne réinitialise jamais temporairement la palette de l'application pour prendre une capture. L'application confirmée de Système relit la palette native courante.

ThemePreview rend uniquement un corps RetroMediaLabLayout détaché, avec une copie indépendante des données de session et sans copier les abonnements de sauvegarde. Chaque rôle de la palette locale est explicite, afin qu'une palette native sans masque ne réhérite pas les couleurs du thème actif. La vue, le style et le service sont détruits après la capture, dans cet ordre ; seule l'image est conservée. Le rappel différé est lié à la durée de vie du widget d'aperçu.

Les abonnements de traduction expirés sont retirés lors de nouvelles souscriptions, afin de ne pas accumuler les observateurs des vues temporaires. Les publications de thème identiques ne renotifieront pas les abonnés, ce qui évite une nouvelle sauvegarde après la notification système déjà traitée.
