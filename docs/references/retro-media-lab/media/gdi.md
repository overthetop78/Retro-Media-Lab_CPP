# Dreamcast GDI

Référence C# en lecture seule : GdiConstants, GdiDescriptorReader, GdiReader et GdiWriter ; empreintes vérifiées dans l’inventaire. Le découpage sépare constantes et déclarations, descripteur/chemins, lecteur et writer. Les modèles de pistes optiques, sources de fichiers, calculs bornés et transaction de jeu de fichiers sont réutilisés.

Le descripteur contient un nombre de pistes et six champs par piste. Les lignes vides et commentaires sont ignorés ; la tokenisation respecte les guillemets et espaces Unicode. Les nombres utilisent le style None, sans signe ni caractères résiduels. Les numéros sont positifs et distincts, les LBA non négatifs et strictement croissants, les contrôles 0/4 et secteurs 2048/2352. La lecture de texte détecte les BOM UTF-8/UTF-16 comme le C#.

Les fichiers associés doivent exister et leur chemin absolu normalisé doit conserver le préfixe du dossier du descripteur, comparaison ordinale insensible à la casse. Ce confinement lexical reprend le C# ; il n’effectue pas de résolution supplémentaire des liens symboliques. Les chemins d’origine, noms Unicode et offsets déclarés sont conservés. Les sources partagées sont dédupliquées selon la même comparaison.

Reader contrôle l’alignement des données après l’offset et le nombre de secteurs disponibles. La prochaine piste borne la longueur d’une piste intermédiaire. Audio conserve sa taille stockée, Mode1/2048 expose 2048 octets et Mode1 brut expose 2048 octets à partir de l’offset 16. Les fichiers associés, index 1 et propriétés par piste sont fournis dans le document. Comme en C#, la reconnaissance valide le descripteur et l’existence des fichiers ; les dimensions des données sont contrôlées lors de la lecture du document.

Writer produit un fichier .trackNN.bin par piste et un descripteur UTF-8 sans BOM, avec fins de ligne de la plateforme. Il copie les secteurs stockés par blocs de 64 KiB et publie tout le jeu dans la transaction atomique existante. Les pistes à pregap stocké ou sous-canaux, modes incompatibles, faces/couches et numéros dupliqués sont refusés comme en C#.

Correction de fiabilité par rapport au C# : après tri par numéro, les LBA doivent augmenter et la longueur d’une piste intermédiaire doit être exactement la distance au prochain LBA. Le C# pouvait accepter un document dont sa relecture refusait les données trop courtes ou tronquait les données trop longues. Le writer C++ refuse ces documents, sans inventer de remplissage ni changer les LBA. Un nom de sortie contenant un guillemet est également refusé, car la syntaxe du lecteur ne décode pas les échappements de guillemets. Les mises à jour C# ne sont pas modifiées.

Aucune logique n’est ajoutée à App. La composition globale des lecteurs/writers reste à terminer.
