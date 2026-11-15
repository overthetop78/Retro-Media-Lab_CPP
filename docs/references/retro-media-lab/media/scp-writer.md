# Écriture SCP et conversion des révolutions synthétiques

## Référence et découpage

`MediaEngine/Formats/Scp/Writer.cpp` valide et sérialise le conteneur ; `Synthetic.cpp` convertit les instants d’une piste encodée vers la résolution SCP. Les constantes de capture restent avec SCP ; la durée d’un tick d’encodeur appartient à `Encoding/Constants.h`. Le writer réutilise la publication atomique commune, sans dépendance GUI/matérielle.

Le banc compile 28 sources C# originales dont le writer, le lecteur, les modèles, les algorithmes de checksum et la conversion synthétique ; leurs empreintes correspondent à l’inventaire. Les contrats de registres/documents non exercés sont des adaptateurs de banc minimaux. Aucun algorithme comparé n’est remplacé. [Validation](../../../validation/media/scp-writer.md).

## Validation et octets produits

Les validations suivent l’ordre du C# : en-tête, refus Extended, unicité des numéros de piste, appartenance à la plage, cohérence numéro/cylindre/face, nombre de révolutions puis absence d’intervalle nul. Les pistes absentes dans la plage déclarée sont acceptées. Les métadonnées `ChecksumValid`, taille de fichier, origine et nombre déclaré de flux ne contraignent pas les nouveaux octets ; le nombre réel de mots encodés et le checksum sont recalculés.

Les pistes sont écrites dans l’ordre numérique. Les offsets et descripteurs sont little endian, les mots de flux big endian. Les grands intervalles produisent des marqueurs zéro de 65 536 ticks. Lorsqu’un intervalle non terminal est un multiple exact de 65 536, le writer retire un tick et le reporte sur l’intervalle suivant, comme dans le C#. Cette correction historique est préservée, même si les intervalles relus diffèrent ainsi de la liste originale. Un report qui dépasse uint32 produit une erreur, sans enveloppement silencieux.

Les tailles de piste sont contrôlées dans la limite int32 ; les offsets de début de piste doivent tenir dans uint32. Le writer ne rabaisse pas artificiellement la limite totale à celle du lecteur en mémoire : la fin de la dernière piste peut dépasser uint32 si ses champs de position restent représentables, comme dans la source. Les fichiers de cette taille ne sont pas fabriqués par le banc ; les limites sont vérifiées par calcul avant émission. La lecture SCP en mémoire garde sa propre limite int32.

## Sortie bornée et atomicité

Le plan possède uniquement en-tête, table et descripteurs de chaque piste ; il emprunte les pistes constantes pendant l’appel. Le comptage des mots utilise des suites de marqueurs et ne construit pas de tableau de milliards de mots. Le checksum est obtenu pendant cette préparation à partir des mêmes octets à émettre. La sortie peut donc avancer une seule fois, par blocs d’au plus 65 536 octets, sans repositionnement ni relecture du fichier temporaire.

L’API de rappel est synchrone : le consommateur lit chaque span avant son retour, et le propriétaire ne modifie pas l’image pendant l’appel. Les exceptions du consommateur sont propagées. Ce contrat de flux ne promet pas de revenir sur les octets déjà remis au consommateur ; la surcharge fichier emploie `writeAtomicImage`/QSaveFile, avec repli direct désactivé, pour ne remplacer la destination qu’après succès. Validation, débordement et annulation avant publication conservent donc le fichier précédent. Les allocations et temporaires sont libérés par RAII sur erreur et annulation.

## Conversion temporelle

Le tick commun d’encodeur dure 25 ns. Le tick cible SCP vaut 25 × (résolution + 1) ns. Les intervalles sont convertis en cumulant leurs instants absolus puis en arrondissant au plus proche, demi-vers-le-haut ; seuls les instants cibles strictement croissants sont ajoutés. Cela évite d’additionner des erreurs d’arrondi indépendantes. L’index est converti séparément. Les sommes, produits et conversions sont vérifiés ; les intervalles convertis peuvent être vides. La révolution est marquée Synthetic, avec nombre de mots déclaré zéro avant sérialisation.

La conversion cylindre/face vers numéro SCP conserve les vérifications signées C# avant le cast en octet ; elle ne rajoute pas de validation géométrique que cette fonction n’effectuait pas. Les constructeurs de capture restent responsables d’employer une géométrie valide.

## Traductions et suite

Les six nouveaux diagnostics sont dans `MediaEngine/scp-writer.json`, dans 29 langues. Argos les génère ; la relecture corrige notamment les traductions partielles, répétitions, rotations et têtes magnétiques. Les erreurs d’en-tête/débordement/publication utilisent les catalogues déjà portés.

Les interfaces applicatives de documents, registres, capture matérielle et conversion complète de fichiers restent à porter. Ce bloc fournit le writer et la conversion synthétique, validés avec le codec Commodore 900 ; il ne déclare pas les autres familles SCP terminées.
