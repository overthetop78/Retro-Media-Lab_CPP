# Outils Greaseweazle : référence C#

Sources consultées en lecture seule le 14 novembre 2026 dans `F:/GW GUI/src` :

- `RetroMediaLab.App/Theme/Base/Views/Controls/Shell/MainMenu.xaml` : Diagnostics (`info`, `bandwidth`, `rpm`, `seek`, `align`) et Matériel (`pin`, `reset`, `delays`, `update`).
- `RetroMediaLab.App/Theme/Base/Views/Windows/Tools/GwToolWindow.xaml` et `.xaml.cs` : champs, valeurs initiales, sortie brute, commande, exécution/arrêt, confirmation du bootloader, annulation à la fermeture et résumé des informations locales.
- `RetroMediaLab.Infrastructure/Maintenance/ToolCommandBuilder.cs` : construction des arguments par opération, ajout de `--device`, et de `--drive` uniquement pour rpm/seek/align.
- `RetroMediaLab.Infrastructure/Commands/Options/GwOptionValidator.cs` : validation des spécifications de pistes, PLL, vitesse et incompatibilités fake-index/hard-sectors et densel/gen-tg43.
- `RetroMediaLab.Infrastructure/Processes/GreaseweazleRunner.cs` : processus sans shell, flux redirigés, annulation et délai de grâce de deux secondes avant destruction.
- `RetroMediaLab.Infrastructure/Hardware/Parsing/GwInfoParser.cs` : Model/Firmware/Port et distinction d’un avertissement réseau.
- `RetroMediaLab.App/Resources/<langue>/*.resx` : reprise des libellés existants dans les 29 langues.

## Adaptation demandée en C++

Une action **GreaseWeazle** remplace les deux anciens sous-menus après le séparateur du menu Options. Elle ouvre un dialogue à neuf onglets. L’utilisateur a explicitement demandé les contrôles et commandes, pas seulement une ossature.

| Onglet | Paramètres initiaux C# |
|---|---|
| Informations | Aucun paramètre spécifique |
| Bande passante USB | Aucun paramètre spécifique |
| Vitesse RPM | nr = 1 |
| Déplacer la tête | cylinder = 0 ; force et motor-on décochés |
| Alignement | tracks = c=40:h=0-1 ; revs = 3 ; reads = 10 ; format/diskdefs/fake-index/adjust-speed/pll/densel optionnels, tous désactivés ; raw/hard-sectors/gen-tg43/reverse décochés |
| Broches | pin = 26 ; lecture par défaut ; set/high décochés |
| Réinitialiser | Aucun paramètre spécifique |
| Temporisations | select 10, step 3000, settle 15, motor 750, watchdog 10000, pre-write 15, post-write 15, index-mask 15 ; tous optionnels et désactivés |
| Micrologiciel | bootloader décoché, confirmation obligatoire avant cette option |

Infrastructure conserve les brouillons et résultats dans un service de session. La reconstruction des vues lors d’un changement de thème ne doit donc ni perdre ces données ni interrompre une opération.
