# Captures 1920 × 1080

Captures conservées le 14 novembre 2026 à la demande de l’utilisateur.

- Rendu réel des widgets C++/Qt du thème RetroMediaLab, en mode sombre, dans un écran virtuel hors écran sous Windows.
- Résolution du profil : 1920 × 1080 pixels ; facteur système simulé : 100 % ; réglage Taille de l’interface : 100 %.
- Pour une diagonale de 15 pouces, la densité physique calculée serait environ 146.86 PPP. Cette densité est inscrite dans les métadonnées PNG. Elle ne remplace pas un essai sur un moniteur physique.
- Les cadres natifs Windows, le bureau et la barre des tâches ne sont pas simulés. Les vues des dialogues sont assemblées sur la capture du contenu de la fenêtre principale, à leurs positions réelles dans ce profil. Le fond bleu-gris représente uniquement l’espace de bureau virtuel. Aucun contrôle n’est redessiné artificiellement.
- Le lecteur COM_TEST et l’exécutable vide gw.exe sont des données de démonstration du banc. Aucune commande n’est lancée sur du matériel.
- Les captures sont à examiner à 100 % dans un visualiseur pour comparer les tailles en pixels ; un aperçu automatiquement réduit ne représente pas leur taille physique.

| Vue | Dimensions | SHA-256 |
|---|---|---|
| [Application](01-application.png) | 1920 × 1080 | `23c7c8d1647d20ddc0236e8c6b09c62c7e5860db1ef846dea13c401e2fbc3aeb` |
| [Préférences — Général](02-preferences-general.png) | 1920 × 1080 | `88b2f935af2ed86ad8e89bfa49da9cae8f87bdf761c9f93524e0d51fd06ad647` |
| [Préférences — Contrôleurs et lecteurs](03-preferences-controleurs.png) | 1920 × 1080 | `a65de953c35985ba407b96bb0474015f87ec59cbe49bc5da12e6bfd21accd901` |
| [Préférences — Profils](04-preferences-profils.png) | 1920 × 1080 | `2c1d268ad9f4510cc1ec2e02858e03766dbb9454ad08b619c5ab79c10e3af23e` |
| [GreaseWeazle — Informations](05-greaseweazle-informations.png) | 1920 × 1080 | `f4599e5dffb4f7519426cecddae2a7314209c44f01958afe5d0fc87baa0528b3` |
| [GreaseWeazle — Alignement](06-greaseweazle-alignement.png) | 1920 × 1080 | `3878f76707f77f5558ffef83e942a06ec4485299e619db8db6bf87a923316e57` |

## Aperçu GreaseWeazle

![GreaseWeazle — alignement](06-greaseweazle-alignement.png)

## Aperçu Préférences

![Préférences — général](02-preferences-general.png)

## Correction : fenêtres normales

Les captures précédentes avec la fenêtre principale maximisée ont été remplacées. Le banc ne demande désormais aucune maximisation, aucun déplacement et aucun redimensionnement. Les dimensions sont celles décidées par l’application à son ouverture : fenêtre principale 1360 × 820 pixels, dialogues Préférences et GreaseWeazle 1100 × 640 pixels. Sur le profil 800 × 600, le plafonnement à l’espace disponible provient du comportement existant de l’application.

La géométrie a été vérifiée avant et après chaque capture. Les mesures sont conservées dans [geometries.json](geometries.json).

## Contrôles compacts et indicateurs

Captures actualisées après correction des hauteurs et du cache minimum des sélecteurs. La ligne du lecteur calcule sa hauteur après adaptation, sans défilement superflu. Les paramètres GreaseWeazle se répartissent sur plusieurs colonnes selon la place. Aucun redimensionnement ni maximisation n’est demandé par le banc.

Les relevés des 21 pages parcourues figurent dans [controls.json](controls.json) : dimensions des champs et plages de défilement. Les longues listes restent défilables si leur contenu le nécessite.

Comparaison du soulignement des onglets : [50 %](07-onglets-50.png), [100 %](07-onglets-100.png), [200 %](07-onglets-200.png). La fenêtre conserve ses dimensions dans les trois captures.
