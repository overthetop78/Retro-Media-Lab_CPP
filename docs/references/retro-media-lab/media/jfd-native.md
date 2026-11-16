# Structure native JFD

Référence en lecture seule : `F:/GW GUI/src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/Jfd/JfdCodec.cs`, SHA256 `2b574ceda7ac5ec9ce631cf465c9f7c013b20458004d47cd82122a69a408098c`.

Le décodage reprend les tables principales puis remplace les pistes désignées par les tables delta. Les entrées de piste vides restent significatives pour la géométrie ; une entrée absente ne l'est pas. Les secteurs sans données ne sont pas transformés en secteurs lisibles remplis de zéros. Les doubles numéros et numéros hors plage reçoivent des indices logiques supplémentaires, dans l'ordre C#.

La représentation sectorielle conserve les intégrités séparées, le code de taille, la densité, l'ordre enregistré et les quatre octets du descripteur. Un cinquième octet signale la présence d'une disposition native particulière. Les indicateurs de pistes brutes et de positions explicites seront utilisés par le lecteur pour produire ses messages traduits.

L'édition recopie l'ensemble des octets natifs puis applique seulement les modifications compatibles des secteurs présents et des bits d'intégrité. Les historiques, champs inconnus, pistes brutes et secteurs absents restent donc conservés. Les écritures contradictoires vers des descripteurs, données partagées ou plages recouvrantes sont rejetées.

## Compression et nettoyage

La version embarquée de libarchive ne vérifie pas le CRC ni la taille de fin de membre gzip : voir `third_party/libarchive/libarchive/archive_read_support_filter_gzip.c`, fonction de consommation de fin de flux, commentaire `TODO: Verify the length and CRC`. Le JFD utilise donc directement `inflateInit2` avec enveloppe gzip de zlib, déjà embarquée comme dépendance de libarchive. Référence API locale : `third_party/zlib/zlib.h`, documentation de `inflate`, `inflateInit2` et `inflateReset2` ; le mode gzip vérifie le CRC du flux. Aucun nouvel outil ni téléchargement n'est nécessaire. L'extraction des archives générales reste dans libarchive.

Chaque membre concaténé est lu avec la même limite globale de 64 Mio ; le résultat et l'entrée sont tous deux bornés comme dans le C#. Un propriétaire RAII appelle `inflateEnd` sur sortie normale ou exception. Les boucles de décompression, de décodage et d'édition acceptent l'annulation.

## État

Ce bloc porte `Unpack`, `Decode`, `ReadTables`, `Rewrite` et les utilitaires binaires. `CanCreate`, `Create`, le lecteur du registre et le writer restent à porter. Il ne rend pas JFD disponible dans la composition générale de l'application.
