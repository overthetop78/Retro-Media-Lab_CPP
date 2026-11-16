# Reconstruction SCP Apple

## Organisation

`Reconstruction/Apple/Decoder` collecte les candidats et conserve les tags, le format enregistré et la révolution lors de la sélection. `AppleII` construit DOS13/DOS16, ProDOS et RWTS18 ; `Macintosh` gère les zones et l'identification Mac/Lisa ; `Reader` porte le routage explicite et automatique. Le chemin fichier réutilise le lecteur SCP existant. Aucune logique App ou matérielle ajoutée.

La collecte garde les adresses décodées comme le C# ; elle ne leur impose pas une concordance physique supplémentaire. Elle utilise les fenêtres continues déjà partagées et publie une progression typée par piste. Les cylindres SCP hors géométrie Macintosh sont ignorés tout en publiant leur progression.

## Comportements conservés

- Priorité d'intégrité vraie, inconnue, puis fausse ; égalités stables. Tags et code de format copiés dans le secteur retenu.
- Apple II : inférence DOS13/DOS16 depuis les numéros observés, ordre physique vers DOS, limite de 50 pistes, minimum de 35. ProDOS exige les deux secteurs physiques d'un bloc ; intégrité combinée et révolution maximale. Les positions incomplètes restent absentes.
- RWTS18 : six blocs de 768 octets par piste et mêmes bornes de pistes.
- Macintosh : sept facteurs autour de la cellule estimée ; score par secteurs distincts plausibles, intégrités vraies et nombre de candidats ; premier meilleur score conservé, arrêt lorsqu'une piste entière est valide. Le décodeur natif IWM est utilisé, avec une fonction injectable pour vérifier cette sélection sans refaire les tests du codec.
- Adressage Macintosh zoné et une/deux faces inférés ; tags Lisa Office, charge utile Lisa complète, PREBOOT MacWorks et signatures MFS/HFS. Les mêmes priorités que le C# sont conservées, y compris le repli ProDOS lorsque le bloc MDB ne porte pas une signature Mac et qu'aucune famille Lisa n'a déjà été retenue.
- Le routage SOS exige une charge utile ProDOS complète et son marqueur d'amorçage. L'automatique essaie Macintosh/Lisa, Apple II, RWTS18 ; ratio de blocs disponibles prioritaire, puis nombre de blocs, ordre stable.

## Rejets et exceptions

`MediaError` peut maintenant posséder des causes nommées contenant les `exception_ptr` originaux. Le diagnostic principal demeure une clé traduisible, les causes gardent leurs diagnostics propres : aucune concaténation définitive d'erreurs anglaises. L'échec Apple expose les trois identités et causes au consommateur. Les accès et annulations ne sont pas transformés en absence d'encodage.

La classification des rejets de lecture inclut les diagnostics de reconstruction SCP ajoutés dans les derniers blocs ; les erreurs d'argument, d'accès et d'annulation conservent leur traitement distinct. Les registres et fonctions de décodage sont possédés ; les buffers restent dans les valeurs/objets partagés, sans thread ou ressource native supplémentaire.

## Vérifications

Un banc temporaire unique : DOS13/16, paires ProDOS et positions absentes, sélection d'intégrité/métadonnées/révolution, RWTS18 étendu, zones double face, tags Lisa, MDB MFS/HFS, PREBOOT et Lisa complète ; facteurs Macintosh/arrêt anticipé et progression des pistes ignorées ; routage SOS, classement par ratio puis compte, échec des trois familles avec causes originales, propagation d'annulation et d'accès.

Deux petits cas d'intégration réelle complètent ces contrôles : lecture d'un fichier SCP Macintosh encodé et reconstruction de son secteur avec tags ; Apple II avec un secteur 6-and-2 dont le profil 16 secteurs est indiqué explicitement au constructeur de la capture. La première invocation de ce dernier cas omettait ce paramètre obligatoire ; seule l'invocation du banc a été corrigée, sans modifier le codec.

Compilation Debug MediaEngine et 30 bibliothèques de catalogues ; audit global des 1 461 fichiers C++ et paramètres des 30 catalogues. Traductions Argos relues et corrigées, en particulier les termes de famille de formats et les marqueurs d'amorçage. Banc, capture et journaux temporaires supprimés après utilisation.

## Référence

Sources C# `Images/Reading/Reconstruction/Apple/*.cs`, `SectorCandidateSelector.cs`, `ScpReconstructionExceptions.cs` consultées sans modification. Empreintes actualisées dans l'inventaire. Les constructeurs communs, géométries, codecs et sondes existants sont réutilisés ; les autres services de conversion/reconnaissance restent suivis dans l'inventaire général.
