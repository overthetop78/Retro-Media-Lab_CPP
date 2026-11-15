# Validation ISO et sélection des writers

Compilation Debug de MediaEngine et dépendances réussie sans avertissement (`build/iso-writers-build.log`, `build/iso-writers-final-build.log`). Les trente DLL de catalogues sont compilées (`build/iso-writers-catalogs.log`) ; solution Visual C++ régénérée.

Un seul banc synthétique en mémoire, sans média externe ni ancienne suite rejouée, construit 64 secteurs ISO et un fichier HELLO.TXT contenant DATA. Il vérifie la sonde ISO, le document, la casse de sourceExtension, le parcours du registre MFS par défaut, le nom du volume PORTAGE, le nom/version du fichier extrait et ses octets, puis l’écriture GCM identique aux octets initiaux.

Le même banc vérifie les candidats déclarés indépendamment du document, l’exclusion d’une représentation incompatible avant CanWrite, l’ordre de résolution et l’arrêt au premier writer compatible, les doublons, la normalisation sans trim et la comparaison ordinale d’un identifiant non ASCII. Le writer refuse la perte d’informations de couche et de prégap stocké. Le lecteur signale un changement de longueur entre reconnaissance et lecture.

```
PASS: ISO recognition/document/filesystem extraction/write, preserved source metadata, ordered writer resolution, representation filtering, layer rejection and changed source length
```

La relance ciblée accompagne uniquement l’ajout du refus de prégap stocké. Le banc et son image temporaire sont supprimés après consignation du résultat dans `build/iso-writers-smoke.log`.

Traductions Argos générées pour 29 langues, lecture rapide effectuée, corrections des contresens observés sur writer, ISO, octets et piste unique. L’invariant ISO est ajouté au fichier commun ; le contrôle a détecté et permis de corriger son ancienne traduction arabe dans GameCubeNotIso. Aucune certification linguistique professionnelle n’est revendiquée.

Audits : 841 fichiers C++ pour constantes/catalogues, 29 langues ; 829 fichiers pour les frontières entre composants, zéro violation constatée. Le registre ne déclare pas disponibles les writers ni les adaptateurs de formats non encore portés ; aucune composition globale incomplète n’est présentée comme complète.
