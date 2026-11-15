# Documents et conteneurs Amstrad/FDS

Compilation Debug de MediaEngine et dépendances réussie ; compilation finale sans avertissement (`build/media-document-cartridge-final-build.log`). Les trente DLL de catalogues sont compilées (`build/media-document-cartridge-catalogs.log`). La solution Visual C++ est régénérée par le script existant.

Un seul petit banc temporaire, sans C# exécuté ni média externe, vérifie les raccordements nouveaux :

- CPR : sonde, lecture de la banque 2 sans banques précédentes inventées, exploration générique et nom virtuel sous forme de clé ; sortie atomique identique aux octets du conteneur synthétique.
- ROM : sélection explicite du format amstrad.rom, avertissement de banque courte et réécriture des seuls 8192 octets présents.
- FDS : sonde, document, exploration et extraction de DATA ; réécriture d’une face brute. Une variante avec en-tête, nombre de faces non déclaré et chemin source UTF-8 vérifie la conservation intégrale de l’en-tête source.
- Contrat writer : SupportsTarget reconnaît les capacités sans dépendre d’un document, avec comparaison insensible à la casse.
- Contexte : une future conserve sa source après destruction du handle appelant et lit à cheval sur deux pages.
- Mécanique : aucune terminaison d’accès lorsque le callback de position échoue avant acquisition ; terminaison unique lorsqu’un BeginRead échoue après début d’acquisition.

Résultat conservé dans `build/cartridge-documents-smoke.log` :

```
PASS: CPR sparse mapping/exploration/write, short ROM warning/write, FDS extraction/write, deferred context ownership, mechanical acquisition cleanup
```

Une première exécution du banc a refusé la sonde ROM car le banc utilisait l’identifiant de writer amstrad-rom au lieu du format amstrad.rom ; correction du banc, sans modification du lecteur. Aucune ancienne batterie comparative n’a été rejouée. Les relances ont accompagné les ajouts ciblés au contrat writer et à la conservation de l’en-tête FDS.

Audits actuels : 825 fichiers C++ pour constantes/catalogues, 29 langues ; 813 fichiers pour les frontières entre composants, zéro violation constatée. Ces contrôles statiques ne certifient pas la totalité de l’architecture ni l’ensemble des formats non portés.

Les deux nouveaux catalogues sont générés par Argos. Le français est relu et corrigé ; clés, paramètres et invariants des 29 langues sont validés. La relecture linguistique complète des autres langues reste explicitement ouverte.

Les empreintes des quinze sources C# associées sont vérifiées dans le document de référence et l’inventaire. La liaison mécanique secteur/optique du document, le service mécanique concret, les adaptateurs des autres formats et les façades globales restent à terminer.
