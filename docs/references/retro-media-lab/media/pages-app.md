# Raccordement des pages App

Référence C# consultée en lecture seule : `F:/GW GUI/src/RetroMediaLab.App/Theme/Base/Views/Controls/Explorer/Sections/ExplorerSection.Navigation.cs` et `ExplorerSection.Media.cs`.

L'explorateur présente les dossiers avant les fichiers, navigue par arbre et double-clic, affiche le détail de la sélection, distingue les volumes et les sessions optiques. Les catalogues `Resources/<langue>/Explorer.resx` contiennent déjà les traductions à reprendre. Les états et résultats doivent survivre à la destruction d'une présentation de thème.

Contrats C++ disponibles : `MediaEngine/MediaExploration.h` (`analyzeMediaOpeningAsync`, résultat disque ou média), `ExplorationInfo.h` (volumes et entrées possédées), `DiskExploration.h` (sélection d'interprétation). App n'utilise aucune API MediaFileSystems ou MediaAnalysis.

État initial vérifié : `WorkspaceTabs.cpp` crée sept QWidget vides ; les cinq pages demandées restent donc à implémenter. Ce document ne constitue pas une validation de leur fonctionnement.

## Premier raccordement de l’explorateur

Session conservée dans WorkspaceState ; ouverture asynchrone, annulation et attente de fin à la destruction dans Infrastructure ; navigation et présentation dans la vue du thème. Les volumes et sessions optiques, les dossiers, les fichiers, les dates locales et les types classifiés sont raccordés au résultat public. Les 30 catalogues Explorer (Constants + 29 langues) ont été repris des RESX C# sans nouvelle traduction automatique. TranslationService peut désormais résoudre explicitement le domaine des libellés média.

Validation : compilation Release de la session, de la vue et des 30 catalogues réussie ; contrôle des littéraux réussi (1613 sources) ; frontières de bibliothèques contrôlées sans violation. Aucun codec inchangé n’a été retesté. La vérification à l’écran et le raccordement à la lecture physique restent ouverts, tout comme les quatre autres pages : ce raccordement ne clôture pas l’objectif.

## Visualisation : premier raccordement et essai réel

Références C# supplémentaires : `ViewModels/Visualization/TrackSegment.cs`, `Presenters/Visualization/SectorMediaInspectorPresenter.cs`, `ScpInspectorPresenter.cs`, `Theme/Base/Views/Controls/Visualization/` et `Resources/<langue>/Visualizer.resx`. Le présentateur C# distingue notamment données présentes, absence, défaut d’intégrité et secteur dégradé : la distinction dégradée complète reste à raccorder.

La session Infrastructure prépare les descripteurs sur un worker, conserve le document et les sélections hors du thème, annule les opérations devenues obsolètes et attend leur terminaison à la destruction. La vue dessine les faces/pistes/secteurs ; les flux utilisent des moyennes angulaires calculées à partir des intervalles réels et le choix de révolution. Le signal audio utilise les minima/maxima des échantillons par colonne de pixels. Les autres représentations disposent d’une vue des éléments décrits par MediaEngine et d’une inspection à la demande ; leurs inspections spécialisées restent à compléter.

Compilation Release du bloc et des 30 catalogues réussie. Audit des littéraux : 1620 sources, PASS. Déploiement local par cmake --install effectué.

Essai Windows avec la version installée : ouverture de `F:/Retro/AMX Art 2.1.ssd` en lecture seule ; face circulaire et table des secteurs affichées ; bouton + passant de 100 % à 120 % avec agrandissement réel ; clic sur la piste 24 modifiant la sélection et les octets affichés. Passage à l’explorateur : même image ouverte, système Acorn DFS, capacité 100 Kio, 11 entrées, types de fichiers et libellés français affichés. Fenêtre de vérification fermée normalement et absence de fenêtre RML constatée ensuite.

Cet essai ne valide ni les trois pages encore vides (Lecture, Écriture, Conversion), ni l’ensemble des représentations, ni les changements de langue/thème en cours de session. L’objectif global demeure ouvert.

## Lecture et Écriture : raccordement des contrôles et commandes

Références C# consultées : `Infrastructure/Read/ReadCommandBuilder.cs`, `Write/WriteCommandBuilder.cs`, `Commands/Options/GwOptionValidator.cs`, `Commands/CommandLineTokenizer.cs`, `Naming/SequenceFormatter.cs`, `App/ViewModels/Operations/{Read,Write}OperationViewModel.cs`, `Controllers/MainWindow/Read/ReadTabController.ProfilesAndSettings.cs`, `ReadTabController.Execution.cs`, `WriteTabController.cs` et les RESX correspondants.

Infrastructure contient les brouillons, les options, la validation des spécifications, la numérotation et l’assemblage des commandes. Le lanceur Greaseweazle existant est réutilisé pour `read`, `write` et les futures conversions, sans introduire d’entrées dans le dialogue de maintenance. Ses journaux, ses limitations de sortie et son groupe de processus restent communs. Les validateurs de pistes et PLL ont été mutualisés avec la maintenance. Le choix de moteur est strict : un fournisseur interne/KryoFlux indisponible provoque un message, sans remplacement silencieux.

Les vues présentent lecteur, profil de session, dossier/nom ou source, format/extension, options avancées et arguments experts. La lecture traite les collisions de noms (écraser, numéro suivant ou modifier) ; l’écriture demande confirmation et bloque une interprétation absente/ambiguë. Les fichiers incomplets sont supprimés sur annulation d’une capture et l’arrêt lors de la destruction attend la fin des processus. Les callbacks conservés avec les sessions sont définis dans App.dll et non dans le thème. Le terminal existant reçoit les commandes et sorties par notification ; la barre d’état suit l’opération active.

Compilation Release réussie, sans avertissement C++ relevé ; catalogues Constants + 29 langues issus du C#. Deux corrections de compilation ont été nécessaires pendant le bloc : inclusion explicite de QWidget et séparation d’une déclaration auto mélangeant pointeurs et caractère. Audit des littéraux : 1630 sources, PASS ; audit des frontières : 1618 sources, 0 violation.

Essai de l’application installée : nom de capture saisi, console ouverte et activation de l’option Révolutions ; commande observée `read --device COM3 --drive A --revs 5 G:/Retro/RML-capture-preview.scp`. Choix de l’image Acorn SSD depuis la page Écriture ; assemblage `write --device COM3 --drive A "F:/Retro/AMX Art 2.1.ssd"` observé avant l’ajout du blocage explicite des formats ambigus du C#. Aucun bouton d’exécution matérielle n’a été actionné. Les cadres source pleine largeur et lecture brute sans sélecteur vide ont été revus ensuite. La fenêtre de vérification a été fermée normalement.

Les profils sont actuellement conservés pendant la session, mais leur persistance reste à raccorder. La progression détaillée et les résumés de capture, la validation des exécutions/arrêts et la page Conversion demeurent ouverts : ces vues ne clôturent pas l’objectif global.


### Conversion — session et premier lot GUI (2026-11-17)

Références C# : `Controllers/MainWindow/ConversionTabController.cs`, `Services/Conversion/ConversionBatchExecutor.cs`, `Infrastructure/Conversion/ConversionCommandBuilder.cs`. La session Infrastructure utilise les destinations directes et séquentielles du moteur, puis ConversionPlanner ; le filtre du catalogue runtime reprend la compatibilité avec la source chargée, comme le C#. Le moteur sélectionné est respecté, sans repli silencieux vers Host Tools.

La vue propose formats/extensions par famille, sorties et fidélité, tags, options avancées et exécution par lots. Les conflits proposent écrasement, saut ou numérotation pour le lot. Les pertes séquentielles demandent une confirmation avant écriture. Le service possède les futures, leur annulation et leur attente à la destruction ; les callbacks persistants et le rendu des journaux restent dans App.dll. Les journaux internes utilisent ConsoleLogSession, les commandes externes le lanceur GreaseweazleTools existant. Les 30 catalogues reprennent les RESX C# existants, sans traduction automatique.

Vérification effective dans le GUI Release : chargement d’une copie temporaire du SSD Acorn DFS `AMX Art 2.1.ssd`, sélection des deux destinations `cpcemu.dsk` et `raw.scp`, exécution du lot par le moteur interne. Deux fichiers produits : DSK 112896 octets et SCP 2966712 octets ; source de travail 102400 octets. Le dernier contrôle a identifié puis corrigé l’ordre des notifications de progression pour garder le bilan de lot en dernier. Fenêtre fermée normalement ; fixtures et sorties temporaires supprimées. Aucun accès en écriture à un lecteur physique.

Restent explicitement ouverts : profils RAW d’entrée/sortie, choix de conflit par fichier (actuellement décision globale au lot), profils persistants de conversion et validation des conversions séquentielles avec pertes. Ce résultat ne clôture pas les cinq pages du point principal.


### Profils des opérations — persistance commune (2026-11-17)

Référence C# : OperationProfileCollection, InMemoryProfileStore, ProfileSettings, ProfileOptionsController et les méthodes Capture/Apply des ViewModels des trois opérations. Infrastructure possède maintenant la collection commune OperationProfiles. Elle conserve un identifiant stable lors du remplacement et du renommage ; chaque opération garde son défaut permanent, non sérialisé. Les doublons sont rejetés par opération. Un chargement JSON invalide laisse intacte la collection précédente.

Les valeurs reprennent les conventions C# : options sans préfixe --, expert, result (raw/known), format, extension et folder pour Lecture ; tags, format:<id> et extensions:<id> pour Conversion. Les fichiers sources ne sont pas enregistrés dans les profils. La destination de Lecture est conservée comme en C#. Les sélections de Conversion restent conservées au chargement d’une nouvelle source si elles sont compatibles.

PreferencesStore ajoute la section profiles au fichier utilisateur existant ; son absence reste compatible. Bootstrap charge la collection avant les vues et souscrit à ses changements pour utiliser QSaveFile. Sessions et vues partagent la collection, indépendante du thème. Préférences permet renommage/suppression avec confirmation, menu contextuel et F2/Suppr ; les trois pages possèdent sélection, enregistrement/remplacement et réinitialisation.

Validation : compilation Release sans avertissement ; contrôle temporaire réussi sur sauvegarde/rechargement, valeurs/options, identifiants, séparation des opérations, doublons, protection des défauts et chargement invalide atomique. Source, exécutable, objet, journal et JSON temporaire supprimés ensuite. Contrôle visuel réel du sélecteur Conversion et des trois listes de Préférences : défaut traduit, renommage/suppression désactivés sur les défauts ; fenêtres fermées normalement. Aucun profil de test ajouté aux préférences réelles.


### Conversion — PCM explicite et conflits par fichier (2026-11-17)

Références C# : ConversionTabController.BrowseSourceAsync/ExecuteAsync, TapePcmProfile, EmulationTapePcmSettings, ConversionConflictWindow et ConversionConflictResolutionFunctions. Le GUI demande le profil PCM pour un RAW non reconnu, puis un profil de sortie si tape.raw est sélectionné. Fréquence, canaux, précision, signe et ordre des octets sont transmis aux lecteurs/convertisseurs existants. Le nouveau contrat public MediaReading laisse MediaDocument opaque et réutilise la validation TapePcm interne ; aucun décodeur ni modèle de média parallèle. Les paramètres source sont également transmis avec le préfixe source. aux conversions séquentielles, comme en C#.

Le dialogue de conflits reprend un choix par fichier et les trois actions globales : écraser, ignorer, numéroter. Infrastructure valide que chaque décision concerne un conflit du lot, réserve tous les noms prévus avant numérotation et refuse une sortie qui écraserait le fichier source. Une sortie numérotée reste autorisée. La commande présentée et journalisée utilise les sorties effectivement résolues. Le refus de pertes séquentielles libère à nouveau l’état occupé de la session.

Les textes sont repris des RESX C# ; little-endian reste un invariant commun dans Constants et est absent des catalogues de langues. Compilation Release sans avertissement ; contrôles de constantes et de frontières entre bibliothèques réussis.

Vérifications ciblées effectives : profil explicite à 22050 Hz ; aller-retour RAW → WAV → RAW conservant exactement les 14 octets d’échantillons du contrôle ; décision mixte numéroter/ignorer/écraser avec nom futur déjà réservé et sources inchangées ; rejet d’une décision absente et de l’écrasement du fichier source. Depuis le GUI : dialogue PCM d’entrée accepté à 22050 Hz, dialogue PCM de sortie, tableau avec deux conflits et action Tout numéroter, puis bilan visible « 2 réussies, 0 échec ». Sorties numérotées produites : RAW 30 octets et WAV 58 octets (profil de sortie RAW à 48000 Hz). Application fermée normalement ; sources, sorties et programme temporaires entièrement supprimés.
