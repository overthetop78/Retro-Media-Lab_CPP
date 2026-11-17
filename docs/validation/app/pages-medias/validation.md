# Vérification des pages médias dans le thème RetroMediaLab

Les cinq pages ont été parcourues vides puis avec une petite image SSD : Lecture, Écriture, Conversion, Visualisation et Explorateur. Le contrôle utilise la véritable DLL du thème et son coordinateur de reconstruction, sans accéder au matériel ni modifier les préférences de l’utilisateur.

- Écran virtuel 1920 × 1080 à 100 % : fenêtre normale de 1360 × 820.
- Écran virtuel 800 × 600 à 100 % : fenêtre normale de 752 × 552.
- Langues fr-FR, en-US et de-DE ; palettes claire/sombre ; densité 75/100 %.
- Conservation des sessions, brouillons, format explicite d’écriture, fichier sélectionné, secteur sélectionné, zoom et contenu de console après reconstruction du thème.
- Contrôles H/L de Densel et bouton de sélection des définitions personnalisées présents ; valeur L transmise au brouillon.
- Détails de l’Explorateur traduits : catégorie, contenu, aperçu, exécution et encodage ; suppression de NotApplicable.

Résultat : parcours réussi dans les deux résolutions. Les PNG de chaque sous-dossier montrent la surface cliente Qt dans le bureau virtuel ; ils ne sont pas des captures de décoration native DWM. Le programme temporaire et ses copies de médias sont supprimés après utilisation.

Le raccordement de la migration et ses contrôles complémentaires sont consignés dans migration/validation.md. La clôture globale est détaillée dans completion.md.
