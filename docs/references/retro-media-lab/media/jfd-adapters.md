# Création et adaptateurs JFD

Références C# consultées en lecture seule dans `F:/GW GUI/src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/Jfd/` :

- `JfdCodec.cs`, SHA256 `2b574ceda7ac5ec9ce631cf465c9f7c013b20458004d47cd82122a69a408098c`.
- `JfdReader.cs`, SHA256 `009030d5bf5da77125cc6b5fbb178b243a28a745ec6f6ec98f27efa961c267cb`.
- `JfdWriter.cs`, SHA256 `7275628bef7415303d69558e5c46b379f41c16cbe6ce4aa6ea98076b2a3a87c9`.

## Création

`Create.cpp` utilise les politiques sectorielles communes déjà portées. Seuls les secteurs physiques complets et uniques, avec codage FM/MFM et tailles natives représentables, sont acceptés. Les données supprimées et les adresses enregistrées différentes sont refusées, comme dans le C#. Les tailles variables sont autorisées lorsque chaque taille est représentable. Le classement dans chaque piste est stable, suivant l'ordre enregistré ou, à défaut, le numéro de secteur.

La table créée contient au moins 160 pistes physiques, soit 80 cylindres à deux emplacements de tête, comme le C#. Un petit jeu de secteurs exporté ne conserve donc pas nécessairement le nombre initial de cylindres dans la géométrie déduite à la relecture. Les pistes absentes restent des entrées sentinelles ; les intégrités d'en-tête et de données restent indépendantes.

## Lecture

`Reader` expose `acorn.jfd`, `.jfd`, la signature JFDI et la représentation sectorielle. La reconnaissance décompresse le fichier si nécessaire et vérifie la signature ; la lecture complète valide les tables. Les erreurs de structure sont classées comme rejets par le registre, tandis que les erreurs d'accès et l'annulation continuent à remonter.

Le chargeur de fichier est injectable et utilise le chemin physique principal, comme le C#, sans substituer silencieusement les données mises en cache dans RecognitionContext. Le chargeur par défaut possède une source aléatoire RAII et borne sa lecture à la limite JFD. Une image devenue trop grande entre la création du contexte et le probe est rejetée sans allocation proportionnelle à sa nouvelle taille.

`ReadBytes` est disponible pour les appels possédant déjà les octets. Les métadonnées stockent l'ensemble de la structure décompressée en base64 et le mode gzip d'origine. Les deux informations sur les pistes brutes et les positions de secteurs sont des recettes de messages traduits du domaine MediaEngine, sans texte anglais affiché directement.

## Écriture

Le writer utilise l'adaptateur sectoriel commun, l'identifiant `jfd` et l'écriture atomique existante. Il n'accepte que les documents disquette sectoriels. Avec des métadonnées natives, le base64 est contrôlé strictement avant le décodage et l'édition native. Des métadonnées invalides ne déclenchent jamais une création de remplacement.

Sans métadonnées natives, il crée une nouvelle structure. La valeur exacte `False` de `jfd.gzip` produit une image non compressée ; toute autre valeur ou l'absence de clé utilise gzip. La compression s'effectue par morceaux dans le producteur atomique, avec zlib déjà embarquée, contrôle d'annulation et propriétaire RAII du compresseur. Une exception ne remplace pas le fichier de destination existant.

Les trois nouveaux diagnostics et les deux informations sont traduits dans les 29 langues. DiscOp, base64 et gzip sont ajoutés à la liste commune des invariants ; les traductions ont été relues et corrigées. Le codec JFD et ses deux adaptateurs sont portés ; la composition globale de l'ensemble des formats reste distincte et inachevée.
