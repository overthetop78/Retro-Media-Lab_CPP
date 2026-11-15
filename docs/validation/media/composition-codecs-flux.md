# Validation de la composition des codecs — 15 novembre 2026

Les deux catalogues C# déclaratifs, les deux classes de constantes et FluxDecoderRegistry ont été relevés en lecture seule. Leurs cinq SHA-256 restent identiques à l’inventaire ; empreinte du manifeste : `73907bdae18c64bc0413b209f63245c414be1a75453c57d430be857ae8af9206`.

Les attentes d’ordre, d’identifiants, de noms et de participation automatique sont extraites des déclarations réelles C# : 25 décodeurs et 24 encodeurs, Lisa seule exclue de l’automatique. Le banc compare ces attentes aux objets C++ effectivement créés. Il ne recompile pas les algorithmes C# dans ce bloc ; leur validation individuelle est documentée dans les blocs de codecs.

Les contrôles vérifient chaque identité, nom, position et unicité, l’indépendance des instances entre deux créations de catalogue, le raccordement du constructeur par défaut, puis les décodages explicites et la réutilisation du même résultat en cache pour les 25 décodeurs sur une révolution vide.

Un registre contenant seulement Lisa confirme son exclusion automatique, le diagnostic RegistryNoAutomatic et la disponibilité de son décodage explicite. Le registre complet conserve un résultat automatique accessible dans son cache et n’utilise pas Lisa.

Six pistes sont encodées par les encodeurs du catalogue puis décodées via le registre par défaut : TYCOM, Membrain, QD MO5, AED 6200P, Centurion et Arburg. Sont comparés adresse logique, numéro, taille, intégrité, charge utile et complément éventuel. La sélection automatique est exécutée sur ces révolutions et son résultat doit correspondre au résultat en cache du décodeur sélectionné, sans sélectionner Lisa. Ce contrôle d’intégration ne constitue pas une nouvelle comparaison exhaustive de l’identification automatique de toutes les familles.

- Construction complète et déploiement Debug/Release réussis.
- Chaque configuration : **370 contrôles, 0 échec, 25 décodeurs, 24 encodeurs**.
- Audits des littéraux et des frontières réussis : **538 fichiers C++ / 526 fichiers contrôlés pour les frontières**, 29 langues.
- Le banc build/verification-flux-composition et ses journaux sont supprimés après validation ; aucun test dans le workflow GitHub.

Défaut de code corrigé : l’absence de spécialisation de participatesInAutomaticDetection pour Lisa dans la classe AppleIwm::Decoder. Le cache et les algorithmes des codecs ne sont pas modifiés.
