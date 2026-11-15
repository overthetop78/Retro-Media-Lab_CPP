# Retrait des comparaisons de masse abandonnées

Les suites natives préparées ne doivent pas être lancées : consigne utilisateur de poursuivre le portage sans nouvelles batteries ni répétition des contrôles déjà validés. Les compilations de production et les contrôles d'intégration bornés restent autorisés.

- [x] Retrait des fichiers temporaires
  - [x] Supprimer les fichiers et artefacts sous `build/verification-sector-registry/` : banc de comparaison préparé, sans exécution native de sa batterie.
  - [x] Supprimer les fichiers et artefacts sous `build/verification-imagedisk/` : banc de comparaison préparé, sans exécution native de sa batterie.
  - [x] Supprimer les fichiers et artefacts sous `build/verification-cpc-dsk/` : banc de comparaison préparé, sans exécution native de sa batterie.

Les empreintes des sources réelles sont déjà conservées dans l'inventaire du portage. La preuve des contrôles de contrats et d'exploration réellement effectués est conservée séparément dans `docs/validation/media/`.
